/**
 * @file TitleEventMenu.cpp
 * @brief タイトルの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "TitleEventMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			struct TitleEventInfo
			{
				uint32_t key;
			};

			// タイトルイベントのアイコンのキー。
			constexpr TitleEventInfo TITLE_EVENT_ICON_KEYS[] =
			{
					{ Hash32("StartFrameBackIcon") }
				,	{ Hash32("OptionFrameBackIcon") }
				,	{ Hash32("RuleFrameBackIcon")   }
				,	{ Hash32("EndFrameBackIcon")    }
			};

			// タイトルイベントのフレームアイコンのキー。
			constexpr TitleEventInfo TITLE_EVENT_FRAME_ICON_KEYS[] =
			{
					{ Hash32("StartFrameIcon")}
				,	{ Hash32("OptionFrameIcon")}
				,	{ Hash32("RuleFrameIcon")  }
				,	{ Hash32("EndFrameIcon")   }
			};
		}
		TitleEventMenu::TitleEventMenu()
			: m_isSelect(false)
			, m_isStickNeutral(true)
			, m_selectIndex(0)
			, m_gamePad(g_pad[0])
		{}


		TitleEventMenu::~TitleEventMenu()
		{}


		void TitleEventMenu::Update()
		{
			// 項目数。
			const int menuSize = static_cast<int>(std::size(TITLE_EVENT_ICON_KEYS));

			// 左スティックのY軸の値を取得。
			const float stickY = m_gamePad->GetLStickXF();

			// 入力の闘値を超えているか。
			const float InputThreshold = 0.5f;
			// X軸の入力がある場合。
			if (fabsf(stickY) < InputThreshold)
			{
				m_isStickNeutral = true;
			}

			if (m_isStickNeutral)
			{
				// 右に入力がある場合。
				if (stickY > InputThreshold)
				{
					m_selectIndex = (m_selectIndex + 1) % menuSize;
					m_isStickNeutral = false;
					SelectVisual();
				}
				// 左に入力がある場合。
				if (stickY < -InputThreshold)
				{
					m_selectIndex = (m_selectIndex - 1 + menuSize) % menuSize;
					m_isStickNeutral = false;
					SelectVisual();
				}
			}
			MenuBase::Update();
		}


		void TitleEventMenu::InitializeLogic()
		{
			// 共通のアニメーションパラメーター。
			Vector4 startColor(1.0f, 1.0f, 1.0f, 1.0f);
			Vector4 endColor(1.0f, 1.0f, 1.0f, 0.2f);
			float duration = 0.6f;
			for (const auto& info : TITLE_EVENT_ICON_KEYS)
			{
				auto* icon = GetUI<UIIcon>(info.key);
				if (icon == nullptr)return;

				// 最初はアイコンを非表示にする。
				icon->SetIsDraw(false);

				auto colorAnim = std::make_unique<UIColorAnimation>();
				colorAnim->SetParameter(
					startColor
					, endColor
					, duration
					, util::EasingType::EaseOut
					, util::LoopMode::Once
				);
				// アイコンにアニメーションを登録。
				icon->AddAnimation(Hash32("EventColorAnim"), std::move(colorAnim));
			}

			// 最初の選択状態を設定。
			SelectVisual();
		}


		void TitleEventMenu::SelectVisual()
		{
			// アニメーションにかける秒数。
			float transDuration = 0.6f;
			// 項目数。
			const int menuSize = static_cast<int>(std::size(TITLE_EVENT_ICON_KEYS));

			for (int i = 0; i < menuSize; i++)
			{
				// 選択されている時はスキップ。
				if (i == m_selectIndex) continue;

				// アイコンとフレームアイコンを取得。
				auto* icon = GetUI<UIIcon>(TITLE_EVENT_ICON_KEYS[i].key);
				auto* frame = GetUI<UIIcon>(TITLE_EVENT_FRAME_ICON_KEYS[i].key);
				if (icon == nullptr && frame == nullptr) continue;

				// 非選択中はアイコンを非表示にする。
				icon->SetIsDraw(false);
				// 非選択中はフレームアイコンを非表示にする。
				frame->SetIsDraw(false);

				auto* colorAnim = static_cast<UIColorAnimation*>
					(icon->FindAnimation(Hash32("EventColorAnim")));
				if (colorAnim)
				{
					colorAnim->StopAnimation();
				}
			}

			// 選択中のアイコンとフレームアイコンを表示する。
			auto* selectedIcon = GetUI<UIIcon>(TITLE_EVENT_ICON_KEYS[m_selectIndex].key);
			auto* selectedFrame = GetUI<UIIcon>(TITLE_EVENT_FRAME_ICON_KEYS[m_selectIndex].key);
			if (selectedIcon)
			{
				selectedIcon->SetIsDraw(true);
				auto* colorAnim = static_cast<UIColorAnimation*>
					(selectedIcon->FindAnimation(Hash32("EventColorAnim")));
				if (colorAnim)
				{
					colorAnim->PlayAnimation();
				}
			}
			if (selectedFrame)
			{
				selectedFrame->SetIsDraw(true);
			}
		}


		uint32_t TitleEventMenu::GetSelectKey()const
		{
			return TITLE_EVENT_ICON_KEYS[m_selectIndex].key;
		}
	}
}