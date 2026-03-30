/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"
#include "Source/Camera/CameraSteering.h"


namespace app
{
	namespace actor {
		class DaddyPenguin;
		class ChildPenguin;
		class Enemy;
		class EnemyController;
	}

	class InGameScene : public IScene
	{
		appScene(InGameScene);


	public:
		InGameScene();
		~InGameScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate()override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;

		bool IsLoaded() const { return m_phase == LoadPhase::Done; }

	private:
		bool m_nextScene = false;


		static constexpr int CHILD_PENGUIN_NUM = 100;
		actor::DaddyPenguin* m_daddyPenguin = nullptr;
		actor::ChildPenguin* m_childPenguins[CHILD_PENGUIN_NUM] = {};

		camera::CameraSteering m_cameraSteering;

		enum class LoadPhase { None, Stage, Daddy, Children, Enemy, Camera, Ocean, Done };
		LoadPhase m_phase = LoadPhase::None;
		int m_childIndex = 0;

		Ocean* m_ocean = nullptr;
	};
}