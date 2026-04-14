/**
 * @file DaddyPenguinIState.cpp
 * @brief 親ペンギンのステートインターフェース
 * @author 藤谷
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "DaddyPenguinStatus.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Actor/Character/Penguin/PenguinStateMachine.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Camera/LoseCamera.h"
#include "Source/Camera/WinCamera.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace actor
	{

		DaddyPenguinIState::DaddyPenguinIState(DaddyPenguinStateMachine* owner)
			: m_owner(owner)
			, m_seHandle(-1)
		{}




		/************************************/


		namespace
		{
			const Vector3 EFFECT_SCALE = Vector3(70.0f, 70.0f, 70.0f);
		}


		void DaddyPenguinCommandShoutState::Enter()
		{
			// 1. マネージャーを介して子ペンギンへの命令を切り替える（トグル）
			auto* childPenMan = ChildPenguinManager::GetInstance();
			childPenMan->ToggleCommand();

			auto* effect = &EffectManager::Get();
			auto* sound = &SoundManager::Get();

			// 2. 切り替わった後の命令を取得
			auto currentCommand = childPenMan->GetCommand();

			// 3. 命令に応じて演出（アニメーションやエフェクト）を分岐
			if (currentCommand == ChildPenguinManager::EnPenguinCommand::Follow)
			{
				// === 「おいで！（追従）」の演出 ===
				m_owner->PlayAnimation(EnPenguinAnimationID::CommandShout);
				effect->PlayEffect(EnEffectKind::DaddyPenguinCommand, m_owner->GetTransform().m_position, Quaternion::Identity, EFFECT_SCALE);
				sound->PlaySE(enSoundKind::enSoundKind_DaddyPenguinShoutFollow, false);
				sound->PlaySE(enSoundKind::enSoundKind_DaddyPenguinSystemFollow, false);
			}
			else if (currentCommand == ChildPenguinManager::EnPenguinCommand::Wait)
			{
				// === 「待て！（待機）」の演出 ===
				// NOTE: 現状は同じ設定を入れていますが、待機用のアニメーションやエフェクト（例: EnPenguinAnimationID::CommandWait など）があればここを変更してください。
				m_owner->PlayAnimation(EnPenguinAnimationID::CommandShout);
				effect->PlayEffect(EnEffectKind::DaddyPenguinCommand, m_owner->GetTransform().m_position, Quaternion::Identity, EFFECT_SCALE);
				sound->PlaySE(enSoundKind::enSoundKind_DaddyPenguinShoutWait, false);
				sound->PlaySE(enSoundKind::enSoundKind_DaddyPenguinSystemWait, false);
			}
		}


		void DaddyPenguinCommandShoutState::Update()
		{}


		void DaddyPenguinCommandShoutState::Exit()
		{}


		DaddyPenguinCommandShoutState::DaddyPenguinCommandShoutState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinWinState::Enter()
		{
			float ry = m_owner->GetTransform().m_rotation.y;

			Vector3 front;
			front.x = sinf(ry);
			front.y = 0.0f;
			front.z = cosf(ry);


			// ターゲットを親ペンギンの座標に設定
			camera::CameraManager::Get().GetController<camera::WinCamera>(camera::WinCamera::ID())->SetTarget(m_owner->GetTransform().m_position, front);

			// 勝利カメラに切り替え
			camera::CameraManager::Get().SwitchCamera(camera::WinCamera::ID(), 1.0f);
		}


		void DaddyPenguinWinState::Update()
		{}


		void DaddyPenguinWinState::Exit()
		{}


		DaddyPenguinWinState::DaddyPenguinWinState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinLoseState::Enter()
		{
			// ターゲットを親ペンギンの座標に設定
			camera::CameraManager::Get().GetController<camera::LoseCamera>(camera::LoseCamera::ID())->SetTarget(m_owner->GetTransform().m_position);

			// 負けカメラに切り替え
			camera::CameraManager::Get().SwitchCamera(camera::LoseCamera::ID(), 1.0f);
		}


		void DaddyPenguinLoseState::Update()
		{
			m_timer += g_gameTime->GetFrameDeltaTime();

			if (m_timer >= 2.5f)
			{
				// ターゲットを親ペンギンの座標に設定
				camera::CameraManager::Get().GetController<camera::DefeatCamera>(camera::DefeatCamera::ID())->SetTarget(m_owner->GetTransform().m_position);
				// 負けカメラに切り替え
				camera::CameraManager::Get().SwitchCamera(camera::DefeatCamera::ID(), 1.0f);
				m_timer = 0.0f;
			}
		}


		void DaddyPenguinLoseState::Exit()
		{}


		DaddyPenguinLoseState::DaddyPenguinLoseState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}




		/************************************/


		void DaddyPenguinEnterIglooState::Enter()
		{
			// 1. 親ペンギンの現在位置を基準に最も近いイグルーの座標と回転を取得
			const Vector3 myPos = m_owner->GetTransform().m_position;
			Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
			Quaternion iglooRot = StageSystem::GetInstance()->GetNearestIglooRotation(myPos);

			// 2. 正面ベクトルを計算する
			// まず基準となるZ軸方向（正面）のベクトルを作成します
			Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);

			// iglooRot の Apply関数に forwardVec を渡して、かまくらの向きに回転させます
			// ※この一行を実行すると、forwardVec の中身が自動的に書き換わります
			iglooRot.Apply(forwardVec);

			// 3. 入り口の座標を算出
			float offsetDistance = 170.0f;
			m_entrancePos = iglooPos + (forwardVec * offsetDistance);
			Vector3 insidePos = iglooPos; // かまくらの中心座標

			// 4. 子ペンギンたちにイベント開始を通知
			// ※事前に ChildPenguinManager に StartIglooEvent 関数を追加しておいてください
			//ChildPenguinManager::GetInstance()->StartIglooEvent(m_entrancePos);

			// 5. 親ペンギン自身の状態をセット
			m_isArrivedEntrance = false;
			m_owner->PlayAnimation(EnPenguinAnimationID::MoveWalk); // 歩きアニメ再生
		}


		void DaddyPenguinEnterIglooState::Update()
		{
			if (!m_isArrivedEntrance)
			{
				Vector3 myPos = m_owner->GetTransform().m_position;
				Vector3 dir = m_entrancePos - myPos;
				dir.y = 0.0f; // 高さは無視

				if (dir.Length() < 5.0f)
				{
					// 入り口に到着した！
					m_isArrivedEntrance = true;
					m_owner->SetMoveSpeed(0.0f);
					m_owner->SetMoveDirection(Vector3::Zero);

					// =========================================================
					// ★ ワープ処理とフラグ切り替え
					// =========================================================
					// 現在地を基準に最も近いイグルーの中心座標を取得
					Vector3 insidePos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);

					// ※めり込んだり、空中に浮いたりする場合は insidePos.y を微調整してください
					insidePos.y += 20.0f;

					// ワープ実行！（プロジェクトの仕様に合わせて SetPosition 関数などを呼び出してください）
					// m_owner->GetDaddyPenguin()->SetPosition(insidePos); // ← アクターの座標更新
					m_owner->SetPosition(insidePos);                       // ← ステートマシンの座標更新

					// 「中にいるよ」フラグを立てて、次フレームで InsideIglooState に遷移させる
					m_owner->SetIsInsideIgloo(true);

					// ※子ペンギンを呼び寄せる処理は、次の InsideIglooState の Enter() で行います
				}
				else
				{
					// 入り口に向かって歩く処理
					dir.Normalize();
					m_owner->SetMoveDirection(dir);
					m_owner->SetMoveSpeed(m_owner->GetPenguinStatus()->GetSneakSpeed());
					m_owner->Move();
				}
			}
		}


		void DaddyPenguinEnterIglooState::Exit()
		{}


		DaddyPenguinEnterIglooState::DaddyPenguinEnterIglooState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
			, m_entrancePos(Vector3::Zero)
			, m_isArrivedEntrance(false)
		{}




		/************************************/


		void DaddyPenguinInsideIglooState::Enter()
		{
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);

			// 現在地を基準に最も近いイグルーの座標と回転を取得
			const Vector3 myPos = m_owner->GetTransform().m_position;
			Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
			Quaternion iglooRot = StageSystem::GetInstance()->GetNearestIglooRotation(myPos);
			Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);
			iglooRot.Apply(forwardVec);

			// コントローラーと同じ距離（青い円の中心）
			Vector3 interactPos = iglooPos + (forwardVec * 150.0f);

			// =========================================================
			// ★ ここで「引数1つ」で呼ぶのが大正解です！
			ChildPenguinManager::GetInstance()->StartIglooEvent(interactPos);
			// =========================================================
		}


		void DaddyPenguinInsideIglooState::Update()
		{
			if (g_pad[0]->IsTrigger(enButtonA))
			{
				// 1. 入り口の座標を再計算（入る時と同じ計算）
				// 現在地を基準に最も近いイグルーの座標と回転を取得
				const Vector3 myPos = m_owner->GetTransform().m_position;
				Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
				Quaternion iglooRot = StageSystem::GetInstance()->GetNearestIglooRotation(myPos);
				Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);
				iglooRot.Apply(forwardVec);

				Vector3 exitPos = iglooPos + (forwardVec * 200.0f);
				exitPos.y += 20.0f; // 親ペンギンの高さ

				// 2. 親ペンギン自身を入り口にワープさせる
				m_owner->SetPosition(exitPos);

				// 3. マネージャー経由で、子ペンギン全員に「外に出ろ！」と命令する
				ChildPenguinManager::GetInstance()->EndIglooEvent(exitPos);

				// 4. 中にいるフラグを折る（これで次フレームから通常のIdle状態などに戻ります）
				m_owner->SetIsInsideIgloo(false);
			}
		}


		void DaddyPenguinInsideIglooState::Exit()
		{
			m_owner->SetIsInsideIgloo(false);
		}


		DaddyPenguinInsideIglooState::DaddyPenguinInsideIglooState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}
	}
}
