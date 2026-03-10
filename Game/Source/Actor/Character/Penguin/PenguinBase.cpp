/**
 * @file PenguinBase.cpp
 * @brief ペンギンの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "PenguinBase.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			/** アニメーションの情報 */
			const AnimationData ANIMATION_DATA[] =
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



			/** モデルの情報 */
			const ModelData MODEL_DATA[] =
			{
				{
					"Assets/modelData/penguin/daddyPenguin/DaddyPenguin.tkm",
					ANIMATION_DATA,
					EnModelUpAxis::enModelUpAxisZ,
					std::size(ANIMATION_DATA)
				},
				{
					"Assets/modelData/penguin/childPenguin/ChildPenguin.tkm",
					ANIMATION_DATA,
					EnModelUpAxis::enModelUpAxisZ,
					std::size(ANIMATION_DATA)
				}
			};
		}


		void PenguinBase::Start()
		{
			CharacterBase::Start();
		}


		void PenguinBase::Update()
		{
			CharacterBase::Update();
		}


		void PenguinBase::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}


		void PenguinBase::Init(const EnPenguinType penguinType)
		{
			// ペンギンの種類を数値に変換
			const uint8_t type = static_cast<uint8_t>(penguinType);
			// モデルデータを初期化
			CharacterBase::Init(MODEL_DATA[type]);
		}
	}
}