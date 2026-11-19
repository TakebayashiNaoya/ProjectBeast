/**
 * @file OrcaTypes.h
 * @brief シャチで使う列挙型
 */
#pragma once


namespace app
{
	namespace actor
	{
		/**
		 * @brief シャチのアニメーションクリップ
		 * @details 並びは Orca.cpp の ANIMATION_DATA の登録順と一致させること
		 */
		enum class EnOrcaAnimationType : uint8_t
		{
			Swim,	// 泳ぐ（ループ）
			Mouth,	// 口を開けながら泳ぐ（噛みつきに使う。非ループ）
			Num
		};


		/**
		 * @brief シャチが泳ぐ深さの段階
		 * @details Y座標は「波面 − 深さ」で決まる。値はパラメーターで調整する
		 */
		enum class EnOrcaDepthLevel : uint8_t
		{
			Swim,	// 待機・巡回・探索・咆哮・追跡（背びれだけ水面に出る）
			Attack,	// 噛みつき（浮上し、頭を持ち上げて口を水面に出す）
		};
	}
}
