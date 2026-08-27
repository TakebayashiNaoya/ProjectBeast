/**
 * @file InGameButtonMenu.cpp
 * @brief インゲーム中にボタンメニューを表示するクラス
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
			/** Yボタン強調（クマ襲撃中）の脈動の振幅と速さ */
			constexpr float Y_EMPHASIS_PULSE_AMPLITUDE = 0.18f;
			constexpr float Y_EMPHASIS_PULSE_SPEED = 6.0f;
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
						"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon"
					},
					{}, // 数字UIは使用しないため空のリストを渡す
					Vector3(300.0f, 0.0f, 0.0f),
					1.0f, // duration（既存と同じ値を明示）
					{},   // テキストUIは使用しない
					{ "JumpStaminaGaugeA", "JumpStaminaGaugeB", "OrderCooldownGaugeA", "OrderCooldownGaugeB" } // ゲージUIも同じ演出で入場させる
				);
			}
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

			ButtonIconUpdate();
			UpdateStaminaGauge();
			UpdateYButtonEmphasis();
			MenuBase::Update();
		}


		void InGameButtonMenu::UpdateYButtonEmphasis()
		{
			/** クマが自分の隊列の子を追っている ＆ 再集合が使えるときだけ、Yボタン周りを強調する。
			 *  「散った群れはYで呼び戻せる」を、必要な瞬間にその場で教えるための演出。
			 *  マップの遠くではぐれた子が襲われても反応しない（自分の群れの危機だけに絞る）。
			 *  常設のプロンプトはうるさいため、状況限定＋既存アイコンの強調に留める */
			auto* cpm = actor::ChildPenguinManager::GetInstance();
			const bool emphasize = cpm != nullptr
				&& cpm->HasBearThreatOnFormation()
				&& cpm->CanCallRegroup();

			m_yEmphasisTimer += g_gameTime->GetFrameDeltaTime();

			const float pulse = emphasize
				? 1.0f + Y_EMPHASIS_PULSE_AMPLITUDE
					* fabsf(sinf(m_yEmphasisTimer * Y_EMPHASIS_PULSE_SPEED))
				: 1.0f;
			const Vector4 color = emphasize
				? Vector4(1.0f, 1.0f, 0.35f, 1.0f)	// 注意を引く黄色
				: Vector4(1.0f, 1.0f, 1.0f, 1.0f);

			const char* yIconNames[] = {
				"NotInputOrderIcon",   "InputOrderIcon",
				"NotInputYbuttonIcon", "InputYbuttonIcon"
			};
			for (int i = 0; i < 4; ++i)
			{
				auto* ui = GetUI<UIIcon>(Hash32(yIconNames[i]));
				if (ui == nullptr) continue;

				// JSONの基準スケールを初回に保存し、脈動は「基準×パルス」で掛ける。
				// (pulse, pulse, 1) の直接上書きだと、基準0.8のメガホンアイコンが
				// 非強調時も1.0に固定され、他のボタンより常に大きく見えてしまう
				if (!m_isYEmphasisBaseCaptured[i])
				{
					m_yEmphasisBaseScales[i] = ui->m_transform.m_localTransform.m_scale;
					m_isYEmphasisBaseCaptured[i] = true;
				}
				const Vector3& base = m_yEmphasisBaseScales[i];
				ui->m_transform.m_localTransform.m_scale =
					Vector3(base.x * pulse, base.y * pulse, base.z);
				ui->m_color = color;
			}
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

			// Y（再集合の呼びかけ）のクールダウン。ロック演出はスライド時代の
			// アニメーション定義（slideGauge*Anim）をそのまま流用している
			updateGauge("OrderCooldownGaugeA", "OrderCooldownGaugeB",
				m_regroupCooldownRatio, m_isRegroupCooldownLocked, m_wasRegroupCooldownLocked,
				m_regroupDisplayRatio, m_gaugeStatus->GetSlideFollowSpeed(),
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
			// Reload後にJSONの基準スケールを取り直す
			m_isYEmphasisBaseCaptured.fill(false);

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

			// サークルゲージも初期状態では非表示にする
			const char* gaugeNames[] = {
				"JumpStaminaGaugeA", "JumpStaminaGaugeB",
				"OrderCooldownGaugeA", "OrderCooldownGaugeB"
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