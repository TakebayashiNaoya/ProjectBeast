/**
 * @file DaddyPenguinController.cpp
 * @brief 親ペンギンのプレイヤーコントローラー
 */
#include "stdafx.h"
#include "DaddyPenguin.h"
#include "DaddyPenguinController.h"
#include "DaddyPenguinIState.h"
#include "DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Manager/BattleManager.h"
#include "Source/UI/Menus/IglooPromptMenu.h"
#include "Source/Util/RandomDevice.h"
#include <algorithm> // std::min用


namespace app
{
	namespace actor
	{
		namespace
		{
			/**
			 * @brief 自動プレイのボット操作が有効かどうか
			 * @details 環境変数 BEAST_AUTOPLAY が "0" 以外の値で設定されているときに有効。
			 *          デバイスロスト調査で入れた自動周回（TitleScene参照）と同じスイッチ。
			 */
			bool IsAutoplayBotEnabled()
			{
				char buf[8];
				size_t len = 0;
				return getenv_s(&len, buf, sizeof(buf), "BEAST_AUTOPLAY") == 0
					&& len > 0 && buf[0] != '0';
			}

			/** ボットが目標を選び直す間隔（秒） */
			constexpr float BOT_REPICK_INTERVAL = 4.0f;
			/** ボットが目標に到達したとみなす距離 */
			constexpr float BOT_ARRIVE_DISTANCE = 150.0f;
			/** ボットがスライド移動に切り替える目標までの距離 */
			constexpr float BOT_SLIDE_DISTANCE = 700.0f;
			/** ボットが再集合を呼ぶ間隔（秒） */
			constexpr float BOT_REGROUP_INTERVAL = 10.0f;
			/** 子が誰も残っていないときにボットがうろつく半径 */
			constexpr float BOT_WANDER_RADIUS = 2000.0f;
			/** ボットが渦潮を迂回し始める距離と反発の強さ */
			constexpr float BOT_WHIRLPOOL_AVOID_RADIUS = 350.0f;
			constexpr float BOT_WHIRLPOOL_AVOID_WEIGHT = 2.0f;
		}


		DaddyPenguinController::DaddyPenguinController(DaddyPenguin* owner)
			: m_owner(owner)
			, m_stateMachine(owner->GetStateMachine())
			, m_iglooPromptMenu(nullptr)
		{}


		void DaddyPenguinController::UpdateAutoplayBot(
			float& inputX, float& inputY, float& stickLength,
			bool& outSlide, bool& outRegroupCall)
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const Vector3 myPos = m_owner->GetTransform().m_position;
			auto* manager = ChildPenguinManager::GetInstance();

			// 目標の選び直し：一定間隔、または到達したら
			m_botRepickTimer -= deltaTime;
			Vector3 toTarget = m_botTargetPos - myPos;
			toTarget.y = 0.0f;
			if (m_botRepickTimer <= 0.0f
				|| toTarget.LengthSq() < BOT_ARRIVE_DISTANCE * BOT_ARRIVE_DISTANCE)
			{
				m_botRepickTimer = BOT_REPICK_INTERVAL;

				// 隊列に入っていない一番近い子ペンギンへ向かう
				float nearestDistSq = FLT_MAX;
				bool found = false;
				if (manager != nullptr)
				{
					for (auto* cp : manager->GetChildPenguin())
					{
						if (cp == nullptr || manager->IsFollower(cp)) continue;

						/** 渦潮に捕まって旋回中の子は追わない。高速で回る目標を
						 *  追いかけると渦の中でくるくる空回りしてしまう */
						if (cp->GetStateMachine()->GetIsInWhirlpool()) continue;

						Vector3 diff = cp->GetTransform().m_position - myPos;
						diff.y = 0.0f;
						const float distSq = diff.LengthSq();
						if (distSq < nearestDistSq)
						{
							nearestDistSq = distSq;
							m_botTargetPos = cp->GetTransform().m_position;
							found = true;
						}
					}
				}

				// 全員隊列にいる（または子がいない）ならランダムにうろつく
				if (!found)
				{
					const float angle = util::RandomDevice::Random(0.0f, 2.0f * Math::PI);
					const float radius = util::RandomDevice::Random(BOT_WANDER_RADIUS * 0.3f, BOT_WANDER_RADIUS);
					m_botTargetPos = Vector3(cosf(angle) * radius, myPos.y, sinf(angle) * radius);
				}

				toTarget = m_botTargetPos - myPos;
				toTarget.y = 0.0f;
			}

