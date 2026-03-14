/**
 * @file DaddyPenguinParameter.h
 * @brief 親ペンギンのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 親ペンギンパラメーター
		 * @details 親ペンギン固有のパラメーターを保持する
		 */
		struct MasterDaddyPenguinParameter : public MasterCharacterParameter
		{
			appParameter(MasterDaddyPenguinParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// 親ペンギン固有のパラメーターをここに追加していく
			/** スニークの速さ */
			float sneakSpeed;
		};
	}
}

