/**
 * @file InGameScene.h
 * @brief インゲームシーン
 * @author 立山、竹林
 */
#pragma once
#include "IScene.h"
#include "Source/Camera/CameraSteering.h"
#include "Source/UI/CountDownMenu.h"


namespace app
{
	/** 前方宣言 */
	namespace actor
	{
		class DaddyPenguin;
		class ChildPenguin;
	}
	namespace ui
	{
		class CountDownMenu;
		class FinishMenu;
		class PauseScreenMenu;
		class SoundOptionMenu;
	}

	class InGameUIManager;


	/**
	 * @brief インゲームシーン
	 */
	class InGameScene : public IScene
	{
		appScene(InGameScene);

	public:
		InGameScene();
		~InGameScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;

		bool IsLoaded() const { return m_loadPhase == LoadPhase::Done; }


	private:
		//------------------------------------------------------------
		// ロードフェーズ
		//------------------------------------------------------------
		enum class LoadPhase
		{
			None, Stage, StageWait, Daddy, Children, Enemy, Camera, Ocean, Done
		};
		LoadPhase m_loadPhase = LoadPhase::None;
		int m_childIndex = 0;

		//------------------------------------------------------------
		// ゲームフェーズ
		//------------------------------------------------------------
		enum class GamePhase
		{
			CountDown,  /** カウントダウン中（プレイヤー・AI・シロクマ停止） */
			Playing,    /** プレイ中 */
			Finishing,  /** FINISH演出中 */
		};
		GamePhase m_gamePhase = GamePhase::CountDown;

		/** ロード完了後のゲームフェーズ更新をまとめた関数 */
		void UpdateGamePhase();

		//------------------------------------------------------------
		// アクター
		//------------------------------------------------------------
		static constexpr int CHILD_PENGUIN_NUM = 100;
		actor::DaddyPenguin* m_daddyPenguin = nullptr;
		actor::ChildPenguin* m_childPenguins[CHILD_PENGUIN_NUM] = {};

		camera::CameraSteering m_cameraSteering;

		Ocean* m_ocean = nullptr;
		SkyCube* m_skyCube = nullptr;

		//------------------------------------------------------------
		// UIマネージャー
		//------------------------------------------------------------
		InGameUIManager* m_uiManager = nullptr;

		//------------------------------------------------------------
		// ポーズ
		//------------------------------------------------------------
		enum class PauseState { Pause, SoundOption, Tutorial };
		PauseState m_pauseState = PauseState::Pause;

		//------------------------------------------------------------
		// シーン遷移
		//------------------------------------------------------------
		bool m_nextScene = false;
		bool m_goTitle = false;

		/** 前フレームのカウントダウンタイプを保持（初期値は None） */
		ui::EnCountDownType m_lastCountType = ui::EnCountDownType::None;

		/** クリア判定用定数 */
		static constexpr int CLEAR_COUNT = 50;
	};
}