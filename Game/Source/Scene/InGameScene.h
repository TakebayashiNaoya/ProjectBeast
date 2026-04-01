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
		class Enemy;
		class EnemyController;
	}
	namespace ui
	{
		class Layout;
		class CountDownMenu;
		class InGameTimerMenu;
		class FinishMenu;
		class PauseScreenMenu;
		class SoundOptionMenu;
		class SearchMenu;
	}


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
		// ロードフェーズ（既存）
		//------------------------------------------------------------
		enum class LoadPhase
		{
			None, Stage, StageWait, Daddy, Children, Enemy, Camera, Ocean, Done
		};
		LoadPhase m_loadPhase = LoadPhase::None;
		int m_childIndex = 0;

		//------------------------------------------------------------
		// ゲームフェーズ（新規）
		//------------------------------------------------------------
		enum class GamePhase
		{
			CountDown,  // カウントダウン中（プレイヤー・AI・シロクマ停止）
			Playing,    // プレイ中
			Finishing,  // FINISH演出中
		};
		GamePhase m_gamePhase = GamePhase::CountDown;

		//------------------------------------------------------------
		// 終了判定
		//------------------------------------------------------------

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
		// UI レイアウト
		//------------------------------------------------------------
		ui::Layout* m_countDownLayout = nullptr;
		ui::Layout* m_timerLayout = nullptr;
		ui::Layout* m_finishLayout = nullptr;
		ui::Layout* m_remainingChildLayout = nullptr;
		ui::Layout* m_pauseLayout = nullptr;
		std::vector<ui::Layout*> m_searchLayouts;
		ui::Layout* m_enemySleepingLayout = nullptr;
		ui::Layout* m_pbWakingUpTimerLayout = nullptr;

		ui::CountDownMenu* m_countDownMenu = nullptr;
		ui::InGameTimerMenu* m_timerMenu = nullptr;
		ui::FinishMenu* m_finishMenu = nullptr;
		ui::PauseScreenMenu* m_pauseMenu = nullptr;
		std::vector<ui::SearchMenu*> m_searchMenus;

		// サウンドオプション（ポーズ中から開く用）
		ui::Layout* m_soundOptionLayout = nullptr;
		ui::SoundOptionMenu* m_soundOptionMenu = nullptr;

		// タイトルへ戻るフラグ
		bool m_goTitle = false;

		// ポーズ中のサブ状態
		enum class PauseState { Pause, SoundOption };
		PauseState m_pauseState = PauseState::Pause;

		/** 前フレームのカウントダウンタイプを保持（初期値は None） */
		ui::EnCountDownType m_lastCountType = ui::EnCountDownType::None;

		//------------------------------------------------------------
		// シーン遷移
		//------------------------------------------------------------
		bool m_nextScene = false;

		/** クリア判定用定数 */
		static constexpr int CLEAR_COUNT = 50;
	};
}