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

			constexpr TitleEventInfo TITLE_EVENT_ICON_KEYS[] =
			{
					{ Hash32("StartHightLightIcon") }
				,	{ Hash32("OptionHightLightIcon") }
				,	{ Hash32("ReTitleHightLightIcon") }
			};
		}
		TitleEventMenu::TitleEventMenu()
			: m_isSelect(false)
			, m_isStickNeutral(true)
			, m_selectIndex(0)
			, m_gamePad(g_pad[0])
		{
		}
		
		
		TitleEventMenu::~TitleEventMenu()
		{}
		
		
		void TitleEventMenu::Update()
		{
			// 項目数。
			const int menuSize = static_cast<int>(std::size(TITLE_EVENT_ICON_KEYS));
			
			// 左スティックのY軸の値を取得。
			const float stickY = m_gamePad->GetLStickYF();

			// 入力の闘値を超えているか。
			const float InputThreshold = 0.5f;
			// Y軸の入力がある場合。
			if (fabsf(stickY) < InputThreshold)
			{
				m_isStickNeutral = true;
			}

			if (m_isStickNeutral)
			{
				// 上に入力がある場合。
				if (stickY < -InputThreshold)
				{
					m_selectIndex = (m_selectIndex + 1) % menuSize;
					m_isStickNeutral = false;
					SelectVisual();
				}
				// 下に入力がある場合。
				if (stickY > InputThreshold)
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

				auto colorAnim = std::make_unique<UIColorAnimation>();
				colorAnim->SetParameter(
						startColor
					,	endColor
					,	duration
					,	util::EasingType::EaseOut
					,	util::LoopMode::Once
				);
				// アイコンにアニメーションを登録。
				icon->AddAnimation(Hash32("EventColorAnim"), std::move(colorAnim));
			}
		}


		void TitleEventMenu::SelectVisual()
		{
			// アニメーションにかける秒数。
			float transDuration = 0.6f;

			for (int i = 0; i < static_cast<int>(std::size(TITLE_EVENT_ICON_KEYS)); i++)
			{
				auto* icon = GetUI<UIIcon>(TITLE_EVENT_ICON_KEYS[i].key);
				if (icon == nullptr)continue;

				auto* baseAnim = icon->FindAnimation(Hash32("EventColorAnim"));
				if (baseAnim == nullptr)continue;
				
				// キャスト処理を行う。
				auto* colorAnim = static_cast<UIColorAnimation*>(baseAnim);


				// 選択されている場合はアニメーションを再生して、選択されていない場合はアニメーションを再生しない。
				if (i == m_selectIndex)
				{
					Vector4 blinkStart(1.0f, 1.0f, 1.0f, 1.0f);
					Vector4 blinkEnd(1.0f, 1.0f, 1.0f, 0.2f);

					colorAnim->SetParameter(
							blinkStart
						,	blinkEnd
						,	transDuration
						,	util::EasingType::EaseOut
						,	util::LoopMode::PingPong
					);
					// アニメーションを再生する。
					colorAnim->PlayAnimation();
				}
				else
				{
					Vector4 startColor(1.0f, 1.0f, 1.0f, 0.2f);
					Vector4 endColor(1.0f, 1.0f, 1.0f, 1.0f);

					colorAnim->SetParameter(
							startColor
						,	endColor
						,	transDuration
						,	util::EasingType::EaseOut
						,	util::LoopMode::Once
					);
					colorAnim->PlayAnimation();
				}
			}
		}

		uint32_t TitleEventMenu::GetSelectKey() const
		{
			return TITLE_EVENT_ICON_KEYS[m_selectIndex].key;
		}
	}
}