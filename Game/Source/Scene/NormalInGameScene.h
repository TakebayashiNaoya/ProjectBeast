/**
 * @file NormalInGameScene.h
 * @brief インゲームシーン（ノーマルステージ）
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

		/** STAGE_INFO_TABLE 上の自分の位置（制限時間・ステージ名・配置JSONの一次資料） */
		static constexpr int STAGE_INFO_INDEX = 1;

	protected:
		float GetTimeLimit() const override
		{
			return STAGE_INFO_TABLE[STAGE_INFO_INDEX].timeLimit;
		}

		Vector3 GetDaddySpawnPos() const override
		{
			return { 0.0f, 160.0f, 0.0f };
		}

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			/** { 0, 0, 10, 10, 80 } から改定（2026-08-23）。
			 *  旧配合は8割が世話焼きで、介助対象（おっちょこ10体）に対して稼働上限（実測約20体）を
			 *  大きく超えており、大半が何もしない背景になっていた。さらにまじめ・甘えん坊が0体のため、
			 *  チュートリアルで説明した2タイプが本編のNormalに一度も登場しなかった。
			 *  新配合は Easy { 50, 20, 10, 10, 10 } → Hard { 20, 20, 20, 20, 20 } の中間として、
			 *  トラブル役（やんちゃ・おっちょこ）を 10→15→20 と単調に増やす難易度曲線に乗せる。
			 *  世話焼きは介入対象（やんちゃ＋おっちょこ）の半数、甘えん坊は減速上限
			 *  min(隊列内の甘えん坊数, 20)% に合わせて全難易度 20 で固定。 */
			return { 35, 20, 15, 15, 15, 3000.0f, 1050.0f };
		}

		const char* GetStageJsonPath() const override
		{
			return "Assets/parameter/stage/StageObject_Normal.json";
		}

		const char* GetEnemyJsonPath() const override
		{
			return STAGE_INFO_TABLE[STAGE_INFO_INDEX].enemyLayoutJsonPath;
		}

		const char* GetWhirlpoolPositionsJsonPath() const override
		{
			return STAGE_INFO_TABLE[STAGE_INFO_INDEX].whirlpoolPositionsJsonPath;
		}

		const char* GetWhirlpoolParameterJsonPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Normal.json";
		}

		const char* GetWhirlpoolParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Normal.bin";
		}

		const char* GetFeverParameterJsonPath() const override
		{
			return "Assets/parameter/fever/feverParameter_Normal.json";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Normal.json";
		}

		const char* GetOceanParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Normal.bin";
		}

		const char* GetStageName() const override { return STAGE_INFO_TABLE[STAGE_INFO_INDEX].name; }
		const char* GetAchievementJsonPath() const override { return "Assets/parameter/achievement/AchievementList_Normal.json"; }

		const char* GetTerrainJsonPath() const override
		{
			return "Assets/parameter/stage/TerrainConfig_Normal.json";
		}
	};
}