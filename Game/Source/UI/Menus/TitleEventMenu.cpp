/**
 * @file TitleEventMenu.cpp
 * @brief タイトルの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "TitleEventMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr std::array<uint32_t, static_cast<uint8_t>(TitleEventMenu::EnEventType::Num)> EVENT_ICON_KEYS =
			{
				Hash32("StartIcon"),
				Hash32("SoundIcon"),
				Hash32("RuleIcon"),
				Hash32("EndIcon")
			};

		}
		TitleEventMenu::TitleEventMenu()
			: m_isSelect(false)
			, m_isStickNeutral(true)
			, m_isDraw(false)
			, m_selectIndex(TitleEventMenu::EnEventType::Start)
			, m_gamePad(g_pad[0])
			, m_bgIcon(nullptr)
			, m_rogoIcon(nullptr)
			, m_frameIcon(nullptr)
			, m_frameBackIcon(nullptr)
			, m_eventIcon{ nullptr }
		{}


		TitleEventMenu::~TitleEventMenu()
		{}


		void TitleEventMenu::Update()
		{
			// 左スティックのX軸の値を取得。
			const float stickX = m_gamePad->GetLStickXF();

			// 入力の閾値を超えているか。
			const float InputThreshold = 0.5f;
			// スティックが中立に戻っているか。
			if (fabsf(stickX) < InputThreshold)
			{
				m_isStickNeutral = true;
			}

			if (m_isStickNeutral)
			{
				const int eventNum = static_cast<int>(EnEventType::Num);

				// 右に入力がある場合（スティックまたは十字キー）。
				if (stickX > InputThreshold || m_gamePad->IsTrigger(enButtonRight))
				{
					m_selectIndex = static_cast<EnEventType>((static_cast<uint8_t>(m_selectIndex) + 1) % eventNum);
					m_isStickNeutral = false;
					SelectVisual();
				}
				// 左に入力がある場合（スティックまたは十字キー）。
				if (stickX < -InputThreshold || m_gamePad->IsTrigger(enButtonLeft))
				{
					m_selectIndex = static_cast<EnEventType>((static_cast<uint8_t>(m_selectIndex) - 1 + eventNum) % eventNum);
					m_isStickNeutral = false;
					SelectVisual();
				}
			}

			UpdateDrawFlag();
			MenuBase::Update();
		}


		void TitleEventMenu::InitializeLogic()
		{
			// Reload後に古いポインタが残らないようリセット
			m_bgIcon = nullptr;
			m_rogoIcon = nullptr;
			m_frameIcon = nullptr;
			m_frameBackIcon = nullptr;
			m_eventIcon.fill(nullptr);

			// UIパーツを取得
			GetUIParts();

			// 最初は全て非表示
			if (m_bgIcon) m_bgIcon->SetIsDraw(false);

			if (m_rogoIcon) m_rogoIcon->SetIsDraw(false);

			if (m_frameIcon) m_frameIcon->SetIsDraw(false);

			if (m_frameBackIcon) m_frameBackIcon->SetIsDraw(false);

			for (auto* it : m_eventIcon)
			{
				if (it) it->SetIsDraw(false);
			}

			// 最初の選択状態を設定。
			SelectVisual();
		}


		void TitleEventMenu::SelectVisual()
		{
			auto* icon = m_frameBackIcon;
			// アイコンがない場合は処理しない。
			if (!icon || !m_frameIcon) return;


			constexpr uint32_t animKey = Hash32("EventColorAnim");

			// フレームの背景アイコンからアニメーションを取り除く。
			if (!icon->FindAnimation(animKey))
			{
				// アニメーションのパラメーター
				const Vector4 startColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				const Vector4 endColor = Vector4(1.0f, 1.0f, 1.0f, 0.2f);
				constexpr float duration = 0.6f;

				auto colorAnim = std::make_unique<UIColorAnimation>();
				colorAnim->SetParameter(
					startColor
					, endColor
					, duration
					, util::EasingType::EaseOut
					, util::LoopMode::PingPong
				);

				// アイコンにアニメーションを登録。
				icon->AddAnimation(animKey, std::move(colorAnim));
				icon->PlayAnimation();
			}

			// 選択されているアイコンを取得。
			auto* eventIcon = m_eventIcon.at(static_cast<uint8_t>(m_selectIndex));
			// アイコンがない場合は処理しない。
			if (!eventIcon) return;

			// アイコンの位置をフレームの背景アイコンの位置に合わせる。
			const Vector3 eventIconPosition = eventIcon->m_transform.m_localTransform.m_position;
			icon->m_transform.m_localTransform.m_position = eventIconPosition;
			m_frameIcon->m_transform.m_localTransform.m_position = eventIconPosition;
		}


		uint32_t TitleEventMenu::GetSelectKey()const
		{
			return EVENT_ICON_KEYS[static_cast<int>(m_selectIndex)];
		}



		void TitleEventMenu::GetUIParts()
		{
			if (!m_bgIcon) m_bgIcon = GetUI<UIIcon>(Hash32("TitleBackGround"));
			if (!m_rogoIcon) m_rogoIcon = GetUI<UIIcon>(Hash32("PentaktRogoIcon"));
			if (!m_frameIcon) m_frameIcon = GetUI<UIIcon>(Hash32("Frame"));
			if (!m_frameBackIcon) m_frameBackIcon = GetUI<UIIcon>(Hash32("FrameBack"));

			for (int i = 0; i < static_cast<int>(EnEventType::Num); i++)
			{
				if (!m_eventIcon.at(i))
				{
					m_eventIcon.at(i) = GetUI<UIText>(EVENT_ICON_KEYS.at(i));
				}
			}
		}

		void TitleEventMenu::UpdateDrawFlag()
		{
			GetUIParts();
			if (m_bgIcon) m_bgIcon->SetIsDraw(true);
			if (m_rogoIcon) m_rogoIcon->SetIsDraw(true);
			if (m_frameIcon) m_frameIcon->SetIsDraw(true);
			if (m_frameBackIcon) m_frameBackIcon->SetIsDraw(true);

			for (auto* it : m_eventIcon)
			{
				if (it) it->SetIsDraw(true);
			}
		}
	}
}