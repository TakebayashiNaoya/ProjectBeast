/**
 * @file MasterInGameButtonGaugeParameter.h
 * @brief インゲームボタンの再集合ゲージ専用のパラメーター管理クラス
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲームボタンの再集合ゲージ専用のパラメーター管理クラス
		 * @note JSONから受け取る変数群は、InGameButtonGaugeStatusクラスのステータスの値を決定するために使用される
		 */
		struct MasterInGameButtonGaugeParameter : public core::IMasterParameter
		{
			appParameter(MasterInGameButtonGaugeParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */
			/** 再集合ゲージの表示追従速度（1秒あたりに追従できる割合） */
			float regroupFollowSpeed;
		};
	}
}
