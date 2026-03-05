/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"


namespace app
{
	class TitleScene :public IScene
	{
		appScene(TitleScene);


	public:
		TitleScene();
		~TitleScene();

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	private:
		bool m_nextScene = false;


	private:
		SpriteRender m_titleRender;
	};
}