/**
 * @file WpWarningMenu.cpp
 * @brief WpWarningのメニュークラス
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
			, m_isDraw(false)
		{}


		void WpWarningMenu::InitializeLogic()
		{
			m_speechBubble = nullptr;
			m_warning = nullptr;


			m_animStatus.reset();
			m_animStatus = std::make_unique<WpWarningAnimStatus>();

			auto CheckIcon = [this](const char* name)
				{
					auto* ui = GetUI<UIIcon>(Hash32(name));
					K2_ASSERT(ui, "取得失敗");
					ui->m_isDraw = false;
					return ui;
				};

			// アイコンを取得
			m_speechBubble = CheckIcon("speechBubble");
			m_warning = CheckIcon("warning");
		}


		void WpWarningMenu::Update()
		{
			UpdateIconPosition();

			MenuBase::Update();
		}


		void WpWarningMenu::UpdateIconPosition()
		{
			auto UpdateAnimation = [this](UIIcon* ui)
				{
					if (!ui->IsPlayAnimation())
					{
						SetAnimation(ui);
					}
				};

			UpdateAnimation(m_speechBubble);
			UpdateAnimation(m_warning);

			if (!m_isDraw)
			{
				ResetAnimation(m_speechBubble);
				ResetAnimation(m_warning);
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
			if (icon->FindAnimation(growAndShrinkAnimKey))
			{
				icon->StopAnimation();
				icon->RemoveAnimation(growAndShrinkAnimKey);
			}
		}
	}
}