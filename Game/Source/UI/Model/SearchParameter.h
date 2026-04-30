/**
 * @file SearchParameter.h
 * @brief シロクマの索敵・追跡に関するパラメータ
 * @author 忽那
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		struct SearchParameter : public core::IMasterParameter
		{
			appParameter(SearchParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			float offsetValueY;
			float dotValue;
			float iconPosY;
			float iconPosX;
			float iconPosZ;
			Vector3 offsetA;
			Vector3 offsetB;
		};
	}
}
