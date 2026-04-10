/**
 * @file Types.h
 * @brief ステージに関する定義場所
 * @author 藤谷
 */
#pragma once
#include "Source/Util/Curve.h"


namespace
{
	/** 渦潮の回転速度 */
	constexpr float ROTATION_SPEED = 3.0f;

	/** 渦潮の拡大率の変化にかかる時間 */
	constexpr float SCALE_CHANGE_TIME = 5.0f;
	/** 渦潮の拡大率が最大値で留まる時間 */
	constexpr float WHIRLPOOL_STAY_TIME = 10.0f;
	/** 渦潮の最小値 */
	const Vector3 MIN_SCALE = Vector3(0.0f, 0.0f, 0.0f);
	/** 渦潮の最大値 */
	const Vector3 MAX_SCALE = Vector3(5.0f, 5.0f, 5.0f);
}