			// 目標へのワールド方向をカメラ相対のスティック入力へ変換する
			const float distToTargetSq = toTarget.LengthSq();
			if (distToTargetSq > FLT_EPSILON)
			{
				toTarget.Normalize();

				// アクティブな渦潮を迂回する。近いほど強い反発を進行方向へ加算する
				if (auto* wpManager = nature::WhirlpoolManager::GetInstance())
				{
					Vector3 avoid = Vector3::Zero;
					wpManager->ForEach([&](nature::Whirlpool* wp)
						{
							Vector3 away = myPos - wp->GetTransform().m_position;
							away.y = 0.0f;
							const float dist = away.Length();
							if (dist < 1.0f || dist >= BOT_WHIRLPOOL_AVOID_RADIUS) return;
							avoid += away * (1.0f / dist) * (1.0f - dist / BOT_WHIRLPOOL_AVOID_RADIUS);
						});
					if (avoid.LengthSq() > FLT_EPSILON)
					{
						toTarget += avoid * BOT_WHIRLPOOL_AVOID_WEIGHT;
						toTarget.y = 0.0f;
						if (toTarget.LengthSq() > FLT_EPSILON) toTarget.Normalize();
					}
				}

				const auto& camData = camera::CameraManager::Get().GetCurrentCameraData();
				Vector3 camForward = camData.target - camData.position;
				camForward.y = 0.0f;
				if (camForward.LengthSq() > FLT_EPSILON)
				{
					camForward.Normalize();
					const Vector3 camRight = { camForward.z, 0.0f, -camForward.x };
					inputX = toTarget.Dot(camRight);
					inputY = toTarget.Dot(camForward);
					stickLength = 1.0f;
				}
			}

			// スタック検出：移動する意思があるのに動けていなければ、目標を捨てて
			// しばらくスライドを禁止する。上り坂でスライドを押しっぱなしにすると
			// ずり落ちで無限に足踏みする（人間ならXを離す場面）ため
			Vector3 movedFromCheck = myPos - m_botStuckCheckPos;
			movedFromCheck.y = 0.0f;
			if (movedFromCheck.LengthSq() > 30.0f * 30.0f)
			{
				m_botStuckTimer = 0.0f;
				m_botStuckCheckPos = myPos;
			}
			else
			{
				m_botStuckTimer += deltaTime;
				if (m_botStuckTimer >= 3.0f)
				{
					m_botStuckTimer = 0.0f;
					m_botStuckCheckPos = myPos;
					m_botRepickTimer = 0.0f;		// 次フレームで目標を選び直す
					m_botNoSlideTimer = 5.0f;		// 走りで迂回する
				}
			}
			if (m_botNoSlideTimer > 0.0f)
			{
				m_botNoSlideTimer -= deltaTime;
			}

			// 遠距離はスライドで移動する（傾斜モデルの経路を実プレイ同様に通す）
			outSlide = (m_botNoSlideTimer <= 0.0f)
				&& distToTargetSq > BOT_SLIDE_DISTANCE * BOT_SLIDE_DISTANCE;

			// 再集合はクールダウンが明けるたびに呼ぶ
			m_botRegroupTimer -= deltaTime;
			outRegroupCall = false;
			if (m_botRegroupTimer <= 0.0f && manager != nullptr && manager->CanCallRegroup())
			{
				outRegroupCall = true;
				m_botRegroupTimer = BOT_REGROUP_INTERVAL;
			}
		}

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

			// 自動プレイ（デバッグ）：ボットの入力でスティックを上書きする
			bool botSlide = false;
			bool botRegroupCall = false;
			if (IsAutoplayBotEnabled())
			{
				UpdateAutoplayBot(inputX, inputY, stickLength, botSlide, botRegroupCall);
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
			// Yボタン＝散ってしまった子ペンギンの再集合を呼びかける。
			// 逃走をシロクマ最優先にした代わりに置いた手段なので、
			// クールダウン中は空振りさせず、鳴き声もモーションも出さない
			auto* childPenguinManager = ChildPenguinManager::GetInstance();
			bool isRegroupCall = (g_pad[0]->IsTrigger(enButtonY) || botRegroupCall)
				&& childPenguinManager != nullptr
				&& childPenguinManager->CanCallRegroup();
			bool isSlide = g_pad[0]->IsPress(enButtonX) || botSlide;
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
			m_stateMachine->SetIsCommandToggle(isRegroupCall);

			m_stateMachine->SetIsEnterIgloo(isEnterIgloo);
		}
	}
}