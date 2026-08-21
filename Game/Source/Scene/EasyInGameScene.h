/**
 * @file EasyInGameScene.h
 * @brief インゲームシーン（イージーステージ）
 * @author 竹林
 */
#pragma once
#include "InGameSceneBase.h"


namespace app
{
	/**
	 * @brief イージーステージ
	 * @detail ステージ固有のパラメータを返す。
	 *         ゲームロジックは InGameSceneBase が担う。
	 */
	class EasyInGameScene : public InGameSceneBase
	{
		appScene(EasyInGameScene);

	protected:
		float GetTimeLimit() const override
		{
			return 120.0f;
		}

		Vector3 GetDaddySpawnPos() const override
		{
			return { 0.0f, 110.0f, -100.0f };
		}

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			/** やんちゃ・おっちょこちょい・世話焼きを 0 から 10 ずつに変えた。
			 *  カッパ祭の 43セッション・合計86分で clumsy_fall と naughty_disobey が
			 *  0件だったのは地形のせいではなく、転倒判定が ClumsyChildPenguinAI の中に、
			 *  いたずら判定が NaughtyChildPenguinAI の中にしか無いため。
			 *  この2つが 0体では、地形を作り直しても構造上ずっと 0 件のままになる。
			 *  甘えん坊は 50→20。min(隊列内の甘えん坊数, 20)% の減速が常時 0.80 に
			 *  張り付いていたので、「移動しやすく」の向きに合わせて下げた。
			 *  生成半径は流氷原の外縁 R_FIELD 3000 に合わせる（Normal と同じ理由）。 */
			return { 50, 20, 10, 10, 10, 3000.0f, 1000.0f };
		}

		const char* GetStageJsonPath() const override
		{
			return "Assets/parameter/stage/StageObject_Easy.json";
		}

		const char* GetEnemyJsonPath() const override
		{
			return "Assets/parameter/character/enemy/EnemyLayout_Easy.json";
		}

		const char* GetWhirlpoolPositionsJsonPath() const override
		{
			return "Assets/parameter/stage/whirlpoolPositions_Easy.json";
		}

		const char* GetWhirlpoolParameterJsonPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Easy.json";
		}

		const char* GetWhirlpoolParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Easy.bin";
		}

		const char* GetFeverParameterJsonPath() const override
		{
			return "Assets/parameter/fever/feverParameter_Easy.json";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Easy.json";
		}

		const char* GetOceanParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Easy.bin";
		}

		const char* GetStageName() const override
		{
			return "Easy";
		}

		const char* GetAchievementJsonPath() const override
		{
			return "Assets/parameter/achievement/AchievementList_Easy.json";
		}

		const char* GetTerrainJsonPath() const override
		{
			return "Assets/parameter/stage/TerrainConfig_Easy.json";
		}
	};
}
