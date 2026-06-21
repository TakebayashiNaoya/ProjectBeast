/**
 * @file HardInGameScene.h
 * @brief インゲームシーン（ハードステージ）
 * @author 竹林
 */
#pragma once
#include "InGameSceneBase.h"


namespace app
{
	/**
	 * @brief ハードステージ
	 * @detail ステージ固有のパラメータを返す。
	 *         ゲームロジックは InGameSceneBase が担う。
	 */
	class HardInGameScene : public InGameSceneBase
	{
		appScene(HardInGameScene);

	protected:
		float GetTimeLimit() const override
		{
			return 300.0f;
		}

		Vector3 GetDaddySpawnPos() const override
		{
			return { 0.0f, 274.0f, 100.0f };
		}

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			return { 20, 20, 20, 20, 20, 3000.0f };
		}

		const char* GetStageJsonPath() const override
		{
			return "Assets/parameter/stage/StageObject_Hard.json";
		}

		const char* GetEnemyJsonPath() const override
		{
			return "Assets/parameter/character/enemy/EnemyLayout_Hard.json";
		}

		const char* GetWhirlpoolPositionsJsonPath() const override
		{
			return "Assets/parameter/stage/whirlpoolPositions_Hard.json";
		}

		const char* GetWhirlpoolParameterJsonPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Hard.json";
		}

		const char* GetWhirlpoolParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Hard.bin";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Hard.json";
		}

		const char* GetOceanParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Hard.bin";
		}

		const char* GetStageName() const override
		{
			return "Hard";
		}

		const char* GetAchievementJsonPath() const override
		{
			return "Assets/parameter/achievement/AchievementList_Hard.json";
		}

		const char* GetTerrainJsonPath() const override
		{
			return "Assets/parameter/stage/TerrainConfig_Hard.json";
		}
	};
}
