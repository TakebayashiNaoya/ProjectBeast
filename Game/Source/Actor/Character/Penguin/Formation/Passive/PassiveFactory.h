/**
 * @file PassiveFactory.h
 * @brief 陣形ごとのパッシブ効果を組み立てるファクトリー
 * @author 竹林
 */
#pragma once
#include <memory>
#include "IFormationPassive.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形パッシブのファクトリー
		 * @details
		 *   各陣形のコンストラクタから呼び出し、デコレーターチェーンを生成する。
		 *   効果の追加・変更はここだけ修正すればよい。
		 *
		 *   各陣形の構成:
		 *   - 円陣    : パッシブなし（BasePassive）
		 *   - 防御陣形 : 速度 0.8x ＋ 渦潮無効化
		 *   - 三角陣  : 速度 (1.0 + level × 0.1)x
		 *   - 散開陣  : パッシブなし（BasePassive）
		 */
		class PassiveFactory
		{
		public:
			static std::unique_ptr<IFormationPassive> CreateCirclePassive();
			static std::unique_ptr<IFormationPassive> CreateDefensePassive();
			static std::unique_ptr<IFormationPassive> CreateTrianglePassive();
			static std::unique_ptr<IFormationPassive> CreateScatterPassive();
		};
	}
}
