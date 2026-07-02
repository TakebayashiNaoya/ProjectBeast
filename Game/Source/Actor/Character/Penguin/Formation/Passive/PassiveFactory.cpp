/**
 * @file PassiveFactory.cpp
 * @brief 陣形ごとのパッシブ効果を組み立てるファクトリー
 * @author 竹林
 */
#include "stdafx.h"
#include "PassiveFactory.h"
#include "FormationPassiveDecorators.h"


namespace app
{
	namespace actor
	{
		std::unique_ptr<IFormationPassive> PassiveFactory::CreateCirclePassive()
		{
			return std::make_unique<BasePassive>();
		}


		std::unique_ptr<IFormationPassive> PassiveFactory::CreateDefensePassive()
		{
			// 速度 0.8x → 渦潮無効化 の順でラップ
			auto p = std::make_unique<BasePassive>();
			auto withSpeed = std::make_unique<SpeedModifierPassiveDecorator>(std::move(p), 0.8f);
			return std::make_unique<WhirlpoolResistancePassiveDecorator>(std::move(withSpeed));
		}


		std::unique_ptr<IFormationPassive> PassiveFactory::CreateTrianglePassive()
		{
			// 速度 = 1.0 + level × 0.1
			auto p = std::make_unique<BasePassive>();
			return std::make_unique<SpeedModifierPassiveDecorator>(std::move(p), 1.0f, 0.1f);
		}


		std::unique_ptr<IFormationPassive> PassiveFactory::CreateScatterPassive()
		{
			return std::make_unique<BasePassive>();
		}
	}
}
