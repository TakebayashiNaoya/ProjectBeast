/**
 * @file LevelUpIconAnimStatus.h
 * @brief 陣形レベルアップアイコン専用のアニメーションステータスクラス
 */
#pragma once
#include "UIAnimationStatus.h"

#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 陣形レベルアップアイコン専用のアニメーションステータスクラス
		 * @details
		 *   コンストラクタでJSONをUIAnimationParameterへ読み込ませる役割を持つ。
		 *   これにより UIAnimationFactory::Attach<UIColorAnimation>(...) が
		 *   LEVELUP_ICON_FADE_IN/OUT_ANIM_KEY を解決できるようになる。
		 */
		class LevelUpIconAnimStatus : public UIAnimationStatus
		{
		public:
			LevelUpIconAnimStatus();
			~LevelUpIconAnimStatus();


			void SetUp() override;
			void Update() override;
		};
	}
}
