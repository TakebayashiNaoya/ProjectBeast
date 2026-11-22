/**
 * @file FormationEffects.cpp
 * @brief 陣形効果の具体クラス群の実装
 */
#include "stdafx.h"
#include "FormationEffects.h"
#include "Source/Manager/BattleManager.h"


namespace app
{
	namespace actor
	{
		void BearAttackNullifyEffect::Enter(const FormationContext& ctx)
		{
			/** 無効化の判定と反撃（ノックバック＋スタン）は EnemyController の
			 *  攻撃ヒット処理側で行う。ここではフラグを立てるだけ */
			BattleManager::GetInstance().SetBearAttackNullified(true);
		}


		void BearAttackNullifyEffect::Exit(const FormationContext& ctx)
		{
			BattleManager::GetInstance().SetBearAttackNullified(false);
		}
	}
}
