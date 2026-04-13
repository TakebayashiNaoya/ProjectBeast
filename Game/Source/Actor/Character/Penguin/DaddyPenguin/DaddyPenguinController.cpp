/**
 * @file DaddyPenguinController.cpp
 * @brief 親ペンギンのプレイヤーコントローラー
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinController.h"
#include "DaddyPenguinStateMachine.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraManager.h"
#include "Source/UI/IglooPromptMenu.h"
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

		void DaddyPenguinController::Update()
		{
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
				moveDirection = inputDir;

				/** スティックの倒し具合とBボタンの状態でスニークかダッシュかを決める */
				const float SNEAK_THRESHOLD = 0.9f;
				if (stickLength <= SNEAK_THRESHOLD || isPressB) {
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
			Vector3 iglooPos = StageSystem::GetInstance()->GetObjectPosition("igloo");
			Quaternion iglooRot = StageSystem::GetInstance()->GetObjectRotation("igloo");

			Vector3 forwardVec = Vector3(-1.0f, 0.0f, 0.0f);
			iglooRot.Apply(forwardVec);

			float interactAreaOffset = 150.0f;
			Vector3 interactPos = iglooPos + (forwardVec * interactAreaOffset);

			Vector3 myPos = m_owner->GetTransform().m_position;

			Vector3 diff = myPos - interactPos;
			diff.y = 0.0f;

			float interactRadius = 100.0f;
			bool isNearIgloo = (diff.Length() <= interactRadius);

			// Aボタンプロンプトの表示・非表示を更新
			if (m_iglooPromptMenu)
			{
				m_iglooPromptMenu->SetTargetPosition(interactPos);
				m_iglooPromptMenu->SetDraw(isNearIgloo);
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
			// 共通のアクション入力
			m_stateMachine->SetActionInput(moveDirection, isSneak, isDash, isJump, isSlide);

			// 親ペンギン固有の入力
			m_stateMachine->SetIsCommandToggle(isCommandToggle);

			m_stateMachine->SetIsEnterIgloo(isEnterIgloo);
		}
	}
}