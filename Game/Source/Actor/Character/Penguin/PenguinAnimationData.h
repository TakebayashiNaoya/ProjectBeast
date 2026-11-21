/**
 * @file PenguinAnimationData.h
 * @brief ペンギンのアニメーションデータ
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief アニメーションの種類
		 */
		enum class EnPenguinAnimationID : uint8_t
		{
			// 命令を出す
			CommandShout = 0,
			// うつ伏せで死亡する
			DeathFaceDown,
			// その場で何もしない
			IdleStanding,
			// 歩きながらジャンプする
			JumpWalking,
			// 水中から出ようとする
			LaunchBegin,
			// 走る
			MoveRun,
			// 泳ぐ
			MoveSwim,
			// 歩く
			MoveWalk,
			// 滑り始める
			SlideStart,
			// 滑っている
			Sliding,
			// 起き上がる
			StandUp,
			// こける
			Trip,
			Max
		};


		/** アニメーションの情報 */
		const AnimationData ANIMATION_DATA[static_cast<uint8_t>(EnPenguinAnimationID::Max)] =
		{
			// 命令を出す
			{ "Assets/animData/penguin/commandShout.tka", false },
			// うつ伏せで死亡する
			{ "Assets/animData/penguin/deathFaceDown.tka", false },
			// その場で何もしない
			{ "Assets/animData/penguin/idleStanding.tka", true },
			// 歩きながらジャンプする
			{ "Assets/animData/penguin/jumpWalking.tka", true },
			// 水中から出ようとする
			{ "Assets/animData/penguin/launchBegin.tka", false },
			// 走る
			{ "Assets/animData/penguin/moveRun.tka", true },
			// 泳ぐ
			{ "Assets/animData/penguin/moveSwim.tka", true },
			// 歩く
			{ "Assets/animData/penguin/moveWalk.tka", true },
			// 滑り始める
			{ "Assets/animData/penguin/slideBegin.tka", false },
			// 滑っている
			{ "Assets/animData/penguin/sliding.tka", true },
			// 起き上がる
			{ "Assets/animData/penguin/standUp.tka", false },
			// こける
			{ "Assets/animData/penguin/trip.tka", false }
		};
	}
}

