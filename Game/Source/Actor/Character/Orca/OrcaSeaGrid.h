/**
 * @file OrcaSeaGrid.h
 * @brief シャチが泳げる場所の地図（航行可能マスク）
 */
#pragma once
#include <functional>
#include <vector>


namespace app
{
	namespace actor
	{
		/**
		 * @brief シャチが泳げる場所の地図
		 * @details 地形の高さから「海」セルと「航行可能」セルを作る。
		 *          - 海セル: セル内 3×3 点の地形の高さの最大値が seaFloorMaxY 以下で、セルの4隅が原点から boundaryRadius 以内
		 *          - 航行可能セル: 海セルで、かつ「セル内のどの点からも、海でないセルの正方形まで bodyClearance 以上」
		 *          航行可能セルの中の点はどこでも、浅い所から bodyClearance 以上離れている。
		 *          シャチの XZ は必ず ResolveMove() の戻り値で書き換えることで、海から出ないことを保証する。
		 *          エンジンのシングルトンには依存しない（高さは関数で受け取る）ので、合成した地形でテストできる。
		 */
		class OrcaSeaGrid
		{
		public:
			/** 地図を作る条件 */
			struct Settings
			{
				/** セルの一辺 */
				float cellSize = 25.0f;
				/** 海底がこれ以下なら海 */
				float seaFloorMaxY = -6.0f;
				/** 海でない所から離す距離 */
				float bodyClearance = 15.0f;
				/** 原点からこの半径の外は海でない扱い（外周の氷壁） */
				float boundaryRadius = 3700.0f;
			};

			/** 地形の高さを返す関数（ワールドX, ワールドZ → ワールドY） */
			using HeightFunc = std::function<float(float x, float z)>;


		public:
			/**
			 * @brief 地図を作る
			 * @param settings 条件
			 * @param heightFunc 地形の高さを返す関数
			 */
			void Build(const Settings& settings, const HeightFunc& heightFunc);

			/** @brief 作り済みか */
			bool IsBuilt() const { return m_size > 0; }

			/** @brief 点が航行可能か（Y は見ない） */
			bool IsNavigable(const Vector3& pos) const;

			/**
			 * @brief 点が属する連結域の番号
			 * @return 航行可能でなければ -1
			 */
			int GetComponent(const Vector3& pos) const;

			/**
			 * @brief from から to へ動こうとしたときに、実際に動ける位置を返す
			 * @details 線分をセル半分以下の刻みで進め、各刻みで「そのまま → X だけ → Z だけ → 止まる」の順に試す。
			 *          from が航行可能なら戻り値も必ず航行可能。Y は from のまま返す
			 */
			Vector3 ResolveMove(const Vector3& from, const Vector3& to) const;

			/**
			 * @brief 指定した連結域の中で target に最も近い航行可能な点を探す
			 * @param target 目標
			 * @param component 連結域の番号。-1 ならどの連結域でもよい
			 * @param out 見つかった点（target が航行可能でその連結域なら target そのもの）
			 * @return 見つかれば true
			 */
			bool FindNearestNavigable(const Vector3& target, int component, Vector3& out) const;

			/** @brief a から b まで、航行可能なセルだけを通る直線で行けるか */
			bool HasSeaLine(const Vector3& a, const Vector3& b) const;

			/**
			 * @brief a から b までの経路を探す（A*、8方向、角の斜め抜け禁止）
			 * @param outWaypoints 経由点（a は含まず b を含む。見通しで間引き済み）
			 * @param outLength 経路長
			 * @return 経路があれば true（a・b が航行可能で同じ連結域のときだけ）
			 */
			bool FindPath(const Vector3& a, const Vector3& b, std::vector<Vector3>& outWaypoints, float* outLength = nullptr) const;

			/**
			 * @brief 合成した地形で上の関数の約束を確かめる
			 * @param outMessage 失敗したときの内容
			 * @return 全部合格なら true
			 */
			static bool SelfTest(std::string& outMessage);


		public:
			/** @brief 条件 */
			const Settings& GetSettings() const { return m_settings; }
			/** @brief 1辺のセル数 */
			int GetSize() const { return m_size; }
			/** @brief 地図の左下（-X, -Z 側）のワールド座標 */
			float GetOrigin() const { return m_origin; }
			/** @brief セルが航行可能か（デバッグ表示用） */
			bool IsNavigableCell(int cx, int cz) const;
			/** @brief セルが海か（デバッグ表示用） */
			bool IsSeaCell(int cx, int cz) const;
			/** @brief 航行可能セルの数 */
			int GetNavigableCellCount() const { return m_navigableCount; }
			/** @brief 最も大きい連結域の番号 */
			int GetLargestComponent() const { return m_largestComponent; }


		private:
			/** @brief セル番号 */
			inline int Index(int cx, int cz) const { return cz * m_size + cx; }
			/** @brief セルが地図の中か */
			inline bool InGrid(int cx, int cz) const { return cx >= 0 && cz >= 0 && cx < m_size && cz < m_size; }
			/** @brief ワールド座標 → セル（地図の外も計算する） */
			void ToCell(float x, float z, int& cx, int& cz) const;
			/** @brief セルの中心のワールド座標 */
			Vector3 CellCenter(int cx, int cz) const;
			/** @brief 航行可能か（地図の外は false） */
			bool NavAt(int cx, int cz) const;
			/** @brief 経由点を見通しで間引く */
			void SmoothPath(const Vector3& start, std::vector<Vector3>& waypoints) const;


		private:
			Settings m_settings;
			/** 1辺のセル数 */
			int m_size = 0;
			/** 地図の -X, -Z 側の端 */
			float m_origin = 0.0f;
			/** 海セル */
			std::vector<uint8_t> m_sea;
			/** 航行可能セル */
			std::vector<uint8_t> m_navigable;
			/** 連結域の番号（航行可能でなければ -1） */
			std::vector<int> m_component;
			/** 航行可能セルの数 */
			int m_navigableCount = 0;
			/** 最も大きい連結域 */
			int m_largestComponent = -1;

			/** 経路探索の作業領域（毎回確保しないため使い回す） */
			mutable std::vector<float> m_pathCost;
			mutable std::vector<int> m_pathParent;
			mutable std::vector<uint32_t> m_pathStamp;
			mutable uint32_t m_pathSearchId = 0;
		};
	}
}
