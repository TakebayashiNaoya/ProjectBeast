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
			/** 歩行速度 */
			float walkSpeed;
			/** 最大食事数 */
			int maxEat;

			/** 最大スタミナ */
			float maxStamina;
			/** スタミナの消費量 */
			float staminaDrainRate;
			/** ペンギンを追うのを止める距離 */
			float lostChaseDistance;
		};
	}
}