/**
 * @file MasterBearReactionParameter.h
 * @brief BearReactionのパラメーター管理クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief BearReactionのパラメーター管理クラス
		 */
		struct MasterBearReactionParameter : public core::IMasterParameter
		{
			appParameter(MasterBearReactionParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */

			/** アイコンのXオフセット */
			float offsetX;
			/** アイコンのYオフセット */
			float offsetY;
			/** アイコンの有効距離 */
			float activeDistance;
		};
	}
}


