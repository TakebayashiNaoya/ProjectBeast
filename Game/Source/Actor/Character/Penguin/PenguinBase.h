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
		/**
		 * @brief ペンギンの基底クラス
		 */
		class PenguinBase : public CharacterBase
		{
		public:
			PenguinBase() = default;
			virtual ~PenguinBase() override = default;


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


		protected:
			/** スライド中の地形傾斜に合わせたモデル描画用回転（補間済み） */
			Quaternion m_slideModelRotation;
		};
	}
}