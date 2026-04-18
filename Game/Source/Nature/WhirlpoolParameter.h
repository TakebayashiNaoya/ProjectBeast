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

			float whirlpoolRadius;		/** 渦潮の影響範囲半径 */
			float attractSpeed;			/** 引き寄せ速度（半径方向） */
			float pushSpeed;			/** 押し出し速度（半径方向） */
			float rotateSpeed;			/** 渦巻き回転速度（ラジアン/秒） */
			float attractThreshold;		/** 引き寄せ完了とみなす中心からの距離 */
			float uvRotationSpeed;		/** UV回転速度（ラジアン/秒） */
			float scaleChangeTime;		/** 渦潮の拡大率の変化にかかる時間 */
			float stayTime;				/** 渦潮の拡大率が最大値で留まる時間 */
			float createInterval;		/** 渦潮の生成間隔 */
		};
	}
}
