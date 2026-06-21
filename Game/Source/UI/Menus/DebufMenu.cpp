/**
 * @file DebufMenu.cpp
 * @brief 甘えん坊ペンギンから親ペンギンにデバフを掛ける演出のメニュー
 * @author 忽那
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
					{}, // アイコンUIはなし。
					{}, // 数字UIはなし。
					Vector3(-400.0f, 0.0f, 0.0f), // 開始位置のオフセット。画面上部から少し下にずらす。
					1.0f, // アニメーションの持続時間。1秒。
					{ "counterDigit", "percentDigit" } // テキストUIはなし。
				);
			}

			// アニメーションが開始している場合は、更新する。
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

			auto* pChildManager = app::actor::ChildPenguinManager::GetInstance();
			if (pChildManager)
			{
				// 現在の甘えん坊の数を取得。
				int clingyCount = pChildManager->GetClingyCount();

				// 文字と数字を両方表示するもの。
				auto* countText = GetUI<UIText>(Hash32("counterDigit"));
				auto* percentText = GetUI<UIText>(Hash32("percentDigit"));

				if (clingyCount > 0)
				{
					// 甘えん坊がいるのでメニュー自体を描画ONにする
					m_isDraw = true;

					// 表示するデバフ率の計算
					int debuffPercent = clingyCount * 1;

					if (countText)
					{
						countText->SetText(std::to_string(clingyCount) + "匹");
						countText->m_isDraw = true;
					}

					if (percentText)
					{
						percentText->SetText(std::to_string(debuffPercent) + "%");
						percentText->m_isDraw = true;
					}
				}
				else
				{
					// 甘えん坊がいない時は、メニュー自体を非表示にする
					m_isDraw = false;

					if (countText)   countText->m_isDraw = false;
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
			auto* textA = GetUI<UIText>(Hash32("counterDigit"));
			if (textA) textA->m_isDraw = false;

			auto* textB = GetUI<UIText>(Hash32("percentDigit"));
			if (textB) textB->m_isDraw = false;
		}
	}
}