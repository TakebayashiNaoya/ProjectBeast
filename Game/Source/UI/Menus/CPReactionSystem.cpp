/**
 * @file CPReactionSystem.cpp
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionMenu.h"
#include "CPReactionSystem.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/UI/Layout.h"


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
			: m_reactionLayouts{}
			, m_reactionMenus{}
			, m_reactionStatusParent(nullptr)
		{}


		CPReactionSystem::~CPReactionSystem()
		{
			for (auto layout : m_reactionLayouts)
			{
				delete layout;
			}
		}


		void CPReactionSystem::Initialize()
		{
			m_reactionStatusParent = std::make_unique<CPReactionStatus>();
			m_reactionStatusParent->SetUpUI();

			for (int i = 0; i < MAX_REACTIONS_NUM; ++i)
			{
				auto* oldLayout = m_reactionLayouts.at(i);
				delete oldLayout;
				oldLayout = nullptr;

				auto* layout = new Layout();
				layout->Initialize<CPReactionMenu>(
					"Assets/parameter/UI/cpReaction/CPReaction.json"
				);

				m_reactionLayouts.at(i) = layout;
				m_reactionMenus.at(i) = layout->GetMenu<CPReactionMenu>();
				m_reactionMenus.at(i)->SetStatus(m_reactionStatusParent.get());
			}
		}


		void CPReactionSystem::Update()
		{
			for (auto layout : m_reactionLayouts)
			{
				if (layout) layout->Update();
			}
		}


		void CPReactionSystem::Render(RenderContext& rc)
		{
			for (auto layout : m_reactionLayouts)
			{
				if (layout) layout->Render(rc);
			}
		}


		CPReactionMenu* CPReactionSystem::SearchTargettableMenu()
		{
			// リアクションしていないメニューを探す
			for (auto* menu : m_reactionMenus)
			{
				if (menu->GetReactionType() == EnReactionType::None) return menu;
			}

			// 見つからなければ、先頭のメニューを上書きする
			return m_reactionMenus.at(0);
		}
	}
}