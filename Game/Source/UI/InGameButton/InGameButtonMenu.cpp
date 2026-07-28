/**
 * @file InGameButtonMenu.cpp
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "InGameButtonGaugeAnimStatus.h"
#include "InGameButtonMenu.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Manager/BattleManager.h"
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 命令標識が回りきるまでの時間(秒) */
			constexpr float SIGN_FLIP_DURATION = 0.4f;
			/** 命令標識が回るときの半回転の回数。奇数にすると必ず反対の面を向いて止まる */
			constexpr int SIGN_FLIP_HALF_TURN_COUNT = 1;
			/** 命令標識の表(GO)のUI名 */
			constexpr const char* SIGN_GO_UI_NAME = "OrderSignGoIcon";
			/** 命令標識の裏(WAIT)のUI名 */
			constexpr const char* SIGN_WAIT_UI_NAME = "OrderSignWaitIcon";
		}


		InGameButtonMenu::InGameButtonMenu()
		{
			// スタミナゲージ専用のステータスを生成。
			m_gaugeStatus = std::make_unique<InGameButtonGaugeStatus>();
			m_gaugeStatus->SetUp();

			// スタミナゲージ専用のアニメーションステータスを生成。
			m_gaugeAnimStatus = std::make_unique<InGameButtonGaugeAnimStatus>();
			m_gaugeAnimStatus->SetUp();
		}


		void InGameButtonMenu::Update()
		{
			if (!m_startingAnimLogic.IsAnimationStarted())
			{
				m_startingAnimLogic.Initialize(
					this,
					{
						"NotInputJumpIcon", "NotInputSneakIcon", "NotInputSlideIcon", "NotInputOrderIcon",
						"InputJumpIcon",    "InputSneakIcon",    "InputSlideIcon",    "InputOrderIcon",
						"NotInputAbuttonIcon", "NotInputBbuttonIcon", "NotInputXbuttonIcon", "NotInputYbuttonIcon",
						"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon",
						SIGN_GO_UI_NAME,       SIGN_WAIT_UI_NAME
					},
					{}, // 数字UIは使用しないため空のリストを渡す
					Vector3(300.0f, 0.0f, 0.0f),
					1.0f, // duration（既存と同じ値を明示）
					{},   // テキストUIは使用しない
					{ "JumpStaminaGaugeA", "JumpStaminaGaugeB", "SlideStaminaGaugeA", "SlideStaminaGaugeB" } // ゲージUIも同じ演出で入場させる
				);
			}
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

			ButtonIconUpdate();
			UpdateStaminaGauge();
			UpdateCommandSign();
			MenuBase::Update();
		}


		void InGameButtonMenu::UpdateStaminaGauge()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			// 追従イージング・色演出・描画をまとめて行うラムダ式
			auto updateGauge = [&](const char* gaugeAName, const char* gaugeBName,
				float rawTargetRatio, bool isLocked, bool& wasLocked,
				float& displayRatio, float followSpeed,
				uint32_t lockAnimKey, uint32_t unlockAnimKey)
				{
					auto* gaugeA = GetUI<UICircleGauge>(Hash32(gaugeAName));
					auto* gaugeB = GetUI<UICircleGauge>(Hash32(gaugeBName));
					if (!gaugeA || !gaugeB) return;

					// ゲームからのスタミナ残量(0.0〜1.0)を安全にクランプ
					float targetRatio = util::clamp(rawTargetRatio, 0.0f, 1.0f);

					displayRatio = targetRatio;

					// （時計回りに削れていき、反時計回りに回復する）
					gaugeA->SetProgressRange(0.0f, displayRatio);

					// 外枠(GaugeB)は常に100%表示
					gaugeB->SetProgress(1.0f);

					// スタミナ残量に応じた色の変更 (緑 → 黄色 → 赤)
					Vector4 gaugeColor;
					if (displayRatio > 0.5f) {
						gaugeColor = Vector4(0.0f, 1.0f, 0.0f, 1.0f); // 緑
					}
					else if (displayRatio > 0.2f) {
						gaugeColor = Vector4(1.0f, 1.0f, 0.0f, 1.0f); // 黄色
					}
					else {
						gaugeColor = Vector4(1.0f, 0.0f, 0.0f, 1.0f); // 赤
					}
					gaugeA->SetBgColor(gaugeColor);

					// ロックされた瞬間（使用可能→クールダウン）にロック演出を再生
					if (isLocked && !wasLocked)
					{
						gaugeA->RemoveAnimation(unlockAnimKey);
						if (UIAnimationFactory::Attach<UIColorAnimation>(gaugeA, lockAnimKey))
						{
							gaugeA->FindAnimation(lockAnimKey);
							gaugeA->PlayAnimation();
						}
					}
					// ロックが解除された瞬間（クールダウン→使用可能）に復帰演出を再生
					else if (!isLocked && wasLocked)
					{
						gaugeA->RemoveAnimation(lockAnimKey);
						if (UIAnimationFactory::Attach<UIColorAnimation>(gaugeA, unlockAnimKey))
						{
							gaugeA->FindAnimation(unlockAnimKey);
							gaugeA->PlayAnimation();
						}
					}

					wasLocked = isLocked;

					if (displayRatio >= 1.0f && targetRatio >= 1.0f) {
						gaugeA->m_isDraw = false;
						gaugeB->m_isDraw = false;
					}
					else {
						gaugeA->m_isDraw = true;
						gaugeB->m_isDraw = false;
					}
				};

			updateGauge("JumpStaminaGaugeA", "JumpStaminaGaugeB",
				m_jumpStaminaRatio, m_isJumpStaminaLocked, m_wasJumpStaminaLocked,
				m_jumpDisplayRatio, m_gaugeStatus->GetJumpFollowSpeed(),
				animKey::JUMP_GAUGE_LOCK_ANIM_KEY, animKey::JUMP_GAUGE_UNLOCK_ANIM_KEY);

			updateGauge("SlideStaminaGaugeA", "SlideStaminaGaugeB",
				m_slideStaminaRatio, m_isSlideStaminaLocked, m_wasSlideStaminaLocked,
				m_slideDisplayRatio, m_gaugeStatus->GetSlideFollowSpeed(),
				animKey::SLIDE_GAUGE_LOCK_ANIM_KEY, animKey::SLIDE_GAUGE_UNLOCK_ANIM_KEY);
		}


		void InGameButtonMenu::UpdateCommandSign()
		{
			auto* goSign = GetUI<UIIcon>(Hash32(SIGN_GO_UI_NAME));
			auto* waitSign = GetUI<UIIcon>(Hash32(SIGN_WAIT_UI_NAME));
			if (!goSign || !waitSign) return;

			// 子ペンギンがいないシーンでは命令自体が存在しないので標識も出さない
			auto* childPenguinManager = actor::ChildPenguinManager::GetInstance();
			if (childPenguinManager == nullptr)
			{
				goSign->m_isDraw = false;
				waitSign->m_isDraw = false;
				return;
			}

			// Yボタンで命令が切り替わった瞬間を捉えて、標識を回し始める
			const bool isWaitCommand = childPenguinManager->GetCommand() == actor::ChildPenguinManager::EnPenguinCommand::Wait;
			if (isWaitCommand != m_wasWaitCommand)
			{
				m_wasWaitCommand = isWaitCommand;
				// 回転中にもう一度切り替わったら今の角度から続きを回す
				// (0に戻すと縮んでいた横幅が一瞬で元に戻ってしまう)
				m_signFlipTimer = m_isSignFlipping ? SIGN_FLIP_DURATION - m_signFlipTimer : 0.0f;
				m_isSignFlipping = true;
			}

			if (m_isSignFlipping)
			{
				m_signFlipTimer += g_gameTime->GetFrameDeltaTime();
				if (m_signFlipTimer >= SIGN_FLIP_DURATION)
				{
					m_signFlipTimer = SIGN_FLIP_DURATION;
					m_isSignFlipping = false;
				}
			}

			// 何回半回転したかを求める。0.5回転ごと(板が真横を向いて見えなくなる瞬間)に
			// 手前の面が入れ替わるので、0.5足した値の偶奇でどちらの面が見えているかを判定する
			const float turnProgress = (m_signFlipTimer / SIGN_FLIP_DURATION) * SIGN_FLIP_HALF_TURN_COUNT;
			const bool isShowingNewFace = (static_cast<int>(turnProgress + 0.5f) % 2) != 0;
			const bool isShowingWait = isShowingNewFace ? isWaitCommand : !isWaitCommand;

			goSign->m_isDraw = !isShowingWait;
			waitSign->m_isDraw = isShowingWait;

			// 横幅を回転角のコサインで縮めて、板が左右にくるっと回っているように見せる
			const float widthRate = fabsf(cosf(Math::PI * turnProgress));

			Vector3 goSignScale = m_goSignBaseScale;
			goSignScale.x *= widthRate;
			goSign->m_transform.m_localTransform.m_scale = goSignScale;

			Vector3 waitSignScale = m_waitSignBaseScale;
			waitSignScale.x *= widthRate;
			waitSign->m_transform.m_localTransform.m_scale = waitSignScale;
		}


		void InGameButtonMenu::ButtonIconUpdate()
		{
			// UI表示を切り替えるラムダ式（ローカル関数）
			auto updateUI = [&](bool isInput, const char* notInputAct, const char* inputAct, const char* notInputBtn, const char* inputBtn)
				{
					if (auto* ui = GetUI<UIIcon>(Hash32(notInputAct))) ui->m_isDraw = !isInput;
					if (auto* ui = GetUI<UIIcon>(Hash32(inputAct)))    ui->m_isDraw = isInput;
					if (auto* ui = GetUI<UIIcon>(Hash32(notInputBtn))) ui->m_isDraw = !isInput;
					if (auto* ui = GetUI<UIIcon>(Hash32(inputBtn)))    ui->m_isDraw = isInput;
				};

			// スニークが使用可能かどうか（シロクマとの距離）
			const bool isSneakAvailable = BattleManager::GetInstance().IsSneakAvailable();

			// 各ボタンに対して一括処理
			updateUI(IsInputAButton(), "NotInputJumpIcon", "InputJumpIcon", "NotInputAbuttonIcon", "InputAbuttonIcon");
			// isSneakAvailable が false なら、Bボタンを押していても isInput は常に false 
			updateUI(IsInputBButton() && isSneakAvailable, "NotInputSneakIcon", "InputSneakIcon", "NotInputBbuttonIcon", "InputBbuttonIcon");
			updateUI(IsInputXButton(), "NotInputSlideIcon", "InputSlideIcon", "NotInputXbuttonIcon", "InputXbuttonIcon");
			updateUI(IsInputYButton(), "NotInputOrderIcon", "InputOrderIcon", "NotInputYbuttonIcon", "InputYbuttonIcon");

			UpdateSneakIconColor();
		}


		void InGameButtonMenu::InitializeLogic()
		{
			// 初期化対象のアイコン名リスト
			const char* iconNames[] = {
				"NotInputJumpIcon", "NotInputSneakIcon", "NotInputSlideIcon", "NotInputOrderIcon",
				"InputJumpIcon",    "InputSneakIcon",    "InputSlideIcon",    "InputOrderIcon",
				"NotInputAbuttonIcon", "NotInputBbuttonIcon", "NotInputXbuttonIcon", "NotInputYbuttonIcon",
				"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon"
			};

			for (const char* name : iconNames)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_isDraw = false;
				}
			}

			// スタミナサークルゲージも初期状態では非表示にする
			const char* gaugeNames[] = {
				"JumpStaminaGaugeA", "JumpStaminaGaugeB",
				"SlideStaminaGaugeA", "SlideStaminaGaugeB"
			};

			for (const char* name : gaugeNames)
			{
				if (auto* ui = GetUI<UICircleGauge>(Hash32(name)))
				{
					ui->m_isDraw = false;
				}
			}

			// 命令標識は回転演出の基準になるスケールをJsonから拾っておく
			if (auto* goSign = GetUI<UIIcon>(Hash32(SIGN_GO_UI_NAME)))
			{
				m_goSignBaseScale = goSign->m_transform.m_localTransform.m_scale;
				goSign->m_isDraw = false;
			}
			if (auto* waitSign = GetUI<UIIcon>(Hash32(SIGN_WAIT_UI_NAME)))
			{
				m_waitSignBaseScale = waitSign->m_transform.m_localTransform.m_scale;
				waitSign->m_isDraw = false;
			}

			// 回転しきった状態から始めて、今の命令の面をそのまま見せる
			m_signFlipTimer = SIGN_FLIP_DURATION;
			m_isSignFlipping = false;

			auto* childPenguinManager = actor::ChildPenguinManager::GetInstance();
			m_wasWaitCommand = childPenguinManager != nullptr
				&& childPenguinManager->GetCommand() == actor::ChildPenguinManager::EnPenguinCommand::Wait;
		}


		void InGameButtonMenu::UpdateSneakIconColor()
		{
			const Vector4 normalColor(1.0f, 1.0f, 1.0f, 1.0f);   // 通常（白）
			const Vector4 grayColor(0.4f, 0.4f, 0.4f, 1.0f);     // グレーアウト
			const Vector4& color = BattleManager::GetInstance().IsSneakAvailable() ? normalColor : grayColor;

			const char* sneakIconNames[] = {
				"NotInputSneakIcon",   "InputSneakIcon",
				"NotInputBbuttonIcon", "InputBbuttonIcon"
			};
			for (const char* name : sneakIconNames)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_color = color;
				}
			}
		}


		bool InGameButtonMenu::IsInputAButton() const
		{
			return g_pad[0]->IsPress(enButtonA);
		}


		bool InGameButtonMenu::IsInputBButton() const
		{
			return g_pad[0]->IsPress(enButtonB);
		}


		bool InGameButtonMenu::IsInputXButton() const
		{
			return g_pad[0]->IsPress(enButtonX);
		}


		bool InGameButtonMenu::IsInputYButton() const
		{
			return g_pad[0]->IsPress(enButtonY);
		}
	}
}