/**
 * @file WpWarningMenu.cpp
 * @brief WpWarningのメニュークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "WpWarningMenu.h"

#include "Source/Nature/Whirlpool.h"

#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		WpWarningMenu::WpWarningMenu()
			: m_speechBubble(nullptr)
			, m_warning(nullptr)
			, m_status(nullptr)
			, m_animStatus(nullptr)
			, m_whirlpool(nullptr)
			, m_isDraw(false)
		{}


		void WpWarningMenu::InitializeLogic()
		{
			m_speechBubble = nullptr;
			m_warning = nullptr;

			m_isDraw = false;

			m_animStatus.reset();
			m_animStatus = std::make_unique<WpWarningAnimStatus>();


			// アイコンを取得
			m_speechBubble = GetUI<UIIcon>(Hash32("speechBubble"));
			m_warning = GetUI<UIIcon>(Hash32("warning"));

			std::vector<UIIcon*> icons =
			{
				m_speechBubble,
				m_warning
			};

			for (auto* it : icons)
			{
				if (!it) continue;

				it->m_isDraw = m_isDraw;
			}
		}


		void WpWarningMenu::Update()
		{
			UpdateIconPosition();

			WpWarning::Update();
		}


		void WpWarningMenu::UpdateIconPosition()
		{
			std::vector<UIIcon*> icons =
			{
				m_speechBubble,
				m_warning
			};

			if (!m_whirlpool || !m_status)
			{
				m_isDraw = false;
				for (auto* it : icons)
				{
					if (!it) continue;
					ResetAnimation(it);
					it->m_isDraw = m_isDraw;

				}
				return;
			}

			// 座標の更新
			const Vector3 wpPosition = m_whirlpool->GetTransform().m_position;

			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, wpPosition);
			const Vector3 prevPosition = Vector3(
				screenPos.x,
				screenPos.y + m_status->GetIconOffsetY(),
				0.0f
			);


			for (auto* it : icons)
			{
				if (!it) continue;

				if (!it->IsPlayAnimation())
				{
					SetAnimation(it);
					it->PlayAnimation();
				}

				it->m_transform.m_localTransform.m_position = prevPosition;

				it->m_isDraw = m_isDraw;
			}
		}


		void WpWarningMenu::SetAnimation(UIIcon* icon)
		{
			const uint32_t animKey = animKey::WP_GROW_AND_SHRINK_ANIM_KEY;
			icon->RemoveAnimation(animKey);

			UIAnimationFactory::Attach<UIScaleAnimation>(icon, animKey);

			auto* growAndShrinkAnim = icon->FindAnimation(animKey);


			if (growAndShrinkAnim)
			{
				growAndShrinkAnim->PlayAnimation();
			}
		}


		void WpWarningMenu::ResetAnimation(UIIcon* icon)
		{
			const uint32_t growAndShrinkAnimKey = animKey::WP_GROW_AND_SHRINK_ANIM_KEY;
			icon->StopAnimation();
			icon->RemoveAnimation(growAndShrinkAnimKey);
		}
	}
}