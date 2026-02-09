/**
 * @file PlayerParameter.h
 * @brief プレイヤーのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief プレイヤーパラメーター
		 * @details プレイヤー固有のパラメーターを保持する
		 */
		struct MasterPlayerParameter : public MasterCharacterParameter
		{
			appParameter(MasterPlayerParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// プレイヤー固有のパラメーターをここに追加していく
		};
	}
}

