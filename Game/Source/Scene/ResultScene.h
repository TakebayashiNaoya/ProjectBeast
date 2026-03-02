/**
 * @file ResultScene.h
 * @brief リザルトシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"


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
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	private:
		bool m_nextScene = false;


	private:
		SpriteRender m_resultRender;
	};
}