/**
 * @file ChildPenguinManager.cpp
 * @brief 子ペンギンのマネージャー
 * @author 立山
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinManager.h"

namespace app
{
	namespace actor
	{
		ChildPenguinManager* ChildPenguinManager::m_instance = nullptr;

		ChildPenguinManager::ChildPenguinManager()
		{

		}


		ChildPenguinManager::~ChildPenguinManager()
		{

		}


		void ChildPenguinManager::Start()
		{
			for (auto& cp : m_childPenguinList)
			{
				if (!cp)
				{
					continue;
				}
				cp->StartWrapper();
			}
		}

		void ChildPenguinManager::Update()
		{
			for (auto& cp : m_childPenguinList)
			{
				if (!cp)
				{
					continue;
				}
				cp->UpdateWrapper();
			}
		}


		void ChildPenguinManager::CreateChildPenguin(const int childPenguinNum)
		{
			for (int i = 0; i < childPenguinNum; i++)
			{
				m_childPenguinList.push_back(new ChildPenguin);
			}
		}


		void ChildPenguinManager::Render(RenderContext& rc)
		{
			for (auto& cp : m_childPenguinList)
			{
				if (!cp)
				{
					continue;
				}
				cp->RenderWrapper(rc);
			}
		}
	}
}