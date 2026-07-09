/**
 * @file Formations.h
 * @brief 陣形インターフェースと全具体クラスの定義
 * @author 竹林
 */
#pragma once
#include <memory>
#include <vector>
#include "Source/Actor/Character/Penguin/Formation/Effect/FormationEffectChain.h"
#include "Source/Actor/Character/Penguin/Formation/Ult/IUltEffect.h"


namespace app
{
	namespace actor
	{
		struct MasterFormationParameter;

		/**
		 * @brief 陣形インターフェース
		 * @details
		 *   CalculatePositions() でフォロワー数分の座標を計算すると
		 *   m_outerRadius が更新され、GetJoinRadius() が
		 *   現在の人数に応じた入隊判定半径を返す。
		 *
		 *   m_passive:   常時有効なエフェクトチェーン（ロジックのみ）
		 *   m_ult:       ウルト発動中のみ有効なエフェクトチェーン（ロジックのみ）
		 *   m_ultVisual: ウルトの演出（ビジュアルエフェクト）。効果とは分離して所有する。
		 *   FormationController がパッシブ・ウルトの2本を組み合わせて速度倍率・渦潮耐性を返し、
		 *   UltController が m_ult と m_ultVisual の両方に Enter/Update/Exit を転送する。
		 */
		class IFormation
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param param JSONから読み込んだこの陣形のパラメーター
			 * @details 定義は Formations.cpp（MasterFormationParameter の完全な定義が必要なため）
			 */
			explicit IFormation(const MasterFormationParameter& param);

			virtual ~IFormation() = default;

			/** @brief パッシブエフェクトチェーンを返す */
			const FormationEffectChain& GetPassive() const { return m_passive; }

			/** @brief ウルトエフェクトチェーンを返す（UltController が Enter/Update/Exit を呼ぶ） */
			FormationEffectChain& GetUlt() { return m_ult; }

			/**
			 * @brief ウルトの演出を返す（UltController が Enter/Update/Exit を呼ぶ）
			 * @return 演出インスタンス。演出を持たない陣形は nullptr
			 */
			IUltEffect* GetUltVisual() { return m_ultVisual.get(); }

			/**
			 * @brief ウルト持続時間を返す（秒）
			 * @details パラメーターを直接参照するのでホットリロードが即座に反映される。
			 *          定義は Formations.cpp（MasterFormationParameter の完全な定義が必要なため）
			 */
			float GetUltDuration() const;

			/**
			 * @brief ウルトクールダウンを返す（秒）
			 * @details パラメーターを直接参照するのでホットリロードが即座に反映される。
			 *          定義は Formations.cpp（MasterFormationParameter の完全な定義が必要なため）
			 */
			float GetUltCooldown() const;

			/** @brief 最外半径（CalculatePositions後に有効） */
			float GetOuterRadius() const { return m_outerRadius; }

			/** @brief 最外半径を直接セットする（save/restore用） */
			void  SetOuterRadius(float r) { m_outerRadius = r; }

			/**
			 * @brief 入隊判定半径（最外半径 + 入隊マージン）を返す
			 * @details パラメーターを直接参照するのでホットリロードが即座に反映される。
			 *          定義は Formations.cpp（MasterFormationParameter の完全な定義が必要なため）
			 */
			float GetJoinRadius() const;

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
			float m_outerRadius = 0.0f;  /** 最外半径 */

			FormationEffectChain m_passive;  /** 常時有効なエフェクトチェーン（ロジックのみ） */
			FormationEffectChain m_ult;      /** ウルト発動中のみ有効なエフェクトチェーン（ロジックのみ） */

			std::unique_ptr<IUltEffect> m_ultVisual;  /** ウルトの演出（ビジュアルエフェクト） */

			const MasterFormationParameter* m_param;  /** 陣形パラメーターへの非所有ポインタ（ホットリロード対応） */
		};




		/****************************************/


		/**
		 * @brief リング陣の共通基底クラス
		 * @details
		 *   リング k（1始まり）に baseFollowers*k 体を等間隔配置する。
		 *   各リングの半径は radiusPerRing*k。全リングで隣接間隔が均一になる。
		 *   Circle・Cluster・Scatter はこのクラスを継承し、パラメーターだけ変える。
		 */
		class RingFormation : public IFormation
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param param JSONから読み込んだこの陣形のパラメーター（baseFollowers/radiusPerRing を使用）
			 * @details 定義は Formations.cpp（MasterFormationParameter の完全な定義が必要なため）
			 */
			explicit RingFormation(const MasterFormationParameter& param);

			/** @brief IFormation::GetJoinRadius() （引数なし）が隠蔽されないようにする */
			using IFormation::GetJoinRadius;

			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count
			) override;

			/**
			 * @brief count 人のときの次入隊リングから入隊半径を計算する
			 * @details パラメーターを直接参照するのでホットリロードが即座に反映される。
			 *          定義は Formations.cpp（MasterFormationParameter の完全な定義が必要なため）
			 */
			float GetJoinRadius(int count) const override;
		};




		/****************************************/


		/**
		 * @brief 円陣（標準間隔）
		 * @details
		 *   パッシブ: レベル連動速度（他陣形と共通、MasterFormationParameter で調整）。
		 *   ウルト: 速度1.5倍 ＋ 渦潮免疫 ＋ ペンギン呼び出し（MasterFormationParameter で調整）。
		 *   演出: UltEffectCircle（NormalUltAura）。
		 */
		class CircleFormation : public RingFormation
		{
		public:
			/** @param param JSONから読み込んだ円陣のパラメーター */
			explicit CircleFormation(const MasterFormationParameter& param);
		};




		/****************************************/


		/**
		 * @brief 密集陣（狭間隔）
		 * @details
		 *   パッシブ: レベル連動速度（他陣形と共通）＋ 渦潮耐性（MasterFormationParameter で調整）。
		 *   ウルト: 渦潮近傍で速度up ＋ シロクマ攻撃無効化（MasterFormationParameter で調整）。
		 *   演出: UltEffectCluster（Barrier）。
		 */
		class ClusterFormation : public RingFormation
		{
		public:
			/** @param param JSONから読み込んだ密集陣のパラメーター */
			explicit ClusterFormation(const MasterFormationParameter& param);
		};




		/****************************************/


		/**
		 * @brief 散開陣（広間隔）
		 * @details
		 *   パッシブ: レベル連動速度（他陣形と共通、MasterFormationParameter で調整）。
		 *   ウルト: ペンギン呼び出し（MasterFormationParameter で調整）。
		 *   演出: UltEffectScatter（CallAura）。
		 */
		class ScatterFormation : public RingFormation
		{
		public:
			/** @param param JSONから読み込んだ散開陣のパラメーター */
			explicit ScatterFormation(const MasterFormationParameter& param);
		};




		/****************************************/


		/**
		 * @brief 三角陣
		 * @details
		 *   パッシブ: レベル連動速度（初期値のみ他陣形より高い。伸び率は他陣形と共通、MasterFormationParameter で調整）。
		 *   ウルト: 速度1.5倍（MasterFormationParameter で調整）。
		 *   演出: UltEffectTriangle（SpeedBoost）。
		 */
		class TriangleFormation : public IFormation
		{
		public:
			/** @param param JSONから読み込んだ三角陣のパラメーター */
			explicit TriangleFormation(const MasterFormationParameter& param);

			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count
			) override;
		};
	}
}
