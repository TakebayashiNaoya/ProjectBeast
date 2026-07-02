/**
 * @file Formations.h
 * @brief 陣形インターフェースと全具体クラスの定義
 * @author 竹林
 */
#pragma once
#include <memory>
#include <vector>
#include "Passive/IFormationPassive.h"
#include "Ult/IUltEffect.h"


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
		 *
		 *   速度倍率と渦潮耐性は m_passive（IFormationPassive）が担う。
		 *   ウルト効果は m_ult（IUltEffect）が担う。
		 */
		class IFormation
		{
		public:
			virtual ~IFormation() = default;

			/** 
			 * @brief パッシブ効果を返す
			 * @return パッシブ効果のインターフェース（非所有ポインタ）
			 */
			IFormationPassive* GetPassive() const { return m_passive.get(); }

			/** 
			 * @brief ウルト効果を返す
			 * @return ウルト効果のインターフェース（非所有ポインタ）
			 */
			IUltEffect* GetUlt() const { return m_ult.get(); }

			/**
			 * @brief 最外半径（CalculatePositions後に有効）
			 * @return 最外半径
			 */
			float GetOuterRadius() const { return m_outerRadius; }

			/** 
			 * @brief 最外半径を直接セットする（save/restore用）
			 * @param r 最外半径
			 */
			void SetOuterRadius(float r) { m_outerRadius = r; }

			/** 
			 * @brief 入隊判定半径（最外半径 + 入隊マージン）
			 * @return 入隊判定半径
			 */
			float GetJoinRadius() const { return m_outerRadius + m_joinMargin; }

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

			std::unique_ptr<IFormationPassive> m_passive;  /** 常時パッシブ効果 */
			std::unique_ptr<IUltEffect>        m_ult;      /** ウルト効果 */
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
		 * @details
		 *   リング間隔: ~15単位。
		 *   パッシブ: なし。
		 *   ウルト: 速度30%UP ＋ 渦潮免疫 ＋ ペンギン呼び出し（距離250）。
		 */
		class CircleFormation : public RingFormation
		{
		public:
			CircleFormation();
		};




		/****************************************/


		/**
		 * @brief 密集陣（狭間隔）
		 * @details
		 *   リング間隔: ~5単位。
		 *   パッシブ: 速度0.8x ＋ 渦潮無効化。
		 *   ウルト: 渦潮近傍で速度50%UP ＋ シロクマ攻撃無効化。
		 */
		class DefenseFormation : public RingFormation
		{
		public:
			DefenseFormation();
		};




		/****************************************/


		/**
		 * @brief 散開陣（広間隔）
		 * @details
		 *   リング間隔: ~28単位。入隊範囲が広くより多くのペンギンを取り込む。
		 *   パッシブ: なし。
		 *   ウルト: ペンギン呼び出し（距離600・広範囲回収特化）。
		 */
		class ScatterFormation : public RingFormation
		{
		public:
			ScatterFormation();
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
		 *
		 *   パッシブ: 速度 (1.0 + level × 0.1)x。
		 *   ウルト: 速度80%UP（純粋なスピード特化）。
		 */
		class TriangleFormation : public IFormation
		{
		public:
			TriangleFormation();

			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count
			) override;


		private:
			static constexpr float ROW_SPACING = 15.0f;  /** 行間隔 */
			static constexpr float COL_SPACING = 15.0f;  /** 列間隔 */
		};
	}
}
