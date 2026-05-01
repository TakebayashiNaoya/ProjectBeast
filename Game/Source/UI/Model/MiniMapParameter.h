/**
 * @file MiniMapParameter.h
 * @brief ミニマップ専用パラメータークラス
 * @author 忽那
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ミニマップ専用パラメーター
		 */
		struct MiniMapParameter : public core::IMasterParameter
		{
			appParameter(MiniMapParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */
			float mapRadius;
			float mapLimitDistance;
			Vector3 mapCenterPos;
		};
	}
}


