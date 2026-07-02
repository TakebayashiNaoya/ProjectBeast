/**
 * @file BearReactionMenu.cpp
 * @brief クマのリアクションメニュークラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "BearReactionMenu.h"


namespace app
{
	namespace ui
	{
		void BearReactionMenu::SetTargetPosition(const Vector3& position)
		{
			m_speechBubble->m_transform.m_localTransform.m_position = position;
			m_tongue->m_transform.m_localTransform.m_position = position;
			m_bed->m_transform.m_localTransform.m_position = position;
			m_feelingDown->m_transform.m_localTransform.m_position = position;
		}


		void BearReactionMenu::SetReactionType(const EnBearReactionType reactionType)
		{
			switch (reactionType)
			{
			case EnBearReactionType::Tongue:
			{
				m_speechBubble->m_isDraw = true;
				m_tongue->m_isDraw = true;
				m_bed->m_isDraw = false;
				m_feelingDown->m_isDraw = false;

				break;
			}
			case EnBearReactionType::Debuff:
			{
				m_speechBubble->m_isDraw = true;
				m_tongue->m_isDraw = false;
				m_bed->m_isDraw = false;
				m_feelingDown->m_isDraw = true;

				break;
			}
			case EnBearReactionType::Bed:
			{
				m_speechBubble->m_isDraw = true;
				m_tongue->m_isDraw = false;
				m_bed->m_isDraw = true;
				m_feelingDown->m_isDraw = false;

				break;
			}
			case EnBearReactionType::None:
			{
				m_speechBubble->m_isDraw = false;
				m_tongue->m_isDraw = false;
				m_bed->m_isDraw = false;
				m_feelingDown->m_isDraw = false;

				break;
			}
			default:
				K2_ASSERT(false, "タイプ不正");
				break;
			}
		}


		void BearReactionMenu::InitializeLogic()
		{
			m_speechBubble = GetUI<UIIcon>(Hash32("speechBubble"));
			m_tongue = GetUI<UIIcon>(Hash32("tongueReaction"));
			m_bed = GetUI<UIIcon>(Hash32("bedReaction"));
			m_feelingDown = GetUI<UIIcon>(Hash32("feelingDownReaction"));

			auto initializeUI = [](UIIcon* ui)
				{
					K2_ASSERT(ui, "UIが見つかりません");
					ui->m_isDraw = false;
				};

			initializeUI(m_speechBubble);
			initializeUI(m_tongue);
			initializeUI(m_bed);
			initializeUI(m_feelingDown);
		}


		void BearReactionMenu::Update()
		{
			MenuBase::Update();
		}


		BearReactionMenu::BearReactionMenu()
			: m_speechBubble(nullptr)
			, m_tongue(nullptr)
			, m_bed(nullptr)
			, m_feelingDown(nullptr)
		{}


		BearReactionMenu::~BearReactionMenu()
		{}
	}
}