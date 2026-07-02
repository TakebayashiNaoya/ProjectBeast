/**
 * @file Formations.h
 * @brief 陣形インターフェースと全具体クラスの定義
 * @author 竹林
 */
#pragma once
#include <vector>


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形インターフェース
		 * @details
		 *   CalculatePositions() でフォロワー数分の座標を計算すると
		 *   m_outerRadius が更新され、GetJoinRadius() が
		 *   現在の人数に応じた入隊判定半径を返す。
		 *   レベル管理は FormationController が担う。
		 */
		class IFormation
		{
		public:
			/** @brief デストラクタ */
			virtual ~IFormation() = default;

			/**
			 * @brief 移動速度倍率を返す
			 * @param level FormationController が管理する現在の陣形レベル
			 */
			virtual float GetSpeedMultiplier(int level) const = 0;

			/** @brief 最外半径（CalculatePositions後に有効） */
			inline float GetOuterRadius()  const { return m_outerRadius; }
			/** @brief 最外半径を直接セットする（save/restore用） */
			inline void  SetOuterRadius(float r) { m_outerRadius = r; }

			/** @brief 入隊判定半径（最外半径 + 入隊マージン） */
			inline float GetJoinRadius()   const { return m_outerRadius + m_joinMargin; }

			/** @brief 渦潮耐性パッシブを持つか（ディフェンス陣形のみtrue） */
			virtual bool HasWhirlpoolResistance() const { return false; }

			/**
			 * @brief 指定フォロワー数に対応する入隊判定半径を返す
			 * @param count フォロワー数
			 */
			virtual float GetJoinRadius(int count) const { return GetJoinRadius(); }

			/**
			 * @brief 陣形座標を計算する
			 * @param center  親ペンギンの座標
			 * @param forward 親ペンギンの前方向（正規化済み）
			 * @param out     計算結果を追記するベクター（呼び出し元がclear済みであること）
			 * @param count   配置するペンギンの数（= 現在のフォロワー数）
			 */
			virtual void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count
			) = 0;


		protected:
			float m_outerRadius = 0.0f;   /** 最外半径 */
			float m_joinMargin  = 20.0f;  /** 入隊判定マージン */
		};




		/****************************************/


		/**
		 * @brief リング陣の共通基底クラス
		 * @details
		 *   リング k（1始まり）に baseFollowers*k 体を等間隔配置する。
		 *   各リングの半径は radiusPerRing*k。全リングで隣接間隔が均一になる。
		 *   Circle・Defense・Scatter はこのクラスを継承し定数だけ変える。
		 */
		class RingFormation : public IFormation
		{
		public:
			RingFormation(int baseFollowers, float radiusPerRing)
				: m_baseFollowers(baseFollowers)
				, m_radiusPerRing(radiusPerRing)
			{}

			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count
			) override;

			/** @brief count 人のときの次入隊リングから入隊半径を計算する */
			float GetJoinRadius(int count) const override
			{
				// リング k の収容数は baseFollowers*k（k=1:9, k=2:18, ...）
				// count 人が埋まったとき、次の入隊先リングを求める
				int ring       = 1;
				int cumulative = 0;
				while (cumulative + m_baseFollowers * ring <= count)
				{
					cumulative += m_baseFollowers * ring;
					++ring;
				}
				return m_radiusPerRing * ring + m_joinMargin;
			}


		private:
			int   m_baseFollowers;  /** リング1の配置数（リングkは m_baseFollowers*k 体） */
			float m_radiusPerRing;  /** リングごとの半径増分 */
		};




		/****************************************/


		/**
		 * @brief 円陣（標準間隔）
		 * @details リング間隔: ~15単位。速度変化なし。
		 */
		class CircleFormation : public RingFormation
		{
		public:
			CircleFormation() : RingFormation(9, 22.0f) {}

			float GetSpeedMultiplier(int /*level*/) const override { return 1.0f; }
		};




		/****************************************/


		/**
		 * @brief 密集陣（狭間隔）
		 * @details リング間隔: ~5単位。速度が下がる代わりに防御効果を得られる。
		 */
		class DefenseFormation : public RingFormation
		{
		public:
			DefenseFormation() : RingFormation(9, 8.0f)
			{
				m_joinMargin = 10.0f;
			}

			float GetSpeedMultiplier(int /*level*/) const override { return 0.8f; }
			bool  HasWhirlpoolResistance()          const override { return true; }
		};




		/****************************************/


		/**
		 * @brief 散開陣（広間隔）
		 * @details リング間隔: ~28単位。入隊範囲が広くより多くのペンギンを取り込む。
		 */
		class ScatterFormation : public RingFormation
		{
		public:
			ScatterFormation() : RingFormation(9, 40.0f)
			{
				m_joinMargin = 50.0f;
			}

			float GetSpeedMultiplier(int /*level*/) const override { return 1.0f; }
		};




		/****************************************/


		/**
		 * @brief 三角陣
		 * @details
		 *   プレイヤーをボーリングの5番ピン位置（4行三角形の3行目中央）に置き、
		 *   外周を1層ずつ拡張する。
		 *
		 *   配置順 (P=プレイヤー):
		 *         ①                ← 先頭
		 *       ②   ③             ← 左上・右上
		 *     ④  [P]  ⑤           ← 左・右
		 *   ⑥  ⑦   ⑧  ⑨         ← 後方行
		 *
		 *   レベル L の三角形: 行数 = 3L+1, 総ピン数 = (3L+1)*(3L+2)/2
		 */
		class TriangleFormation : public IFormation
		{
		public:
			TriangleFormation()
			{
				m_joinMargin = 15.0f;
			}

			/**
			 * @brief 移動速度倍率を返す
			 * @return 1.0 + レベル × SPEED_PER_LEVEL
			 */
			float GetSpeedMultiplier(int level) const override
			{
				return BASE_SPEED + level * SPEED_PER_LEVEL;
			}

			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count
			) override;


		private:
			static constexpr float ROW_SPACING     = 15.0f;  /** 行間隔 */
			static constexpr float COL_SPACING     = 15.0f;  /** 列間隔 */
			static constexpr float BASE_SPEED      = 1.0f;   /** レベル0の速度倍率 */
			static constexpr float SPEED_PER_LEVEL = 0.1f;   /** レベルごとの速度ボーナス */
		};
	}
}
