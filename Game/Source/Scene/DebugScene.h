/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"
#include "Source/Actor/Character/Player/Player.h"


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

		bool RequesutScene(uint32_t& id, float& waitTime) override;

	private:
		static constexpr int kPlayerCount = 20;
		Vector3 m_position = Vector3::Zero;
		int m_spawnedCount = 0;
		app::actor::Player* m_players[kPlayerCount] = {};
	};
}