/**
 * @file IScene.h
 * @brief シーンの基底クラス
 * @author 立山
 */
#pragma once
#include "IScene.h"


namespace app
{
	class DebugScene : public IScene
	{
		appScene(DebugScene);


	public:
		DebugScene();
		~DebugScene();

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id) override;
	};
}