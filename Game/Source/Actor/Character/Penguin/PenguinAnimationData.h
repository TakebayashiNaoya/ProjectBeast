/**
 * @file PenguinAnimationData.h
 * @brief ペンギンのアニメーションデータ
 * @author 藤谷
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
			// うつ伏せで死亡する
			DeathFaceDown = 0,
			// 仰向けで死亡する
			DeathFaceUp,
			// 食べ物をついばむ
			EatCrouching,
			// 食べ物を飲み込む
			EatDrink,
			// 食べるのをやめる
			EatRising,
			// その場で翼をばたつかせる
			IdleFlapingWings,
			// その場で体を振るう
			IdleShake,
			// その場で眠る
			IdleSleep,
			// その場で周りを見回す
			IdleLoocAround,
			// その場で何もしない
			IdleStanding,
			// 走りながらジャンプする
			JumpRunning,
			// 歩きながらジャンプする
			JumpWalking,
			// 水中から出ようとする
			LaunchBegin,
			// 水中から出て着地する
			LaunchEnd,
			// 水中から出て空中にいるときに翼を速くばたつかせる
			LaunchFlapingWingsQuickly,
			// 水中から出て空中にいるときに翼をゆっくりばたつかせる				
			LaunchFkapingWingsSlowly,
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
			// 回れ右
			TurnBack,
			// 左向け左
			TurnLeft,
			// 右向け右
			TurnRight,
			Max
		};


		/** アニメーションの数 */
		constexpr uint8_t ANIMATION_NUM = static_cast<uint8_t>(EnPenguinAnimationID::Max);


		/** アニメーションの情報 */
		const AnimationData ANIMATION_DATA[ANIMATION_NUM] =
		{

			// うつ伏せで死亡する
			{ "Assets/animData/penguin/deathFaceDown.tka", false },
			// 仰向けで死亡する
			{ "Assets/animData/penguin/deathFaceUp.tka", false },
			// 食べ物をついばむ
			{ "Assets/animData/penguin/eatCrouching.tka", true },
			// 食べ物を飲み込む
			{ "Assets/animData/penguin/eatDrink.tka", false },
			// 食べるのをやめる
			{ "Assets/animData/penguin/eatRising.tka", false },
			// その場で翼をばたつかせる
			{ "Assets/animData/penguin/idleFlappingWings.tka", true },
			// その場で周りを見回す
			{ "Assets/animData/penguin/idleLooKAround.tka", true },
			// その場で体を振るう
			{ "Assets/animData/penguin/idleShake.tka", true },
			// その場で眠る
			{ "Assets/animData/penguin/idleSleep.tka", true },
			// その場で何もしない
			{ "Assets/animData/penguin/idleStanding.tka", true },
			// 走りながらジャンプする
			{ "Assets/animData/penguin/jumpRunning.tka", true },
			// 歩きながらジャンプする
			{ "Assets/animData/penguin/jumpWalking.tka", true },
			// 水中から出ようとする
			{ "Assets/animData/penguin/launchBegin.tka", false },
			// 水中から出て着地する
			{ "Assets/animData/penguin/launchEnd.tka", false },
			// 水中から出て空中にいるときに翼を速くばたつかせる
			{ "Assets/animData/penguin/launchFlappingWingsQuickly.tka", true },
			// 水中から出て空中にいるときに翼をゆっくりばたつかせる
			{ "Assets/animData/penguin/launchFlappingWingsSlowly.tka", true },
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
			{ "Assets/animData/penguin/trip.tka", false },
			// 回れ右
			{ "Assets/animData/penguin/turnBack.tka", false },
			// 左向け左
			{ "Assets/animData/penguin/turnLeft.tka", false },
			// 右向け右
			{ "Assets/animData/penguin/turnRight.tka", false }
		};
	}
}

