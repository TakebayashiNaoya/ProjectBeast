/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"
#include "Source/UI/Layout.h"


namespace app
{
	class ResultScene : public IScene
	{
		appScene(ResultScene);


	public:
		ResultScene();
		~ResultScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate()override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	public:
		// InGameScene から遷移前に呼ぶ
		static void SetResult(float clearTime, int collectedPenguin)
		{
			s_clearTime = clearTime;
			s_collectedPenguin = collectedPenguin;
		}


	private:
		bool m_nextScene = false;


	private:
		float m_clearTime;
		int m_collectedPenguin;


	private:
		static float s_clearTime;
		static int   s_collectedPenguin;


	private:
		SpriteRender m_resultRender;
		SpriteRender m_rescueRender;
		SpriteRender m_clearTimeRender;
		SpriteRender m_frame;
		SpriteRender m_titleBackRender;


	private:
		app::ui::Layout m_layout;
	};
}