/**
 * @file Formations.h
 * @brief 陣形インターフェースと全具体クラスの定義
 * @author 竹林
 */
#pragma once
#include <vector>
#include "Effect/FormationEffectChain.h"


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
		 *
		 *   m_passive: 常時有効なエフェクトチェーン
		 *   m_ult:     ウルト発動中のみ有効なエフェクトチェーン
		 *   FormationController が2本を組み合わせて速度倍率・渦潮耐性を返す。
		 */
		class IFormation
		{
		public:
			virtual ~IFormation() = default;

			/** @brief パッシブエフェクトチェーンを返す */
			const FormationEffectChain& GetPassive() const { return m_passive; }

			/** @brief ウルトエフェクトチェーンを返す（UltController が Enter/Update/Exit を呼ぶ） */
			FormationEffectChain* GetUlt() { return &m_ult; }

			/** @brief 最外半径（CalculatePositions後に有効） */
			float GetOuterRadius() const { return m_outerRadius; }

			/** @brief 最外半径を直接セットする（save/restore用） */
			void  SetOuterRadius(float r) { m_outerRadius = r; }

			/** @brief 入隊判定半径（最外半径 + 入隊マージン） */
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

			FormationEffectChain m_passive;  /** 常時有効なエフェクトチェーン */
			FormationEffectChain m_ult;      /** ウルト発動中のみ有効なエフェクトチェーン */
		};




		/****************************************/


		/**
		 * @brief リング陣の共通基底クラス
		 * @details
		 *   リング k（1始まり）に baseFollowers*k 体を等間隔配置する。
		 *   各リングの半径は radiusPerRing*k。全リングで隣接間隔が均一になる。
		 *   Circle・Cluster・Scatter はこのクラスを継承し定数だけ変える。
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
			int   m_baseFollowers;
			float m_radiusPerRing;
		};




		/****************************************/


		/**
		 * @brief 円陣（標準間隔）
		 * @details
		 *   パッシブ: なし。
		 *   ウルト: 速度1.3x ＋ 渦潮免疫 ＋ ペンギン呼び出し（距離250）。
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
		 *   パッシブ: 速度0.8x ＋ 渦潮耐性。
		 *   ウルト: 渦潮近傍で速度1.5x ＋ シロクマ攻撃無効化。
		 */
		class ClusterFormation : public RingFormation
		{
		public:
			ClusterFormation();
		};




		/****************************************/


		/**
		 * @brief 散開陣（広間隔）
		 * @details
		 *   パッシブ: なし。
		 *   ウルト: ペンギン呼び出し（距離600）。
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
		 *   パッシブ: 速度 (1.0 + level × 0.1)x。
		 *   ウルト: 速度1.8x（純粋なスピード特化）。
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
			static constexpr float ROW_SPACING = 15.0f;
			static constexpr float COL_SPACING = 15.0f;
		};
	}
}
