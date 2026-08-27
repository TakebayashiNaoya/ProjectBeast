/**
 * @file StageNavGrid.cpp
 * @brief ハイトマップから作る歩行可否グリッド（簡易ナビゲーション）
 */
#include "stdafx.h"
#include "StageNavGrid.h"

#include <queue>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** セルの目標サイズ（ワールド単位）。地形6000で約150x150セルになる */
			constexpr float NAV_CELL_SIZE = 40.0f;

			/**
			 * @brief 通行不能とみなす傾斜のタンジェント
			 * @details tan(50度)=1.19。接地限界は63度だが、粗いセルへ落とすと傾斜が
			 *          なまって小さく見積もられるため、少し手前に置いてある。
			 *          Hardの氷盤内部の最大傾斜40.7度は通行可能のまま残る。
			 */
			constexpr float NAV_BLOCK_SLOPE_TAN = 1.19f;

			/** このワールドYより低いセルは水とみなす（波の下端。docs/引き継ぎの高さ表を参照） */
			constexpr float NAV_WATER_MAX_Y = -6.0f;

			/** 水から上がれる浜とみなす陸セルの最大ワールドY（波頂+8。それより高い水際は絶壁） */
			constexpr float NAV_SHORE_MAX_Y = 8.0f;

			/** 隣接セル間で乗り越えられる最大段差（ワールド単位） */
			constexpr float NAV_MAX_STEP = 45.0f;

			/** 水セルの移動コスト係数（泳ぎは走りより速いので割安にする） */
			constexpr float NAV_WATER_COST_SCALE = 0.75f;

			/** 出発セルが通行不能だったときに通行可能セルを探す近傍リング数 */
			constexpr int NAV_SNAP_SEARCH_RING = 2;

			/** 8方向の隣接オフセット（X, Z） */
			constexpr int NEIGHBOR_OFFSETS[8][2] = {
				{ 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
				{ 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 },
			};
		}


		void StageNavGrid::Build(
			const std::vector<float>& heights,
			const int vertexW,
			const int vertexH,
			const float totalWidth,
			const float totalDepth,
			const float yOffset)
		{
			if (vertexW < 2 || vertexH < 2) return;

			m_cellNumX = max(1, static_cast<int>(totalWidth / NAV_CELL_SIZE));
			m_cellNumZ = max(1, static_cast<int>(totalDepth / NAV_CELL_SIZE));
			m_cellNum = m_cellNumX * m_cellNumZ;
			m_cellSizeX = totalWidth / m_cellNumX;
			m_cellSizeZ = totalDepth / m_cellNumZ;
			m_minX = -totalWidth * 0.5f;
			m_minZ = -totalDepth * 0.5f;

			// セルごとの集計バッファ
			std::vector<int>   sampleCounts(m_cellNum, 0);
			std::vector<float> sumHeights(m_cellNum, 0.0f);
			std::vector<float> maxHeights(m_cellNum, -FLT_MAX);
			std::vector<float> maxSlopes(m_cellNum, 0.0f);

			// 頂点グリッドを走査し、各頂点をそれが属するセルへ集計する。
			// 頂点の高さと、隣の頂点との勾配（傾斜のタンジェント）を集める
			const float fineDX = totalWidth / (vertexW - 1);
			const float fineDZ = totalDepth / (vertexH - 1);

			for (int z = 0; z < vertexH; ++z)
			{
				for (int x = 0; x < vertexW; ++x)
				{
					const float h = heights[z * vertexW + x];

					// 頂点のワールド座標。
					// 3ds Max Z-up からの変換で worldZ = totalDepth/2 - z*fineDZ になる
					// （TerrainObject::GenerateMesh のチャンクAABB計算と同じ式）
					const float worldX = m_minX + x * fineDX;
					const float worldZ = (totalDepth * 0.5f) - z * fineDZ;

					const int cx = static_cast<int>((worldX - m_minX) / m_cellSizeX);
					const int cz = static_cast<int>((worldZ - m_minZ) / m_cellSizeZ);
					if (cx < 0 || cx >= m_cellNumX || cz < 0 || cz >= m_cellNumZ) continue;

					const int cellIndex = cz * m_cellNumX + cx;
					const float worldY = h + yOffset;

					sampleCounts[cellIndex]++;
					sumHeights[cellIndex] += worldY;
					maxHeights[cellIndex] = max(maxHeights[cellIndex], worldY);

					// 勾配（+X・+Z方向の前進差分）
					if (x + 1 < vertexW && z + 1 < vertexH)
					{
						const float gx = (heights[z * vertexW + (x + 1)] - h) / fineDX;
						const float gz = (heights[(z + 1) * vertexW + x] - h) / fineDZ;
						const float slopeTan = sqrtf(gx * gx + gz * gz);
						maxSlopes[cellIndex] = max(maxSlopes[cellIndex], slopeTan);
					}
				}
			}

			// 集計結果からセルを分類する
			m_cellTypes.assign(m_cellNum, static_cast<uint8_t>(EnCellType::Blocked));
			m_cellHeights.assign(m_cellNum, 0.0f);
			m_flowNext.assign(m_cellNum, -1);
			m_isFlowFieldBuilt = false;

			for (int i = 0; i < m_cellNum; ++i)
			{
				// サンプルが1つも入らなかったセルは安全側で通行不能にする
				if (sampleCounts[i] == 0) continue;

				if (maxHeights[i] <= NAV_WATER_MAX_Y)
				{
					// セル全体が波の下：水。泳いで渡れる。通行高さは海面
					m_cellTypes[i] = static_cast<uint8_t>(EnCellType::Water);
					m_cellHeights[i] = 0.0f;
				}
				else if (maxSlopes[i] > NAV_BLOCK_SLOPE_TAN)
				{
					// 急斜面（絶壁）を含む：通行不能
					m_cellTypes[i] = static_cast<uint8_t>(EnCellType::Blocked);
				}
				else
				{
					// 陸。通行高さは平均（浜は海面まで持ち上げる）
					m_cellTypes[i] = static_cast<uint8_t>(EnCellType::Land);
					m_cellHeights[i] = max(0.0f, sumHeights[i] / sampleCounts[i]);
				}
			}
		}


		int StageNavGrid::CellIndexFromWorld(const Vector3& pos) const
		{
			const int cx = static_cast<int>((pos.x - m_minX) / m_cellSizeX);
			const int cz = static_cast<int>((pos.z - m_minZ) / m_cellSizeZ);
			if (cx < 0 || cx >= m_cellNumX || cz < 0 || cz >= m_cellNumZ) return -1;
			return cz * m_cellNumX + cx;
		}


		Vector3 StageNavGrid::CellCenter(const int cellIndex) const
		{
			const int cx = cellIndex % m_cellNumX;
			const int cz = cellIndex / m_cellNumX;
			return Vector3(
				m_minX + (cx + 0.5f) * m_cellSizeX,
				m_cellHeights[cellIndex],
				m_minZ + (cz + 0.5f) * m_cellSizeZ
			);
		}


		bool StageNavGrid::IsConnected(const int fromIndex, const int toIndex) const
		{
			const EnCellType fromType = static_cast<EnCellType>(m_cellTypes[fromIndex]);
			const EnCellType toType = static_cast<EnCellType>(m_cellTypes[toIndex]);

			if (fromType == EnCellType::Blocked || toType == EnCellType::Blocked) return false;

			// 水どうしは常に行き来できる
			if (fromType == EnCellType::Water && toType == EnCellType::Water) return true;

			// 陸が絡む移動は段差を見る
			const float step = fabsf(m_cellHeights[toIndex] - m_cellHeights[fromIndex]);
			if (step > NAV_MAX_STEP) return false;

			// 水と陸の行き来は浜（低い陸）だけ。高い水際は絶壁なので登れない
			if (fromType != toType)
			{
				const float landY = (fromType == EnCellType::Land)
					? m_cellHeights[fromIndex]
					: m_cellHeights[toIndex];
				if (landY > NAV_SHORE_MAX_Y) return false;
			}

			return true;
		}


		float StageNavGrid::MoveCost(const int fromIndex, const int toIndex) const
		{
			const int dx = abs(fromIndex % m_cellNumX - toIndex % m_cellNumX);
			const int dz = abs(fromIndex / m_cellNumX - toIndex / m_cellNumX);
			const float dist = (dx + dz == 2) ? 1.41421356f : 1.0f;

			// 水は泳げるので割安。両端の平均で評価する
			const bool fromWater = m_cellTypes[fromIndex] == static_cast<uint8_t>(EnCellType::Water);
			const bool toWater = m_cellTypes[toIndex] == static_cast<uint8_t>(EnCellType::Water);
			const float scale =
				1.0f - (1.0f - NAV_WATER_COST_SCALE) * 0.5f * (static_cast<float>(fromWater) + static_cast<float>(toWater));

			return dist * scale;
		}


		int StageNavGrid::FindNearbyPassableCell(const int cellIndex) const
		{
			if (cellIndex < 0) return -1;
			if (m_cellTypes[cellIndex] != static_cast<uint8_t>(EnCellType::Blocked)) return cellIndex;

			const int cx = cellIndex % m_cellNumX;
			const int cz = cellIndex / m_cellNumX;

			for (int ring = 1; ring <= NAV_SNAP_SEARCH_RING; ++ring)
			{
				for (int dz = -ring; dz <= ring; ++dz)
				{
					for (int dx = -ring; dx <= ring; ++dx)
					{
						if (max(abs(dx), abs(dz)) != ring) continue;

						const int nx = cx + dx;
						const int nz = cz + dz;
						if (nx < 0 || nx >= m_cellNumX || nz < 0 || nz >= m_cellNumZ) continue;

						const int neighborIndex = nz * m_cellNumX + nx;
						if (m_cellTypes[neighborIndex] != static_cast<uint8_t>(EnCellType::Blocked))
						{
							return neighborIndex;
						}
					}
				}
			}
			return -1;
		}


		bool StageNavGrid::IsReachable(const Vector3& from, const Vector3& to) const
		{
			// グリッドが無い（チュートリアル等の地形なしステージ）なら判定を諦めて通す
			if (!IsBuilt()) return true;

			const int goalIndex = CellIndexFromWorld(to);
			if (goalIndex < 0) return false;

			// 目的地が急斜面のセルならその時点で不可
			if (m_cellTypes[goalIndex] == static_cast<uint8_t>(EnCellType::Blocked)) return false;

			// 出発地が急斜面のセル上（半端に登った状態）なら近傍の通行可能セルから探索する
			const int startIndex = FindNearbyPassableCell(CellIndexFromWorld(from));
			if (startIndex < 0) return false;
			if (startIndex == goalIndex) return true;

			// A*（オクタイル距離ヒューリスティック）
			struct SNode { float f; int index; };
			struct SNodeGreater
			{
				bool operator()(const SNode& a, const SNode& b) const { return a.f > b.f; }
			};

			const int goalX = goalIndex % m_cellNumX;
			const int goalZ = goalIndex / m_cellNumX;
			auto heuristic = [&](const int index)
			{
				const float dx = static_cast<float>(abs(index % m_cellNumX - goalX));
				const float dz = static_cast<float>(abs(index / m_cellNumX - goalZ));
				// 水コスト割引があるためヒューリスティックにも同じ割引を掛けて過大評価を防ぐ
				return (max(dx, dz) + 0.41421356f * min(dx, dz)) * NAV_WATER_COST_SCALE;
			};

			std::vector<float> costs(m_cellNum, FLT_MAX);
			std::priority_queue<SNode, std::vector<SNode>, SNodeGreater> openList;

			costs[startIndex] = 0.0f;
			openList.push({ heuristic(startIndex), startIndex });

			while (!openList.empty())
			{
				const SNode node = openList.top();
				openList.pop();

				if (node.index == goalIndex) return true;

				const int cx = node.index % m_cellNumX;
				const int cz = node.index / m_cellNumX;

				for (const auto& offset : NEIGHBOR_OFFSETS)
				{
					const int nx = cx + offset[0];
					const int nz = cz + offset[1];
					if (nx < 0 || nx >= m_cellNumX || nz < 0 || nz >= m_cellNumZ) continue;

					const int neighborIndex = nz * m_cellNumX + nx;
					if (!IsConnected(node.index, neighborIndex)) continue;

					// 斜め移動は、間の直交セルがどちらも通行可能なときだけ許す（角抜け防止）
					if (offset[0] != 0 && offset[1] != 0)
					{
						const int sideA = cz * m_cellNumX + nx;
						const int sideB = nz * m_cellNumX + cx;
						if (m_cellTypes[sideA] == static_cast<uint8_t>(EnCellType::Blocked)
							|| m_cellTypes[sideB] == static_cast<uint8_t>(EnCellType::Blocked))
						{
							continue;
						}
					}

					const float newCost = costs[node.index] + MoveCost(node.index, neighborIndex);
					if (newCost >= costs[neighborIndex]) continue;

					costs[neighborIndex] = newCost;
					openList.push({ newCost + heuristic(neighborIndex), neighborIndex });
				}
			}

			return false;
		}


		bool StageNavGrid::BuildFlowField(const Vector3& goalPos)
		{
			m_isFlowFieldBuilt = false;
			if (!IsBuilt()) return false;

			const int goalIndex = FindNearbyPassableCell(CellIndexFromWorld(goalPos));
			if (goalIndex < 0) return false;

			// 目標から全セルへのダイクストラ。m_flowNext[セル] = 目標へ一歩近づくセル
			struct SNode { float cost; int index; };
			struct SNodeGreater
			{
				bool operator()(const SNode& a, const SNode& b) const { return a.cost > b.cost; }
			};

			std::vector<float> costs(m_cellNum, FLT_MAX);
			std::fill(m_flowNext.begin(), m_flowNext.end(), -1);
			std::priority_queue<SNode, std::vector<SNode>, SNodeGreater> openList;

			costs[goalIndex] = 0.0f;
			m_flowNext[goalIndex] = goalIndex;
			openList.push({ 0.0f, goalIndex });

			while (!openList.empty())
			{
				const SNode node = openList.top();
				openList.pop();

				if (node.cost > costs[node.index]) continue;

				const int cx = node.index % m_cellNumX;
				const int cz = node.index / m_cellNumX;

				for (const auto& offset : NEIGHBOR_OFFSETS)
				{
					const int nx = cx + offset[0];
					const int nz = cz + offset[1];
					if (nx < 0 || nx >= m_cellNumX || nz < 0 || nz >= m_cellNumZ) continue;

					const int neighborIndex = nz * m_cellNumX + nx;

					// 探索は目標→周囲の向きだが、実際の移動は周囲→目標の向きになる。
					// 接続判定は移動の向き（neighbor → node）で行う
					if (!IsConnected(neighborIndex, node.index)) continue;

					if (offset[0] != 0 && offset[1] != 0)
					{
						const int sideA = cz * m_cellNumX + nx;
						const int sideB = nz * m_cellNumX + cx;
						if (m_cellTypes[sideA] == static_cast<uint8_t>(EnCellType::Blocked)
							|| m_cellTypes[sideB] == static_cast<uint8_t>(EnCellType::Blocked))
						{
							continue;
						}
					}

					const float newCost = node.cost + MoveCost(neighborIndex, node.index);
					if (newCost >= costs[neighborIndex]) continue;

					costs[neighborIndex] = newCost;
					m_flowNext[neighborIndex] = node.index;
					openList.push({ newCost, neighborIndex });
				}
			}

			m_isFlowFieldBuilt = true;
			return true;
		}


		bool StageNavGrid::GetFlowDirection(const Vector3& from, Vector3& outDir) const
		{
			if (!m_isFlowFieldBuilt) return false;

			const int cellIndex = FindNearbyPassableCell(CellIndexFromWorld(from));
			if (cellIndex < 0) return false;

			int nextIndex = m_flowNext[cellIndex];
			if (nextIndex < 0) return false;			// 目標へ到達できないセル
			if (nextIndex == cellIndex) return false;	// すでに目標セルにいる

			// 次セルの中心にかなり近い場合はさらに一歩先を見る（セル境界での小刻みな向き変えを防ぐ）
			Vector3 target = CellCenter(nextIndex);
			Vector3 toTarget = target - from;
			toTarget.y = 0.0f;
			const float nearDist = min(m_cellSizeX, m_cellSizeZ) * 0.5f;
			if (toTarget.LengthSq() < nearDist * nearDist)
			{
				const int nextNextIndex = m_flowNext[nextIndex];
				if (nextNextIndex >= 0 && nextNextIndex != nextIndex)
				{
					target = CellCenter(nextNextIndex);
					toTarget = target - from;
					toTarget.y = 0.0f;
				}
			}

			if (toTarget.LengthSq() <= FLT_EPSILON) return false;

			toTarget.Normalize();
			outDir = toTarget;
			return true;
		}
	}
}
