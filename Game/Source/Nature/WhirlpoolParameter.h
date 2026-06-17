/**
 * @file WhirlpoolParameter.h
 * @brief 渦潮のパラメーター
 * @author 竹林
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace nature
	{
		/**
		 * @brief 渦潮パラメーター
		 */
		struct MasterWhirlpoolParameter : public core::IMasterParameter
		{
			appParameter(MasterWhirlpoolParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			float whirlpoolRadius;			/** 渦潮の影響範囲半径 */
			float attractSpeed;				/** 引き寄せ速度（半径方向） */
			float rotateSpeed;				/** 渦巻き回転速度（ラジアン/秒） */
			float uvRotationSpeed;			/** UV回転速度（ラジアン/秒） */
			float scaleChangeTime;			/** 渦潮の拡大率の変化にかかる時間 */
			float stayTime;					/** 渦潮の拡大率が最大値で留まる時間 */
			float createInterval;			/** 渦潮の生成間隔 */
			float orbitRadius;				/** 子ペンギンが落ち着く中心からの軌道半径 */
			float orbitRadiusVariation;		/** 軌道半径のランダム変動最大幅（±この値） */
			float orbitOffsetVariation;		/** 個体ごとの軌道半径オフセットの最大幅（±この値） */
			float rotateScaleVariation;		/** 個体ごとの回転速度倍率の変動幅（1.0 ± この値） */
		};
	}
}