/**
 * @file PenguinEffectParameter.h
 * @brief ペンギンのエフェクトパラメーター管理
 * @author 立山
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ペンギンエフェクトパラメーター
		 * @details ペンギンのエフェクトに関連するパラメーターを保持する
		 */
		struct MasterPenguinEffectParameter : public core::IMasterParameter
		{
			appParameter(MasterPenguinEffectParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			/** 泳ぎエフェクトの基本スケール */
			Vector3 splashEffectScale;

			/** エフェクト発生位置の前方オフセット */
			float effectOffsetForward;

			/** 泳ぎエフェクトの再生間隔 */
			float splashEffectInterval;

			/** 泳ぎエフェクトを出すための最低速度の二乗 */
			float minMoveVelocitySq;

			/** 泳ぎエフェクトの最小スケールの割合 */
			float minSplashScaleRatio;

			/** 泳ぎエフェクトの最大スケールの割合 */
			float maxSplashScaleRatio;

			/** 泳ぎエフェクトのスケールを決めるための最低速度 */
			float minSpeed;

			/** 泳ぎエフェクトのスケールを決めるための最高速度 */
			float maxSpeed;

			/** 着地エフェクトのスケール */
			Vector3 landingEffectScale;

			/** スライドエフェクトのスケール */
			Vector3 slideEffectScale;

			/** スライドのエフェクトの再生間隔 */
			float slideEffectInterval;
		};
	}
}