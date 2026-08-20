/**
 * @file DaddyPenguinController.cpp
 * @brief 親ペンギンのプレイヤーコントローラー
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinController.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Manager/BattleManager.h"
#include "Source/UI/Menus/IglooPromptMenu.h"
#include <algorithm> // std::min用


namespace app
{
	namespace actor
	{
		DaddyPenguinController::DaddyPenguinController(DaddyPenguin* owner)
			: m_owner(owner)
			, m_stateMachine(owner->GetStateMachine())
			, m_iglooPromptMenu(nullptr)
		{}

		void DaddyPenguinController::UpdateClingySlow()
		{
			// ChildPenguinManagerから隊列の中に甘えん坊が何匹いるかを取得する。
			m_clingyCount = ChildPenguinManager::GetInstance()->GetClingyCount();

			// 甘えん坊がいなければ等倍のまま。
			if (m_clingyCount <= 0)
			{
				m_speedMultiplier = 1.0f;
				return;
			}

			const int MAX_SLOW_PERCENT = 20;	// 最大減速率（%）
			const int MAX_PERCENT = 100;		// 最大値（%）

			// float計算だとずれが出るので、int計算で減速率を決める。
			int slowPercent = min(m_clingyCount * 1, MAX_SLOW_PERCENT);
			// 現在の減速率を計算。
			int currentPercent = MAX_PERCENT - slowPercent;

			m_speedMultiplier = currentPercent / static_cast<float>(MAX_PERCENT);
		}


		void DaddyPenguinController::Update()
		{
			// 減速率は移動入力の有無にかかわらず毎フレーム更新する
			// （ログのtickが入力していないフレームでも正しい値を拾えるようにするため）
			UpdateClingySlow();

			// =========================================================
			// 移動入力の更新
			// =========================================================

			/**
			 * コントローラーの倒し具合で挙動を変えるため、
			 * 正規化前のスティック入力の生データを取ってくる
			 * NOTE: sThumbLXとsThumbLYは-32768～32767の範囲で値が入るため、
			 *		 32767で割って-1.0f～1.0fの範囲に正規化する
			 */
			const XINPUT_STATE& state = g_pad[0]->GetXInputState();
			float rawX = static_cast<float>(state.Gamepad.sThumbLX) / 32767.0f;
			float rawY = static_cast<float>(state.Gamepad.sThumbLY) / 32767.0f;
			float rawLength = sqrtf(rawX * rawX + rawY * rawY);

			/**
			 * コントローラーorエンジンの入力を取る
			 */
			float inputX = 0.0f;			/** X方向の入力 */
			float inputY = 0.0f;			/** Y方向の入力 */
			float stickLength = 0.0f;		/** スティックの倒し具合（0.0f～1.0f） */
			const float DEAD_ZONE = 0.1f;	/** コントローラーの遊び */

			/** ① コントローラーが倒されている場合は生データを採用 */
			if (rawLength > DEAD_ZONE)
			{
				inputX = rawX;
				inputY = rawY;
				stickLength = min(rawLength, 1.0f);
			}
			/** ② コントローラーが触られていない場合はエンジンのデータを採用 */
			else
			{
				inputX = g_pad[0]->GetLStickXF();
				inputY = g_pad[0]->GetLStickYF();
				stickLength = sqrtf(inputX * inputX + inputY * inputY);
				stickLength = min(stickLength, 1.0f);
			}

			/** Bボタンが押されているか（スニークのトグル用） */
			bool isPressB = g_pad[0]->IsPress(enButtonB);

			// シロクマに近づいているときだけ、Bボタンによる強制忍び足を有効にする
			// （遠いときにBを押しても、歩き自体は別途スティック量で判定されるので問題ない）
			bool isForceSneakByButton = isPressB && BattleManager::GetInstance().IsSneakAvailable();

			Vector3 moveDirection = Vector3::Zero;
			bool isSneak = false;
			bool isDash = false;

			/** スティックがある程度倒されている場合は移動入力として採用 */
			if (stickLength > 0.1f)
			{
				/** カメラの向きと入力から移動方向を決める */
				const auto& camData = camera::CameraManager::Get().GetCurrentCameraData();
				Vector3 camForward = camData.target - camData.position;
				camForward.y = 0.0f;
				camForward.Normalize();
				Vector3 camRight = { camForward.z, 0.0f, -camForward.x };
				Vector3 inputDir = camRight * inputX + camForward * inputY;
				if (inputDir.LengthSq() > FLT_EPSILON) {
					inputDir.Normalize();
				}


				//===========================================================//
				// 甘えん坊の数に応じて親ペンギンの移動速度を調整するロジック//
				//===========================================================//
				// 減速率は UpdateClingySlow() で算出済みのものを使う。
				// 最終的なスティックの入力値。
				float finalStickLength = util::clamp<float>(stickLength * m_speedMultiplier, 0.0f, 1.0f);

				moveDirection = inputDir * finalStickLength;

				/** Bボタンの状態でスニークかダッシュかを決める */
				const float SNEAK_THRESHOLD = 0.9f;
				if (stickLength <= SNEAK_THRESHOLD || isForceSneakByButton) {
					isSneak = true;
					isDash = false;
				}
				else {
					isSneak = false;
					isDash = true;
				}
			}
			else
			{
				moveDirection.Set(Vector3::Zero);
				isSneak = false;
				isDash = false;
			}

			// =========================================================
			// アクション入力の更新
			// =========================================================
			bool isJump = g_pad[0]->IsTrigger(enButtonA);
			bool isCommandToggle = g_pad[0]->IsTrigger(enButtonY);
			bool isSlide = g_pad[0]->IsPress(enButtonX);
			bool isEnterIgloo = false;

			// かまくら近接判定（毎フレーム実行）
			// 親ペンギンの現在位置を基準に最も近いイグルーを取得する
			Vector3 myPos = m_owner->GetTransform().m_position;
			Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
			Quaternion iglooRot = StageSystem::GetInstance()->GetNearestIglooRotation(myPos);

			Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);
			iglooRot.Apply(forwardVec);

			float interactAreaOffset = 220.0f;
			Vector3 interactPos = iglooPos + (forwardVec * interactAreaOffset);

			Vector3 diff = myPos - interactPos;
			diff.y = 0.0f;

			float interactRadius = 80.0f;
			bool isNearIgloo = (diff.Length() <= interactRadius);

			if (m_iglooPromptMenu)
			{
				// 1. ステートマシンに「今かまくらの中にいるステートか？」を確認
				bool isInside = m_stateMachine->IsEqualCurrentState(DaddyPenguinInsideIglooState::ID());

				if (isInside)
				{
					// 【かまくらの中にいるとき】 -> 「出る」を表示
					// 位置は親ペンギン自身の足元（頭上にUIが出るように計算される）
					m_iglooPromptMenu->SetPromptType(ui::IglooPromptMenu::PromptType::Exit);
					m_iglooPromptMenu->SetTargetPosition(m_owner->GetTransform().m_position);
				}
				else if (isNearIgloo)
				{
					// 【外にいて、入り口の近くにいるとき】 -> 「入る」を表示
					// 位置はかまくらの入り口（interactPos）
					m_iglooPromptMenu->SetPromptType(ui::IglooPromptMenu::PromptType::Enter);
					m_iglooPromptMenu->SetTargetPosition(interactPos);
				}
				else
				{
					// 【どちらでもないとき】 -> 非表示
					m_iglooPromptMenu->SetPromptType(ui::IglooPromptMenu::PromptType::None);
				}
			}

			// Aボタンが押されていて、かつかまくら近くにいるならイベント開始
			if (isJump && isNearIgloo)
			{
				isEnterIgloo = true;
				isJump = false;
			}

			// =========================================================
			// ステートマシンへ一括入力
			// =========================================================

			// 陣形のパッシブ/ウルト速度倍率を親ペンギン自身にも反映する（1.0f超もありうるためクランプなしの別枠に渡す）
			m_stateMachine->SetExternalSpeedMultiplier(ChildPenguinManager::GetInstance()->GetFormationSpeedMultiplier());

			// SlideEnd アニメーション中はプレイヤー入力を無視し、
			// 自然に滑り止まるのを待つ
			if (m_stateMachine->IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
			}
			else
			{
				// 共通のアクション入力
				m_stateMachine->SetActionInput(moveDirection, isSneak, isDash, isJump, isSlide);
			}

			// 親ペンギン固有の入力
			m_stateMachine->SetIsCommandToggle(isCommandToggle);

			m_stateMachine->SetIsEnterIgloo(isEnterIgloo);
		}
	}
}