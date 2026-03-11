/**
 * @file EnemyParameter.h
 * @brief エネミーのパラメーター管理
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief エネミーパラメーター
		 * @details エネミー固有のパラメーターを保持する
		 */
		struct MasterEnemyParameter : public MasterCharacterParameter
		{
			appParameter(MasterEnemyParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// エネミー固有のパラメーターをここに追加していく
		};
	}
}

