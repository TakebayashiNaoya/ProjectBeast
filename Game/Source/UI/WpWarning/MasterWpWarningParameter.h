/**
 * @file MasterWpWarningParameter.h
 * @brief WpWarningのパラメーター管理クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief WpWarningのパラメーター管理クラス
		 */
		struct MasterWpWarningParameter : public core::IMasterParameter
		{
			appParameter(MasterWpWarningParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */
			float offsetY;
		};
	}
}


