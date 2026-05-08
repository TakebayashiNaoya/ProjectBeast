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
			auto* menu = SeachTargettableMenu();
			menu->SetTarget(childPenguin);
			menu->PlayUIAnimation(type);
		}


		CPReactionSystem::CPReactionSystem()
			: m_reactionLayouts{},
			m_reactionMenus{}
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
			}
		}


		void CPReactionSystem::Update()
		{
			// 子ペンギンマネージャーを取得
			auto cm = actor::ChildPenguinManager::GetInstance();

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


		CPReactionMenu* CPReactionSystem::SeachTargettableMenu()
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