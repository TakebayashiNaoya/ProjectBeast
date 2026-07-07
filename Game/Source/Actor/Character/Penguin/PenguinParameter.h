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
			/** 最大体力 */
			int maxHp;
			/** 初期体力 */
			int hp;
			/** スニークの速さ */
			float sneakSpeed;
			/** スライドの速さ */
			float slideSpeed;
			/** ジャンプパワー */
			float jumpPower;
			/** ジャンプのスタミナ最大値 */
			float jumpStaminaMax;
			/** ジャンプのスタミナ回復速度(1秒あたり) */
			float jumpStaminaRecoverSpeed;
			/** スライドのスタミナ最大値 */
			float slideStaminaMax;
			/** スライドのスタミナ減少速度(1秒あたり) */
			float slideStaminaDecreaseSpeed;
			/** スライドのスタミナ回復速度(1秒あたり) */
			float slideStaminaRecoverSpeed;
		};
	}
}