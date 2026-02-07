/**
 * @file CharacterParameter.h
 * @brief キャラクターのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief プレイヤーパラメーター
		 * @details プレイヤー固有のパラメーターを保持する
		 */
		struct MasterCharacterParameter : public core::IMasterParameter
		{
			appParameter(MasterCharacterParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD


			/** 最大体力 */
			int maxHp;
			/** 初期体力 */
			int hp;
			/** 歩行速度 */
			float walkSpeed;
			/** 走行速度 */
			float runSpeed;
			/** 体の半径 */
			float radius;
			/** 体の高さ */
			float height;
		};
	}
}

