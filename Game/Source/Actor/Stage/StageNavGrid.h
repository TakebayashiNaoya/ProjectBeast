/**
 * @file StageNavGrid.h
 * @brief ハイトマップから作る歩行可否グリッド（簡易ナビゲーション）
 */
#pragma once
#include <vector>


namespace app
{
	namespace actor
	{
		/**
		 * @brief ハイトマップから作る歩行可否グリッド
		 * @details 地形生成時の頂点高さ配列を粗いセル（約40ワールド単位）へ落とし、
		 *          セルごとに「陸／水／通行不能（急斜面）」を分類する。
		 *          ペンギンは水を泳いで渡れるため、水セルは常に通行可能として扱い、
		 *          水と陸の行き来は浜（低い陸セル）だけに許す。
		 *          これにより「海からは登れない絶壁」がそのまま経路上の壁になる。
		 * @details 提供する機能は2つ。
		 *          - IsReachable()    : 2点間に経路があるか（A*）。徘徊先などの到達性チェック用
		 *          - フローフィールド : 目標（親ペンギン）へのダイクストラ場。全子ペンギンが共有する
		 */
		class StageNavGrid
		{
		public:
			StageNavGrid() = default;
			~StageNavGrid() = default;


		public:
			/**
			 * @brief 地形の頂点高さ配列からグリッドを構築する
			 * @details TerrainObject::GenerateMesh() が高さキャッシュを作った直後に呼ぶ。
			 *          追加のロードやレイキャストは行わない。
			 * @param heights    頂点高さ配列（yOffset適用前の生の高さ）
			 * @param vertexW    頂点グリッドの幅（頂点数）
			 * @param vertexH    頂点グリッドの奥行き（頂点数）
			 * @param totalWidth 地形X方向の総幅（ワールド単位）
			 * @param totalDepth 地形Z方向の総奥行き（ワールド単位）
			 * @param yOffset    地形全体のYオフセット
			 */
			void Build(
				const std::vector<float>& heights,
				const int vertexW,
				const int vertexH,
				const float totalWidth,
				const float totalDepth,
				const float yOffset
			);

			/**
			 * @brief 構築済みかどうかを取得
			 * @return 構築済みならtrue
			 */
			bool IsBuilt() const { return m_cellNum > 0; }

			/**
			 * @brief 2点間に通行可能な経路があるかを判定する（A*）
			 * @details 目的地が通行不能セル（急斜面）の場合は即座にfalseを返す。
			 *          出発地が通行不能セルの場合は近傍の通行可能セルから探索する。
			 * @param from 出発地点（ワールド座標）
			 * @param to   目的地点（ワールド座標）
			 * @return 経路があればtrue。グリッド未構築の場合もtrue（判定を諦めて通す）
			 */
			bool IsReachable(const Vector3& from, const Vector3& to) const;

			/**
			 * @brief 目標地点へのフローフィールドを構築する（ダイクストラ）
			 * @details 目標から全セルへの最短コストと「次に向かうべきセル」を計算する。
			 *          水セルは陸より低コスト（泳ぎのほうが速い）。
			 *          毎フレーム呼ぶものではない（0.5秒間隔を想定）。
			 * @param goalPos 目標地点（ワールド座標）
			 * @return 構築できたらtrue（目標が場外・グリッド未構築ならfalse）
			 */
			bool BuildFlowField(const Vector3& goalPos);

			/**
			 * @brief フローフィールドに沿った移動方向を取得する
			 * @param from   現在地点（ワールド座標）
			 * @param outDir 次に向かうべき方向（XZ平面、正規化済み）
			 * @return 取得できたらtrue（フロー未構築・現在地から目標へ到達不能ならfalse）
			 */
			bool GetFlowDirection(const Vector3& from, Vector3& outDir) const;


		private:
			/** セルの分類 */
			enum class EnCellType : uint8_t
			{
				Blocked,	///< 通行不能（急斜面）
				Land,		///< 陸（歩ける）
				Water,		///< 水（泳げる）
			};

			/**
			 * @brief ワールド座標からセル番号を求める
			 * @param pos ワールド座標
			 * @return セル番号。場外なら-1
			 */
			int CellIndexFromWorld(const Vector3& pos) const;

			/**
			 * @brief セル中心のワールド座標を求める
			 * @param cellIndex セル番号
			 * @return セル中心のワールド座標（Yはセルの通行高さ）
			 */
			Vector3 CellCenter(const int cellIndex) const;

			/**
			 * @brief 隣接セルへ移動できるかを判定する
			 * @param fromIndex 移動元セル番号
			 * @param toIndex   移動先セル番号
			 * @return 移動できればtrue
			 */
			bool IsConnected(const int fromIndex, const int toIndex) const;

			/**
			 * @brief 指定セルの近傍から通行可能なセルを探す
			 * @param cellIndex 起点セル番号
			 * @return 通行可能なセル番号。見つからなければ-1
			 */
			int FindNearbyPassableCell(const int cellIndex) const;

			/**
			 * @brief セル間の移動コストを求める
			 * @details 距離（斜め移動は√2倍）に地形係数（水は泳げるので割安）を掛ける
			 * @param fromIndex 移動元セル番号
			 * @param toIndex   移動先セル番号
			 * @return 移動コスト
			 */
			float MoveCost(const int fromIndex, const int toIndex) const;


		private:
			int   m_cellNumX  = 0;      /** X方向のセル数                     */
			int   m_cellNumZ  = 0;      /** Z方向のセル数                     */
			int   m_cellNum   = 0;      /** 総セル数（cellNumX * cellNumZ）   */
			float m_cellSizeX = 1.0f;   /** セルのX方向サイズ（ワールド単位） */
			float m_cellSizeZ = 1.0f;   /** セルのZ方向サイズ（ワールド単位） */
			float m_minX      = 0.0f;   /** グリッド左端のワールドX           */
			float m_minZ      = 0.0f;   /** グリッド手前端のワールドZ         */

			/** セルの分類（EnCellType） */
			std::vector<uint8_t> m_cellTypes;
			/** セルの通行高さ（ワールドY。水セルは海面0） */
			std::vector<float> m_cellHeights;

			/** フローフィールド：次に向かうべきセル番号（-1=無効） */
			std::vector<int> m_flowNext;
			/** フローフィールドが構築済みかどうか */
			bool m_isFlowFieldBuilt = false;
		};
	}
}
