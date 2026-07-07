/**
 * @file InGameButtonMenu.cpp
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "InGameButtonGaugeAnimStatus.h"
#include "InGameButtonMenu.h"
#include "Source/Manager/BattleManager.h"
#include "Source/UI/Animation/UIAnimationFactory.h"  
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
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
						"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon"
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