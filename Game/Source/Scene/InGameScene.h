/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"


namespace app
{
	class InGameScene : public IScene
	{
		appScene(InGameScene);


	public:
		InGameScene();
		~InGameScene();

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id) override;
	};
}