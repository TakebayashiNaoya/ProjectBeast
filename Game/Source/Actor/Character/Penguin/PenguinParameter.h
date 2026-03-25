/**
 * @file PenguinParameter.h
 * @brief ペンギンのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ペンギンパラメーター
		 * @details ペンギン共通のパラメーターを保持する
		 */
		struct MasterPenguinParameter : public MasterCharacterParameter
		{
			appParameter(MasterPenguinParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// ペンギン固有のパラメーターをここに追加していく
			/** スニークの速さ */
			float sneakSpeed;
			/** スライドの速さ */
			float slideSpeed;
			/** ジャンプパワー */
			float jumpPower;
		};
	}
}

