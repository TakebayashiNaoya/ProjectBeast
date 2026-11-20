/**
 * @file OrcaSeaGrid.cpp
 * @brief シャチが泳げる場所の地図（航行可能マスク）
 */
#include "stdafx.h"
#include "OrcaSeaGrid.h"
#include <queue>
#include <random>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 1セル内で高さを調べる点の数（1辺あたり） */
			constexpr int HEIGHT_SAMPLES_PER_SIDE = 3;
			/** 経路探索の斜め移動のコスト */
			const float DIAGONAL_COST = sqrtf(2.0f);
			/** セル境界ちょうどの点を避けるための内側への寄せ量（セル幅に対する割合） */
			constexpr float CELL_INSET_RATIO = 0.01f;
		}


		void OrcaSeaGrid::Build(const Settings& settings, const HeightFunc& heightFunc)
		{
			m_settings = settings;
			const float cell = settings.cellSize;
			const float radius = settings.boundaryRadius;

			m_origin = -radius;
			m_size = static_cast<int>(ceilf((radius * 2.0f) / cell));
			const int total = m_size * m_size;
			m_sea.assign(total, 0);
			m_navigable.assign(total, 0);
			m_component.assign(total, -1);
			m_navigableCount = 0;
			m_largestComponent = -1;

			// --- 海セル ---
			const float radiusSq = radius * radius;
			for (int cz = 0; cz < m_size; cz++)
			{
				for (int cx = 0; cx < m_size; cx++)
				{
					const float x0 = m_origin + cx * cell;
					const float z0 = m_origin + cz * cell;

					// セルの4隅がすべて外周の内側か
					bool inside = true;
					for (int corner = 0; corner < 4 && inside; corner++)
					{
						const float x = x0 + ((corner & 1) ? cell : 0.0f);
						const float z = z0 + ((corner & 2) ? cell : 0.0f);
						if (x * x + z * z > radiusSq) inside = false;
					}
					if (!inside) continue;

					// セル内 3×3 点の高さの最大値が海底の上限以下か
					bool deep = true;
					for (int sz = 0; sz < HEIGHT_SAMPLES_PER_SIDE && deep; sz++)
					{
						for (int sx = 0; sx < HEIGHT_SAMPLES_PER_SIDE && deep; sx++)
						{
							const float fx = static_cast<float>(sx) / (HEIGHT_SAMPLES_PER_SIDE - 1);
							const float fz = static_cast<float>(sz) / (HEIGHT_SAMPLES_PER_SIDE - 1);
							if (heightFunc(x0 + fx * cell, z0 + fz * cell) > settings.seaFloorMaxY) deep = false;
						}
					}
					if (deep) m_sea[Index(cx, cz)] = 1;
				}
			}

			// --- 航行可能セル ---
			// 自分のセルの正方形と、海でないセル（地図の外を含む）の正方形の最短距離が bodyClearance 以上なら航行可能。
			// オフセット (di, dj) のセルどうしの正方形の距離は、各軸 max(0, |d|-1) × cell の長さを持つ直角三角形の斜辺
			const float clearance = settings.bodyClearance;
			const int reach = static_cast<int>(ceilf(clearance / cell)) + 1;
			for (int cz = 0; cz < m_size; cz++)
			{
				for (int cx = 0; cx < m_size; cx++)
				{
					if (!m_sea[Index(cx, cz)]) continue;

					bool ok = true;
					for (int dz = -reach; dz <= reach && ok; dz++)
					{
						for (int dx = -reach; dx <= reach && ok; dx++)
						{
							const int nx = cx + dx;
							const int nz = cz + dz;
							const bool neighborIsSea = InGrid(nx, nz) && m_sea[Index(nx, nz)];
							if (neighborIsSea) continue;

							const float gx = max(0, abs(dx) - 1) * cell;
							const float gz = max(0, abs(dz) - 1) * cell;
							if (gx * gx + gz * gz < clearance * clearance) ok = false;
						}
					}
					if (ok)
					{
						m_navigable[Index(cx, cz)] = 1;
						m_navigableCount++;
					}
				}
			}

			// --- 連結域（辺でつながるセルどうし。経路探索の「角の斜め抜け禁止」と同じつながり方） ---
			int componentCount = 0;
			int largestSize = 0;
			std::vector<int> stack;
			for (int start = 0; start < total; start++)
			{
				if (!m_navigable[start] || m_component[start] >= 0) continue;

				const int id = componentCount++;
				int size = 0;
				stack.clear();
				stack.push_back(start);
				m_component[start] = id;
				while (!stack.empty())
				{
					const int i = stack.back();
					stack.pop_back();
					size++;
					const int cx = i % m_size;
					const int cz = i / m_size;
					const int nx[4] = { cx + 1, cx - 1, cx, cx };
					const int nz[4] = { cz, cz, cz + 1, cz - 1 };
					for (int n = 0; n < 4; n++)
					{
						if (!InGrid(nx[n], nz[n])) continue;
						const int ni = Index(nx[n], nz[n]);
						if (!m_navigable[ni] || m_component[ni] >= 0) continue;
						m_component[ni] = id;
						stack.push_back(ni);
					}
				}
				if (size > largestSize)
				{
					largestSize = size;
					m_largestComponent = id;
				}
			}

			m_pathCost.assign(total, 0.0f);
			m_pathParent.assign(total, -1);
			m_pathStamp.assign(total, 0);
			m_pathSearchId = 0;
		}


		void OrcaSeaGrid::ToCell(float x, float z, int& cx, int& cz) const
		{
			cx = static_cast<int>(floorf((x - m_origin) / m_settings.cellSize));
			cz = static_cast<int>(floorf((z - m_origin) / m_settings.cellSize));
		}


		Vector3 OrcaSeaGrid::CellCenter(int cx, int cz) const
		{
			const float cell = m_settings.cellSize;
			return Vector3(m_origin + (cx + 0.5f) * cell, 0.0f, m_origin + (cz + 0.5f) * cell);
		}


		bool OrcaSeaGrid::NavAt(int cx, int cz) const
		{
			return InGrid(cx, cz) && m_navigable[Index(cx, cz)] != 0;
		}


		bool OrcaSeaGrid::IsNavigableCell(int cx, int cz) const
		{
			return NavAt(cx, cz);
		}


		bool OrcaSeaGrid::IsSeaCell(int cx, int cz) const
		{
			return InGrid(cx, cz) && m_sea[Index(cx, cz)] != 0;
		}


		bool OrcaSeaGrid::IsNavigable(const Vector3& pos) const
		{
			if (!IsBuilt()) return false;
			int cx, cz;
			ToCell(pos.x, pos.z, cx, cz);
			return NavAt(cx, cz);
		}


		int OrcaSeaGrid::GetComponent(const Vector3& pos) const
		{
			if (!IsBuilt()) return -1;
			int cx, cz;
			ToCell(pos.x, pos.z, cx, cz);
			if (!NavAt(cx, cz)) return -1;
			return m_component[Index(cx, cz)];
		}


		Vector3 OrcaSeaGrid::ResolveMove(const Vector3& from, const Vector3& to) const
		{
			if (!IsNavigable(from)) return from;

			const float dx = to.x - from.x;
			const float dz = to.z - from.z;
			const float length = sqrtf(dx * dx + dz * dz);
			if (length <= FLT_EPSILON) return from;

			const float maxStep = m_settings.cellSize * 0.5f;
			const int steps = max(1, static_cast<int>(ceilf(length / maxStep)));
			const float stepX = dx / steps;
			const float stepZ = dz / steps;

			Vector3 current = from;
			for (int i = 0; i < steps; i++)
			{
				Vector3 candidate(current.x + stepX, from.y, current.z + stepZ);
				if (IsNavigable(candidate))
				{
					current = candidate;
					continue;
				}
				// 壁に沿って滑らせる（X だけ → Z だけ）
				Vector3 candidateX(current.x + stepX, from.y, current.z);
				if (IsNavigable(candidateX))
				{
					current = candidateX;
					continue;
				}
				Vector3 candidateZ(current.x, from.y, current.z + stepZ);
				if (IsNavigable(candidateZ))
				{
					current = candidateZ;
					continue;
				}
				break;
			}
			current.y = from.y;
			return current;
		}


		bool OrcaSeaGrid::FindNearestNavigable(const Vector3& target, int component, Vector3& out) const
		{
			if (!IsBuilt()) return false;

			const int targetComponent = GetComponent(target);
			if (targetComponent >= 0 && (component < 0 || component == targetComponent))
			{
				out = target;
				return true;
			}

			const float cell = m_settings.cellSize;
			const float inset = cell * CELL_INSET_RATIO;
			int tx, tz;
			ToCell(target.x, target.z, tx, tz);

			float bestDistSq = FLT_MAX;
			Vector3 best;
			int foundRing = -1;
			for (int ring = 0; ring <= m_size * 2; ring++)
			{
				// 見つかった輪の2つ外まで見れば、それより外に近い点は無い
				if (foundRing >= 0 && ring > foundRing + 2) break;

				for (int dz = -ring; dz <= ring; dz++)
				{
					for (int dx = -ring; dx <= ring; dx++)
					{
						if (abs(dx) != ring && abs(dz) != ring) continue; // 輪の上だけ
						const int cx = tx + dx;
						const int cz = tz + dz;
						if (!NavAt(cx, cz)) continue;
						if (component >= 0 && m_component[Index(cx, cz)] != component) continue;

						// セルの中で target に最も近い点（境界ちょうどは避けて少し内側）
						const float x0 = m_origin + cx * cell + inset;
						const float z0 = m_origin + cz * cell + inset;
						const float x1 = m_origin + (cx + 1) * cell - inset;
						const float z1 = m_origin + (cz + 1) * cell - inset;
						const Vector3 p(
							min(max(target.x, x0), x1),
							target.y,
							min(max(target.z, z0), z1));
						const float ddx = p.x - target.x;
						const float ddz = p.z - target.z;
						const float distSq = ddx * ddx + ddz * ddz;
						if (distSq < bestDistSq)
						{
							bestDistSq = distSq;
							best = p;
							if (foundRing < 0) foundRing = ring;
						}
					}
				}
			}

			if (foundRing < 0) return false;
			out = best;
			return true;
		}


		bool OrcaSeaGrid::HasSeaLine(const Vector3& a, const Vector3& b) const
		{
			if (!IsNavigable(a) || !IsNavigable(b)) return false;

			const float dx = b.x - a.x;
			const float dz = b.z - a.z;
			const float length = sqrtf(dx * dx + dz * dz);
			const float maxStep = m_settings.cellSize * 0.5f;
			const int steps = max(1, static_cast<int>(ceilf(length / maxStep)));
			for (int i = 1; i < steps; i++)
			{
				const float t = static_cast<float>(i) / steps;
				if (!IsNavigable(Vector3(a.x + dx * t, 0.0f, a.z + dz * t))) return false;
			}
			return true;
		}


		bool OrcaSeaGrid::FindPath(const Vector3& a, const Vector3& b, std::vector<Vector3>& outWaypoints, float* outLength) const
		{
			outWaypoints.clear();
			if (outLength) *outLength = 0.0f;

			const int compA = GetComponent(a);
			const int compB = GetComponent(b);
			if (compA < 0 || compA != compB) return false;

			if (HasSeaLine(a, b))
			{
				outWaypoints.push_back(b);
				if (outLength) *outLength = (b - a).Length();
				return true;
			}

			int ax, az, bx, bz;
			ToCell(a.x, a.z, ax, az);
			ToCell(b.x, b.z, bx, bz);
			const int startIndex = Index(ax, az);
			const int goalIndex = Index(bx, bz);

			// 作業領域は世代番号で使い回す（毎回の全体クリアを避ける）
			m_pathSearchId++;
			if (m_pathSearchId == 0)
			{
				std::fill(m_pathStamp.begin(), m_pathStamp.end(), 0u);
				m_pathSearchId = 1;
			}
			const uint32_t stamp = m_pathSearchId;

			auto heuristic = [&](int i)
			{
				const int dx = abs(i % m_size - bx);
				const int dz = abs(i / m_size - bz);
				return static_cast<float>(max(dx, dz) - min(dx, dz)) + DIAGONAL_COST * static_cast<float>(min(dx, dz));
			};

			using Node = std::pair<float, int>;
			std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;
			m_pathStamp[startIndex] = stamp;
			m_pathCost[startIndex] = 0.0f;
			m_pathParent[startIndex] = -1;
			open.push({ heuristic(startIndex), startIndex });

			bool reached = false;
			while (!open.empty())
			{
				const Node node = open.top();
				open.pop();
				const int i = node.second;
				if (i == goalIndex)
				{
					reached = true;
					break;
				}
				const float g = m_pathCost[i];
				if (node.first - heuristic(i) > g + 1e-3f) continue; // 古いエントリ

				const int cx = i % m_size;
				const int cz = i / m_size;
				for (int dz = -1; dz <= 1; dz++)
				{
					for (int dx = -1; dx <= 1; dx++)
					{
						if (dx == 0 && dz == 0) continue;
						const int nx = cx + dx;
						const int nz = cz + dz;
						if (!NavAt(nx, nz)) continue;
						// 角の斜め抜け禁止: 斜めに進むときは両側の辺のセルも航行可能であること
						if (dx != 0 && dz != 0 && (!NavAt(cx + dx, cz) || !NavAt(cx, cz + dz))) continue;

						const int ni = Index(nx, nz);
						const float ng = g + ((dx != 0 && dz != 0) ? DIAGONAL_COST : 1.0f);
						if (m_pathStamp[ni] == stamp && m_pathCost[ni] <= ng) continue;
						m_pathStamp[ni] = stamp;
						m_pathCost[ni] = ng;
						m_pathParent[ni] = i;
						open.push({ ng + heuristic(ni), ni });
					}
				}
			}
			if (!reached) return false;

			// ゴールから親をたどって、スタートの次のセルまでを並べる
			std::vector<Vector3> reversed;
			for (int i = goalIndex; i != startIndex && i >= 0; i = m_pathParent[i])
			{
				reversed.push_back(CellCenter(i % m_size, i / m_size));
			}
			outWaypoints.assign(reversed.rbegin(), reversed.rend());
			if (outWaypoints.empty()) outWaypoints.push_back(b);
			else outWaypoints.back() = b; // 最後はゴールそのもの（同じセルの中なので航行可能）

			SmoothPath(a, outWaypoints);

			if (outLength)
			{
				float length = 0.0f;
				Vector3 prev = a;
				for (const Vector3& p : outWaypoints)
				{
					Vector3 d = p - prev;
					d.y = 0.0f;
					length += d.Length();
					prev = p;
				}
				*outLength = length;
			}
			return true;
		}


		void OrcaSeaGrid::SmoothPath(const Vector3& start, std::vector<Vector3>& waypoints) const
		{
			if (waypoints.size() <= 1) return;

			std::vector<Vector3> result;
			Vector3 anchor = start;
			size_t index = 0;
			while (index < waypoints.size())
			{
				// anchor から見通せるいちばん遠い経由点へ飛ぶ
				size_t farthest = index;
				for (size_t j = waypoints.size() - 1; j > index; j--)
				{
					if (HasSeaLine(anchor, waypoints[j]))
					{
						farthest = j;
						break;
					}
				}
				result.push_back(waypoints[farthest]);
				anchor = waypoints[farthest];
				index = farthest + 1;
			}
			waypoints = std::move(result);
		}


		bool OrcaSeaGrid::SelfTest(std::string& outMessage)
		{
			// 合成地形: 原点に半径300の円い島（+10）、ほかは海底 −30
			OrcaSeaGrid grid;
			Settings settings;
			settings.cellSize = 25.0f;
			settings.seaFloorMaxY = -6.0f;
			settings.bodyClearance = 100.0f;
			settings.boundaryRadius = 1000.0f;
			grid.Build(settings, [](float x, float z)
			{
				return (x * x + z * z <= 300.0f * 300.0f) ? 10.0f : -30.0f;
			});

			auto fail = [&](const char* message)
			{
				outMessage = message;
				return false;
			};

			if (grid.IsNavigable(Vector3(0.0f, 0.0f, 0.0f)))      return fail(u8"島の中心が航行可能になっている");
			if (grid.IsNavigable(Vector3(350.0f, 0.0f, 0.0f)))    return fail(u8"島から50の点が余白100で弾かれていない");
			if (!grid.IsNavigable(Vector3(500.0f, 0.0f, 0.0f)))   return fail(u8"島から200の点が航行可能になっていない");
			if (grid.IsNavigable(Vector3(1100.0f, 0.0f, 0.0f)))   return fail(u8"外周の外が航行可能になっている");

			// 航行可能な点からランダムな行き先へ動かしても、必ず航行可能な点に止まる
			std::mt19937 random(12345);
			std::uniform_real_distribution<float> position(-950.0f, 950.0f);
			std::uniform_real_distribution<float> offset(-400.0f, 400.0f);
			int tested = 0;
			for (int i = 0; i < 100000 && tested < 10000; i++)
			{
				const Vector3 from(position(random), 0.0f, position(random));
				if (!grid.IsNavigable(from)) continue;
				const Vector3 to(from.x + offset(random), 0.0f, from.z + offset(random));
				if (!grid.IsNavigable(grid.ResolveMove(from, to))) return fail(u8"ResolveMove の戻り値が航行可能でない");
				tested++;
			}
			if (tested < 10000) return fail(u8"ResolveMove の試験点が足りない");

			// 島を挟んだ経路
			std::vector<Vector3> path;
			const Vector3 west(-600.0f, 0.0f, 0.0f);
			const Vector3 east(600.0f, 0.0f, 0.0f);
			if (!grid.FindPath(west, east, path)) return fail(u8"島を回り込む経路が見つからない");
			Vector3 prev = west;
			for (const Vector3& p : path)
			{
				if (!grid.IsNavigable(p)) return fail(u8"経路の経由点が航行可能でない");
				if (!grid.HasSeaLine(prev, p)) return fail(u8"経路の隣り合う経由点の間に見通しが無い");
				prev = p;
			}

			// 島の中心に最も近い航行可能点は、島の縁から余白ぶん離れた所
			Vector3 nearest;
			if (!grid.FindNearestNavigable(Vector3(0.0f, 0.0f, 0.0f), -1, nearest)) return fail(u8"最寄りの航行可能点が見つからない");
			if (!grid.IsNavigable(nearest)) return fail(u8"最寄りの点が航行可能でない");
			const float nearestDist = sqrtf(nearest.x * nearest.x + nearest.z * nearest.z);
			if (nearestDist < 400.0f || nearestDist > 500.0f) return fail(u8"最寄りの点が島の縁＋余白の位置にない");

			// 陸の帯（|x| < 100）で2つに分かれた海
			OrcaSeaGrid split;
			split.Build(settings, [](float x, float z)
			{
				return (fabsf(x) < 100.0f) ? 10.0f : -30.0f;
			});
			const Vector3 left(-500.0f, 0.0f, 0.0f);
			const Vector3 right(500.0f, 0.0f, 0.0f);
			if (split.FindPath(left, right, path)) return fail(u8"陸で分かれた海の間に経路が見つかってしまう");
			Vector3 sameSide;
			if (!split.FindNearestNavigable(right, split.GetComponent(left), sameSide)) return fail(u8"出発側の最寄り点が見つからない");
			if (sameSide.x >= 0.0f) return fail(u8"最寄り点が別の水域から選ばれた");

			outMessage = u8"OK";
			return true;
		}
	}
}
