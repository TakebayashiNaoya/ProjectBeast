/**
 * @file MasterPBWakingUpTimerParameter.h
 * @biref PB起床タイマー専用のパラメーター管理クラス
 * @author 忽那
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief PB起床タイマー専用のパラメーター管理クラス
		 * @note JSONから受け取る変数群は、PBWakingUpTimerStatusクラスのステータスの値を決定するために使用される
		 */
		struct MasterPBWakingUpTimerParameter : public core::IMasterParameter
		{
			appParameter(MasterPBWakingUpTimerParameter); 
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */
			float timerFirstValue;
			float timerSecondValue;
			float timserThirdValue;
			float timerFourthValue;
			float offsetValueY;
			float offsetValueX;
			Vector4 greenColor;
			Vector4 yellowColor;
			Vector4 redColor;
		};
	}
}


