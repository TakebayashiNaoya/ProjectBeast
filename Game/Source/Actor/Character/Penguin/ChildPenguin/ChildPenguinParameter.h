/**
 * @file ChildPenguinParameter.h
 * @brief 子ペンギンのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンパラメーター
		 * @details 子ペンギン固有のパラメーターを保持する
		 */
		struct MasterChildPenguinParameter : public MasterCharacterParameter
		{
			appParameter(MasterChildPenguinParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// 子ペンギン固有のパラメーターをここに追加していく
		};
	}
}

