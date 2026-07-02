/**
 * @file UltFactory.cpp
 * @brief 陣形ごとのウルト効果を組み立てるファクトリー
 * @author 竹林
 */
#include "stdafx.h"
#include "UltFactory.h"
#include "UltEffectDecorators.h"


namespace app
{
	namespace actor
	{
		std::unique_ptr<IUltEffect> UltFactory::CreateCircleUlt()
		{
			// 速度30%UP → 渦潮免疫 → ペンギン呼び出し（距離250）
			std::unique_ptr<IUltEffect> e = std::make_unique<BaseUlt>();
			e = std::make_unique<SpeedBoostDecorator>		(std::move(e), 30.0f);
			e = std::make_unique<WhirlpoolImmunityDecorator>(std::move(e));
			e = std::make_unique<PenguinCallDecorator>		(std::move(e), 250.0f);
			return e;
		}


		std::unique_ptr<IUltEffect> UltFactory::CreateDefenseUlt()
		{
			// 渦潮近傍で速度50%UP → シロクマ攻撃無効化
			std::unique_ptr<IUltEffect> e = std::make_unique<BaseUlt>();
			e = std::make_unique<WhirlpoolSpeedBoostDecorator>(std::move(e), 50.0f);
			e = std::make_unique<BearAttackNullifyDecorator>  (std::move(e));
			return e;
		}

		std::unique_ptr<IUltEffect> UltFactory::CreateScatterUlt()
		{
			// ペンギン呼び出し（距離600）
			std::unique_ptr<IUltEffect> e = std::make_unique<BaseUlt>();
			return std::make_unique<PenguinCallDecorator>(std::move(e), 600.0f);
		}


		std::unique_ptr<IUltEffect> UltFactory::CreateTriangleUlt()
		{
			// 速度80%UP（純粋なスピード特化）
			std::unique_ptr<IUltEffect> e = std::make_unique<BaseUlt>();
			return std::make_unique<SpeedBoostDecorator>(std::move(e), 80.0f);
		}
	}
}
