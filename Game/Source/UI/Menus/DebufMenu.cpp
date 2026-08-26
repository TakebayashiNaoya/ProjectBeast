/**
 * @file DebufMenu.cpp
 * @brief 甘えん坊ペンギンから親ペンギンにデバフを掛ける演出のメニュー
 */
#include "stdafx.h"
#include "DebufMenu.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"


namespace app
{
	namespace ui
	{
		DebufMenu::DebufMenu()
			: m_isDraw(false)
		{}


		DebufMenu::~DebufMenu()
		{}


		void DebufMenu::Update()
		{
			// アニメーションが開始していない場合は、設定する。
			if (!m_startingAnimLogic.IsAnimationStarted())
			{
				m_startingAnimLogic.Initialize(
					this,
					{ "speedDownIcon" },
					{},
					Vector3(-400.0f, 0.0f, 0.0f),
					1.0f,
					{ "speedDownDigit", "percentMark" }
				);
			}

			// アニメーションが終了していない場合は、更新する。
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

			auto* pChildManager = app::actor::ChildPenguinManager::GetInstance();
			if (pChildManager)
			{
				int clingyCount = pChildManager->GetClingyCount();

				auto* digitText   = GetUI<UIText>(Hash32("speedDownDigit"));
				auto* percentText = GetUI<UIText>(Hash32("percentMark"));

				if (clingyCount > 0)
				{
					m_isDraw = true;

					// DaddyPenguinControllerと同じ計算式で減速率を求める
					constexpr int MAX_SLOW_PERCENT = 20;
					int slowPercent = min(clingyCount, MAX_SLOW_PERCENT);

					if (digitText)
					{
						digitText->SetText(std::to_string(slowPercent));
						digitText->m_isDraw = true;
					}

					if (percentText) percentText->m_isDraw = true;
				}
				else
				{
					m_isDraw = false;

					if (digitText)   digitText->m_isDraw   = false;
					if (percentText) percentText->m_isDraw = false;
				}
			}

			MenuBase::Update();
		}


		void DebufMenu::Render(RenderContext& rc)
		{
			if(!m_isDraw)
			{
				return;
			}

			MenuBase::Render(rc);
		}


		void DebufMenu::InitializeLogic()
		{
			auto* digitText = GetUI<UIText>(Hash32("speedDownDigit"));
			if (digitText) digitText->m_isDraw = false;

			auto* percentText = GetUI<UIText>(Hash32("percentMark"));
			if (percentText) percentText->m_isDraw = false;
		}
	}
}
