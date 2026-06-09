/**
 * @file TutorialInGameScene.h
 * @brief チュートリアルステージ
 * @author 竹林
 */
#pragma once
#include "InGameSceneBase.h"


namespace app
{
	/**
	 * @brief チュートリアルステージ
	 * @detail ステージ固有のパラメータを返す。
	 *         ゲームロジックは InGameSceneBase が担う。
	 *         今後 OnLoadComplete / OnUpdatePlaying / OnRenderPlaying を
	 *         オーバーライドして TutorialController などを追加する。
	 */
	class TutorialInGameScene : public InGameSceneBase
	{
		appScene(TutorialInGameScene);

	protected:
		float GetTimeLimit() const override { return 300.0f; }

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			return { 2, 2, 2, 2, 2, 3000.0f };
		}

		const char* GetStageJsonPath() const override
		{
			return "Assets/parameter/stage/StageObject_Tutorial.json";
		}

		const char* GetEnemyJsonPath() const override
		{
			return "Assets/parameter/character/enemy/EnemyLayout_Tutorial.json";
		}

		const char* GetWhirlpoolPositionsJsonPath() const override
		{
			return "Assets/parameter/stage/whirlpoolPositions_Tutorial.json";
		}

		const char* GetWhirlpoolParameterJsonPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Tutorial.json";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Tutorial.json";
		}
	};
}
