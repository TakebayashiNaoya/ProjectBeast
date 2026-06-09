/**
 * @file NormalInGameScene.h
 * @brief インゲームシーン（ノーマルステージ）
 * @author 立山、竹林
 */
#pragma once
#include "InGameSceneBase.h"


namespace app
{
	/**
	 * @brief ノーマルステージ
	 * @detail ステージ固有のパラメータを返す。
	 *         ゲームロジックは InGameSceneBase が担う。
	 */
	class NormalInGameScene : public InGameSceneBase
	{
		appScene(NormalInGameScene);

	protected:
		float GetTimeLimit() const override { return 180.0f; }

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			return { 20, 20, 20, 20, 20, 3000.0f };
		}

		const char* GetStageJsonPath() const override
		{
			return "Assets/parameter/stage/StageObject_Normal.json";
		}

		const char* GetEnemyJsonPath() const override
		{
			return "Assets/parameter/character/enemy/EnemyLayout_Normal.json";
		}

		const char* GetWhirlpoolPositionsJsonPath() const override
		{
			return "Assets/parameter/stage/whirlpoolPositions_Normal.json";
		}

		const char* GetWhirlpoolParameterJsonPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Normal.json";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Normal.json";
		}
	};
}