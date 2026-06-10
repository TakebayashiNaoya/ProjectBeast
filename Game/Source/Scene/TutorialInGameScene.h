/**
 * @file TutorialInGameScene.h
 * @brief チュートリアルステージ
 * @author 竹林
 */
#pragma once
#include "InGameSceneBase.h"
#include "Source/UI/Layout.h"
#include "Source/UI/Menus/TutorialWindowMenu.h"


namespace app
{
	/**
	 * @brief チュートリアルステージ
	 *
	 * ■ チュートリアルウィンドウの流れ
	 *   1. Playing フェーズ初回: window[0] を Open() してゲームをポーズ。
	 *   2. OnPauseUpdate() でウィンドウのアニメーション・入力を更新。
	 *   3. IsClosedByUser() で次のウィンドウへ進み、全部終わったらポーズ解除。
	 *   4. 通常プレイへ移行。
	 *
	 * ■ ウィンドウを増やすには
	 *   1. WINDOW_COUNT を増やす。
	 *   2. OnLoadComplete() に Initialize の行を追加。
	 *   3. JSON を複製して clipPath / asset だけ変える。
	 */
	class TutorialInGameScene : public InGameSceneBase
	{
		appScene(TutorialInGameScene);

	protected:
		//------------------------------------------------------------
		// ステージ固有パラメータ
		//------------------------------------------------------------
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

		//------------------------------------------------------------
		// フックのオーバーライド
		//------------------------------------------------------------
		void OnLoadComplete() override;
		void OnUpdatePlaying() override;
		bool OnPauseUpdate() override;
		bool OnPauseRender(RenderContext& rc) override;

	private:
		/** ウィンドウの数。増やす場合は OnLoadComplete() の初期化行も追加する */
		static constexpr int WINDOW_COUNT = 2;

		/** 各ウィンドウの Layout（所有権を持つ） */
		ui::Layout m_windowLayouts[WINDOW_COUNT];

		/** 現在表示中のウィンドウインデックス（-1 = まだ開始していない） */
		int m_currentWindowIndex = -1;

		/** すべてのウィンドウを表示し終えたか */
		bool m_allWindowsDone = false;

		/** チュートリアルウィンドウ用ポーズ中か（通常ポーズと区別） */
		bool m_isTutorialWindowPause = false;
	};
}
