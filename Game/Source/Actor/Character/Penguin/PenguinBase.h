/**
 * @file PenguinBase.h
 * @brief ペンギンの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{
		class PenguinEffectStatus;


		/**
		 * @brief ペンギンの基底クラス
		 */
		class PenguinBase : public CharacterBase
		{
		public:
			PenguinBase();
			virtual ~PenguinBase() override = default;


			/**
			 * @brief エフェクトステータスを取得
			 * @return エフェクトステータスのポインタ
			 */
			inline PenguinEffectStatus* GetEffectStatus() const { return m_effectStatus.get(); }

			/**
			 * @brief ペンギンの足跡のサイズを取得
			 * @return デフォルトの足跡のサイズ
			 */
			virtual float GetFootprintSize() const { return 12.0f; }

			/**
			 * @brief ペンギンの足跡の優先度を取得
			 * @return 足跡の優先度（大きいほど優先度が高い）
			 */
			virtual int GetFootprintPriority() const { return 1; }

		protected:
			virtual void Start() override;
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;

			/**
			 * @brief スライド中に地形法線に沿ってモデルを傾ける描画補正を行う
			 * @details 物理・ステート判定には影響を与えない。
			 *          DaddyPenguin・ChildPenguin 共通の処理をここに集約する。
			 */
			void UpdateSlideTilt();

			/**
			 * @brief 足跡の描画を更新する
			 */
			void UpdateFootprints();

		protected:
			/** スライド中の地形傾斜に合わせたモデル描画用回転（補間済み） */
			Quaternion m_slideModelRotation;

			std::unique_ptr<PenguinEffectStatus> m_effectStatus;

			Vector3 m_lastFootprintPos = Vector3::Zero;
			bool m_isRightFoot = true;
		};
	}
}