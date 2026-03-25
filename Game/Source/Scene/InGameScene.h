/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"


namespace app
{
	namespace actor {
		class DaddyPenguin;
		class ChildPenguin;
		class IStageObject;
	}

	class InGameScene : public IScene
	{
		appScene(InGameScene);


	public:
		InGameScene();
		~InGameScene();

		bool Start() override;
		void Update() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	private:
		bool m_nextScene = false;


		static constexpr int CHILD_PENGUIN_NUM = 100;
		actor::IStageObject* m_stage = nullptr;
		actor::DaddyPenguin* m_daddyPenguin = nullptr;
		actor::ChildPenguin* m_childPenguins[CHILD_PENGUIN_NUM] = {};
	};
}