/**
 * @file TutorialWindowMenu.cpp
 * @brief チュートリアルポップアップウィンドウ（ひな型）
 */
#include "stdafx.h"
#include "Source/Util/CRC32.h"
#include "TutorialWindowMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** JSON の "name" フィールドと必ず一致させること */
			constexpr uint32_t KEY_BG = Hash32("TutorialWindowBg");
			constexpr uint32_t KEY_VIDEO = Hash32("TutorialWindowVideo");
			constexpr uint32_t KEY_TITLE = Hash32("TutorialWindowTitle");
			constexpr uint32_t KEY_DESC = Hash32("TutorialWindowDesc");
			constexpr uint32_t KEY_CLOSE_PROMPT = Hash32("TutorialWindowClosePrompt");
			constexpr uint32_t KEY_CLOSE_TEXT = Hash32("TutorialWindowCloseText");
		}


		TutorialWindowMenu::TutorialWindowMenu()
			: m_state(State::Closed)
			, m_closedByUser(false)
		{}


		void TutorialWindowMenu::InitializeLogic()
		{
			auto* bg = GetUI<UIIcon>(KEY_BG);
			if (!bg)
			{
				SetAllVisible(false);
				return;
			}

			// 開くアニメーション: スケール (0,0,1) → (1,1,1)
			auto openAnim = std::make_unique<UIScaleAnimation>();
			openAnim->SetParameter(
				Vector3(0.0f, 0.0f, 1.0f),
				Vector3(1.0f, 1.0f, 1.0f),
				OPEN_DURATION,
				util::EasingType::EaseOut,
				util::LoopMode::Once
			);
			bg->AddAnimation(ANIM_OPEN, std::move(openAnim));

			// 閉じるアニメーション: スケール (1,1,1) → (0,0,1)
			auto closeAnim = std::make_unique<UIScaleAnimation>();
			closeAnim->SetParameter(
				Vector3(1.0f, 1.0f, 1.0f),
				Vector3(0.0f, 0.0f, 1.0f),
				CLOSE_DURATION,
				util::EasingType::EaseIn,
				util::LoopMode::Once
			);
			bg->AddAnimation(ANIM_CLOSE, std::move(closeAnim));

			// ホットリロード後も現在の状態に合わせてビジュアルを復元する
			switch (m_state)
			{
			case State::Opened:
				bg->SetIsDraw(true);
				bg->m_transform.m_localTransform.m_scale = Vector3::One;
				SetContentVisible(true);
				if (auto* video = GetUI<UIVideo>(KEY_VIDEO))
				{
					video->Stop();
					video->Play();
					video->SetLoop(true);
				}
				break;

			case State::Opening:
				bg->SetIsDraw(true);
				bg->m_transform.m_localTransform.m_scale = Vector3(0.0f, 0.0f, 1.0f);
				if (auto* anim = bg->FindAnimation(ANIM_OPEN))
					anim->PlayAnimation();
				SetContentVisible(false);
				break;

			case State::Closing:
				bg->SetIsDraw(true);
				bg->m_transform.m_localTransform.m_scale = Vector3::One;
				if (auto* anim = bg->FindAnimation(ANIM_CLOSE))
					anim->PlayAnimation();
				SetContentVisible(false);
				break;

			case State::Closed:
			default:
				SetAllVisible(false);
				break;
			}
		}


		void TutorialWindowMenu::Update()
		{
			m_closedByUser = false;

			// UIScaleAnimation を含む UI 全体を更新
			Base::Update();

			// アニメーション完了チェック（Base::Update() の後で行う）
			CheckAnimationComplete();
		}


		void TutorialWindowMenu::Open()
		{
			m_state = State::Opening;

			// BG のみ表示してスケールをゼロにリセット
			if (auto* bg = GetUI<UIIcon>(KEY_BG))
			{
				bg->SetIsDraw(true);
				bg->m_transform.m_localTransform.m_scale = Vector3(0.0f, 0.0f, 1.0f);

				if (auto* anim = bg->FindAnimation(ANIM_OPEN))
					anim->PlayAnimation();
			}

			SetContentVisible(false);
		}


		void TutorialWindowMenu::Close()
		{
			m_state = State::Closed;
			SetAllVisible(false);

			if (auto* video = GetUI<UIVideo>(KEY_VIDEO))
				video->Stop();
		}


		void TutorialWindowMenu::RequestClose()
		{
			// Opened のときだけ受け付ける（Opening/Closing 中の二重実行・多重呼び出しを防止）
			if (m_state != State::Opened) return;
			StartClosing();
		}


		void TutorialWindowMenu::StartClosing()
		{
			// コンテンツを即非表示にして閉じるアニメーション開始
			SetContentVisible(false);

			if (auto* video = GetUI<UIVideo>(KEY_VIDEO))
				video->Stop();

			if (auto* bg = GetUI<UIIcon>(KEY_BG))
			{
				if (auto* anim = bg->FindAnimation(ANIM_CLOSE))
					anim->PlayAnimation();
			}

			m_state = State::Closing;
		}


		void TutorialWindowMenu::CheckAnimationComplete()
		{
			auto* bg = GetUI<UIIcon>(KEY_BG);
			if (!bg) return;

			if (m_state == State::Opening)
			{
				// 開くアニメーションが終わったらコンテンツを表示して動画を再生
				// アニメーションが見つからない場合は即座に Opened へ遷移する
				auto* anim = bg->FindAnimation(ANIM_OPEN);
				if (!anim || !anim->IsPlayAnimation())
				{
					m_state = State::Opened;
					SetContentVisible(true);

					if (auto* video = GetUI<UIVideo>(KEY_VIDEO))
					{
						video->Stop();
						video->Play();
						video->SetLoop(true);
					}
				}
			}
			else if (m_state == State::Closing)
			{
				// 閉じるアニメーションが終わったら全非表示にして完了通知
				// アニメーションが見つからない場合は即座に Closed へ遷移する
				auto* anim = bg->FindAnimation(ANIM_CLOSE);
				if (!anim || !anim->IsPlayAnimation())
				{
					m_state = State::Closed;
					m_closedByUser = true;
					SetAllVisible(false);
				}
			}
		}


		void TutorialWindowMenu::SetContentVisible(bool visible)
		{
			for (const auto key : { KEY_VIDEO, KEY_TITLE, KEY_DESC, KEY_CLOSE_PROMPT, KEY_CLOSE_TEXT })
			{
				if (auto* ui = GetUI<UIBase>(key))
					ui->SetIsDraw(visible);
			}
		}


		void TutorialWindowMenu::SetAllVisible(bool visible)
		{
			for (const auto key : { KEY_BG, KEY_VIDEO, KEY_TITLE, KEY_DESC, KEY_CLOSE_PROMPT, KEY_CLOSE_TEXT })
			{
				if (auto* ui = GetUI<UIBase>(key))
					ui->SetIsDraw(visible);
			}
		}
	}
}
