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
		}
	}
}