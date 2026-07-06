/**
 * @file TutorialInGameScene.h
 * @brief チュートリアルステージ
 * @author 竹林
 */
#pragma once
#include "InGameSceneBase.h"
#include "TutorialController.h"


namespace app
{
	/**
	 * @brief チュートリアルステージ
	 *
	 * ■ チュートリアルの流れ
	 *   - プレイヤーが各ギミックに近づいたとき（海は水中接触）、
	 *     そのギミック種別の初回のみチュートリアルウィンドウを表示する。
	 *   - 複数同時トリガー時はキューに積んで順番に表示。
	 *
	 * ■ 詳細は TutorialController を参照
	 */
	class TutorialInGameScene : public InGameSceneBase
	{
		appScene(TutorialInGameScene);

	protected:
		//------------------------------------------------------------
		// ステージ固有パラメータ
		//------------------------------------------------------------
		float GetTimeLimit() const override 
		{
			return 300.0f; 
		}

		Vector3 GetDaddySpawnPos() const override
		{
			return { 0.0f, 80.0f, 0.0f };
		}

		PenguinSpawnConfig GetPenguinConfig() const override
		{
			return { 3, 3, 3, 3, 3, 1500.0f };
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

		const char* GetWhirlpoolParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/whirlpoolParameter_Tutorial.bin";
		}

		const char* GetFeverParameterJsonPath() const override
		{
			return "Assets/parameter/fever/feverParameter_Tutorial.json";
		}

		const char* GetOceanParameterJsonPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Tutorial.json";
		}

		const char* GetOceanParameterBinaryPath() const override
		{
			return "Assets/parameter/nature/oceanParameter_Tutorial.bin";
		}

		const char* GetStageName() const override 
		{
			return "Tutorial"; 
		}

		const char* GetAchievementJsonPath() const override 
		{
			return "Assets/parameter/achievement/AchievementList_Tutorial.json"; 
		}

		const char* GetTerrainJsonPath() const override
		{
			return "Assets/parameter/stage/TerrainConfig_Tutorial.json";
		}

		//------------------------------------------------------------
		// フックのオーバーライド
		//------------------------------------------------------------
		void OnLoadComplete()                    override;
		void OnUpdatePlaying()                   override;
		void OnRenderPlaying(RenderContext& rc)  override;
		bool OnPauseUpdate()                     override;
		bool OnPauseRender(RenderContext& rc)    override;


	private:
		TutorialController m_tutorialController;
	};
}
