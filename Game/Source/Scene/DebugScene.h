/**
 * @file DebugScene.h
 * @brief デバッグシーン
 */
#pragma once
#include "IScene.h"
#include <array>


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
		void PauseUpdate()override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	private:
		static constexpr int kPlayerCount = 20;
		Vector3 m_position = Vector3::Zero;
		int m_spawnedCount = 0;
	};
}