/**
 * @file DaddyPenguin.cpp
 * @brief 親ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Util/CRC32.h"

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
				{ "Assets/animData/penguin/daddyPenguin/deathFaceDown.tka", false },
				// 仰向けで死亡する
				{ "Assets/animData/penguin/daddyPenguin/deathFaceUp.tka", false },
				// 食べ物をついばむ
				{ "Assets/animData/penguin/daddyPenguin/eatCrouching.tka", true },
				// 食べ物を飲み込む
				{ "Assets/animData/penguin/daddyPenguin/eatDrink.tka", false },
				// 食べるのをやめる
				{ "Assets/animData/penguin/daddyPenguin/eatRising.tka", false },
				// その場で翼をばたつかせる
				{ "Assets/animData/penguin/daddyPenguin/idleFlappingWings.tka", true },
				// その場で周りを見回す
				{ "Assets/animData/penguin/daddyPenguin/idleLooKAround.tka", true },
				// その場で体を振るう
				{ "Assets/animData/penguin/daddyPenguin/idleShake.tka", true },
				// その場で眠る
				{ "Assets/animData/penguin/daddyPenguin/idleSleep.tka", true },
				// その場で何もしない
				{ "Assets/animData/penguin/daddyPenguin/idleStanding.tka", true },
				// 走りながらジャンプする
				{ "Assets/animData/penguin/daddyPenguin/jumpRunning.tka", true },
				// 歩きながらジャンプする
				{ "Assets/animData/penguin/daddyPenguin/jumpWalking.tka", true },
				// 水中から出ようとする
				{ "Assets/animData/penguin/daddyPenguin/launchBegin.tka", false },
				// 水中から出て着地する
				{ "Assets/animData/penguin/daddyPenguin/launchEnd.tka", false },
				// 水中から出て空中にいるときに翼を速くばたつかせる
				{ "Assets/animData/penguin/daddyPenguin/launchFlappingWingsQuickly.tka", true },
				// 水中から出て空中にいるときに翼をゆっくりばたつかせる
				{ "Assets/animData/penguin/daddyPenguin/launchFlappingWingsSlowly.tka", true },
				// 走る
				{ "Assets/animData/penguin/daddyPenguin/moveRun.tka", true },
				// 泳ぐ
				{ "Assets/animData/penguin/daddyPenguin/moveSwim.tka", true },
				// 歩く
				{ "Assets/animData/penguin/daddyPenguin/moveWalk.tka", true },
				// 滑り始める
				{ "Assets/animData/penguin/daddyPenguin/slideBegin.tka", false },
				// 滑っている
				{ "Assets/animData/penguin/daddyPenguin/sliding.tka", true },
				// 起き上がる
				{ "Assets/animData/penguin/daddyPenguin/standUp.tka", false },
				// こける
				{ "Assets/animData/penguin/daddyPenguin/trip.tka", false },
				// 回れ右
				{ "Assets/animData/penguin/daddyPenguin/turnBack.tka", false },
				// 左向け左
				{ "Assets/animData/penguin/daddyPenguin/turnLeft.tka", false },
				// 右向け右
				{ "Assets/animData/penguin/daddyPenguin/turnRight.tka", false }
			};



			/** モデルの情報 */
			const ModelData MODEL_DATA =
			{
				"Assets/modelData/penguin/daddyPenguin/DaddyPenguin.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};
		}


		DaddyPenguin::DaddyPenguin()
		{
			m_stateMachine = std::make_unique<DaddyPenguinStateMachine>(this);
			m_status = std::make_unique<DaddyPenguinStatus>();
			m_status->Setup();
		}


		void DaddyPenguin::Start()
		{
			Init(MODEL_DATA);
			CharacterBase::Start();
		}


		void DaddyPenguin::Update()
		{
			m_stateMachine->Update();

			CharacterBase::Update();
		}


		void DaddyPenguin::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}