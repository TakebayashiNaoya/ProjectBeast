/**
 * @file CPReactionSystem.cpp
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionSystem.h"

#include "CPReactionStatus.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"


namespace app
{
	namespace ui
	{
		void CPReactionSystem::SetTarget(actor::ChildPenguin* childPenguin, const EnReactionType type)
		{
			auto* menu = SearchTargettableMenu();
			menu->SetTarget(childPenguin);
			menu->PlayUIAnimation(type);
		}


		CPReactionSystem::CPReactionSystem()
			: m_reactionPackets{}
			, m_reactionStatusParent(nullptr)
		{}


		CPReactionSystem::~CPReactionSystem()
		{}


		void CPReactionSystem::Initialize()
		{
			m_reactionStatusParent = std::make_unique<CPReactionStatus>();
			m_reactionStatusParent->SetUp();

			for (int i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				auto& it = m_reactionPackets.at(i);
				it.Initialize("Assets/parameter/UI/cpReaction/CPReaction.json");

				it.GetMenu()->SetStatus(m_reactionStatusParent.get());
			}
		}


		void CPReactionSystem::Update()
		{
			for (auto& packet : m_reactionPackets)
			{
				packet.Update();
			}
		}


		void CPReactionSystem::Render(RenderContext& rc)
		{
			for (auto& packet : m_reactionPackets)
			{
				packet.Render(rc);
			}
		}


		CPReactionMenu* CPReactionSystem::SearchTargettableMenu()
		{
			// リアクションしていないメニューを探す
			for (auto& packet : m_reactionPackets)
			{
				auto* menu = packet.GetMenu();
				if (menu && menu->GetReactionType() == EnReactionType::None) return menu;
			}

			// 見つからなければ、先頭のメニューを上書きする
			return m_reactionPackets.at(0).GetMenu();
		}
	}
}