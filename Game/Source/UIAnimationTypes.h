/**
 * @file UIAnimatioTypes.h
 * @brief UIAnimationKeyの定義場所
 * @author 忽那
 */
#pragma once
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace animKey
		{
			/** PB起床タイマー専用のUIAnimationKey */
			constexpr uint32_t PB_CIRCLE_COLOR_FIRST_ANIM_KEY = Hash32("greenLerpAnim");
			constexpr uint32_t PB_CIRCLE_COLOR_SECOND_ANIM_KEY = Hash32("yellowLerpAnim");
			constexpr uint32_t PB_CIRCLE_COLOR_THIRD_ANIM_KEY = Hash32("redAnim");


			constexpr uint32_t PB_NEEDLE_ROT_ANIM_KEY = Hash32("rotationAnim");


			/** アチーブメント通知用のUIAnimationKey */
			constexpr uint32_t ACHIEVE_FADE_IN_ANIM_KEY = Hash32("fadeInAnim");
			constexpr uint32_t ACHIEVE_STAMP_ANIM_KEY = Hash32("stampAnim");
			constexpr uint32_t ACHIEVE_FADE_OUT_ANIM_KEY = Hash32("fadeOutAnim");


			/** 救助数(子ペンギン)のUIAnimationKey */
			constexpr uint32_t RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY = Hash32("rTlanslateUpAnim");
			constexpr uint32_t RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY = Hash32("rBounceDownAnim");

			constexpr uint32_t RESCUE_REMAIN_SINK_DOWN_ANIM_KEY = Hash32("rSinkDownAnim");
			constexpr uint32_t RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY = Hash32("rBounceDownUpAnim");

			constexpr uint32_t RESCUE_TOTAL_SINK_DOWN_ANIM_KEY = Hash32("tSinkDownAnim");
			constexpr uint32_t RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY = Hash32("tBounceDownUpAnim");

			constexpr uint32_t RESCUE_TOTAL_TLANSLATE_UP_ANIM_KEY = Hash32("tTlanslateUpAnim");
			constexpr uint32_t RESCUE_TOTAL_BOUNCE_DOWN_ANIM_KEY = Hash32("tBounceDownAnim");



			/** CPReaction用のUIAnimationKey */
			constexpr uint32_t CPREACTION_SWAY_ANIM_KEY = Hash32("swayAnimation");

			/** WpWarning用のUIAnimationKey */
			constexpr uint32_t WP_GROW_AND_SHRINK_ANIM_KEY = Hash32("growAndShrinkAnimation");


			/** リザルト画面用のUIAnimationKey */
			constexpr uint32_t SCORE_POPUP_FADE_IN_ANIM_KEY = Hash32("scorePopupFadeIn");
			constexpr uint32_t SCORE_POPUP_FADE_OUT_ANIM_KEY = Hash32("scorePopupFadeOut");
		}
	}
}