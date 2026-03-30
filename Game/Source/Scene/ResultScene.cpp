/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#include "stdafx.h"
#include "ResultScene.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"
#include "Source/UI/UIParts.h" 
#include "Source/Util/CRC32.h" 
#include "TitleScene.h"


namespace app
{
	float ResultScene::s_clearTime = 0.0f;
	int   ResultScene::s_collectedPenguin = 0;


	ResultScene::ResultScene()
		:m_clearTime(0.0f)
		, m_collectedPenguin(0)
	{}


	ResultScene::~ResultScene()
	{}


	bool ResultScene::Start()
	{
		m_resultRender.Init("Assets/spriteData/Scene/NorthPole.DDS", 1920.0f, 1080.0f);

		m_clearTime = s_clearTime;
		m_collectedPenguin = s_collectedPenguin;

		// JSONレイアウトを読み込んでUIを構築
		m_layout.Initialize<app::ui::MenuBase>("Assets/parameter/result/result.json");

		// 取得した値を UIDigit にセット
		auto* timeDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultTimeDigit"));
		auto* scoreDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultScoreDigit"));

		if (timeDigit)
		{
			timeDigit->SetNumber(static_cast<int>(m_clearTime));
			timeDigit->m_isDraw = true;
		}
		if (scoreDigit)
		{
			scoreDigit->SetNumber(m_collectedPenguin);
			scoreDigit->m_isDraw = true;
		}


		return true;
	}


	void ResultScene::Update()
	{
		m_layout.Update();   // UIの毎フレーム更新

		auto* timeDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultTimeDigit"));
		auto* scoreDigit = m_layout.GetMenu<app::ui::MenuBase>()->GetUI<app::ui::UIDigit>(Hash32("ResultScoreDigit"));

		if (timeDigit)  timeDigit->SetNumber(static_cast<int>(m_clearTime));
		if (scoreDigit) scoreDigit->SetNumber(m_collectedPenguin);

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			m_nextScene = true;
		}
	}


	void ResultScene::PauseUpdate()
	{}


	void ResultScene::Render(RenderContext& rc)
	{
		m_resultRender.Draw(rc);
		m_layout.Render(rc); // UIの描画
	}


	bool ResultScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_nextScene) {
			id = TitleScene::ID();
			waitTime = 3.0f;
			return true;
		}
		return false;
	}
}