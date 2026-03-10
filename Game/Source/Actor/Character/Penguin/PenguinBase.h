/**
 * @file PenguinBase.h
 * @brief ペンギンの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{


		/**
		 * @brief ペンギンの基底クラス
		 */
		class PenguinBase : public CharacterBase
		{
		public:
			/**
			 * @brief ペンギンの種類
			 */
			enum class EnPenguinType : uint8_t
			{
				// 親ペンギン
				Daddy,
				// 子ペンギン
				Child
			};


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
				TurnRight
			};


		public:
			PenguinBase() = default;
			virtual ~PenguinBase() override = default;


		protected:
			virtual void Start() override;
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		protected:
			/**
			 * @brief 初期化
			 * @param penguinType ペンギンの種類
			 */
			void Init(const EnPenguinType penguinType);
		};
	}
}

