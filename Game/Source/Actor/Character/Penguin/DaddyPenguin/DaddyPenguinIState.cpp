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
#include "Source/Effect/EffectManager.h"
#include "Source/Manager/IglooManager.h"
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
			// Yボタンの役割を「待機・追従の切り替え」から
			// 「パニックで散った子ペンギンの再集合を呼びかける」へ変更した。
			//
			// 逃走をシロクマ最優先にしたことで、群れがまるごと散る場面が生まれる。
			// その散開に対して、プレイヤーが群れを呼び戻せる手段がこれ。
			// クールダウン中はそもそもこのステートへ入らない
			// （DaddyPenguinController が CanCallRegroup() で弾いている）。
			//
			// 待機命令そのもの（EnPenguinCommand::Wait）は消していない。
			// ToggleCommand() / SetCommand() は残っているので、
			// 別のボタンへ割り当て直せば元の切り替えも復活できる。
			auto* childPenMan = ChildPenguinManager::GetInstance();
			childPenMan->CallRegroup();

			auto* effect = &EffectManager::Get();
			auto* sound = &SoundManager::Get();

			// === 「おいで！」の演出 ===
			m_owner->PlayAnimation(EnPenguinAnimationID::CommandShout);
			effect->PlayEffect(EnEffectKind::DaddyPenguinCommand, m_owner->GetTransform().m_position, Quaternion::Identity, EFFECT_SCALE);
			sound->PlaySE(enSoundKind::enSoundKind_DaddyPenguinShoutFollow, false);
			sound->PlaySE(enSoundKind::enSoundKind_DaddyPenguinSystemFollow, false);
		}


		void DaddyPenguinCommandShoutState::Update()
		{}


		void DaddyPenguinCommandShoutState::Exit()
		{}


		DaddyPenguinCommandShoutState::DaddyPenguinCommandShoutState(DaddyPenguinStateMachine* owner)
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
			//m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);

			//// 現在地を基準に最も近いイグルーの座標と回転を取得
			//const Vector3 myPos = m_owner->GetTransform().m_position;
			//Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
			//Quaternion iglooRot = StageSystem::GetInstance()->GetNearestIglooRotation(myPos);
			//Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);
			//iglooRot.Apply(forwardVec);

			//// コントローラーと同じ距離（青い円の中心）
			//Vector3 interactPos = iglooPos + (forwardVec * 150.0f);

			//// =========================================================
			//// ★ ここで「引数1つ」で呼ぶのが大正解です！
			//ChildPenguinManager::GetInstance()->StartIglooEvent(interactPos);
			//// =========================================================



			// 1. 中での待機アニメーションを再生
			m_owner->PlayAnimation(EnPenguinAnimationID::IdleStanding);

			// 2. シロクマに壊された時に一緒に弾き出されるよう、IglooManagerに登録
			// ※ m_owner から DaddyPenguin のポインタを取得して渡します
			// （もし以下の行でコンパイルエラーが出る場合は、環境に合わせて親ペンギンのポインタ取得関数に書き換えてください）
			DaddyPenguin* daddy = m_owner->GetOwnerDaddyPenguin();
			IglooManager::GetInstance().SetInsideDaddy(daddy);

			// ★ 修正：親が中に入った後に、子ペンギンたちを呼び寄せる（元の安全な状態）
			// 入り口の座標を再計算して渡します
			const Vector3 myPos = m_owner->GetTransform().m_position;
			Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
			Quaternion iglooRot = StageSystem::GetInstance()->GetNearestIglooRotation(myPos);
			Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);
			iglooRot.Apply(forwardVec);
			Vector3 entrancePos = iglooPos + (forwardVec * 150.0f);

			// 子供たちに合図を出す
			ChildPenguinManager::GetInstance()->StartIglooEvent(entrancePos);
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

			IglooManager::GetInstance().ClearPenguins();
		}


		DaddyPenguinInsideIglooState::DaddyPenguinInsideIglooState(DaddyPenguinStateMachine* owner)
			: DaddyPenguinIState(owner)
		{}
	}
}
