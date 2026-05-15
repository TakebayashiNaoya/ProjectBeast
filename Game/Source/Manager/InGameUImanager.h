/**
 * @file InGameUIManager.h
 * @brief インゲームUIの生成・更新・描画・配線を管理するクラス
 * @author 竹林
 */
#pragma once
#include <vector>


namespace app
{
	/** 前方宣言 */
	namespace actor
	{
		class DaddyPenguin;
		class Enemy;
	}
	namespace ui
	{
		class Layout;
		class CountDownMenu;
		class InGameTimerMenu;
		class FinishMenu;
		class PauseScreenMenu;
		class SoundOptionMenu;
		class TutorialMenu;
		class SearchMenu;
		class RemainingChildMenu;
		class EnemySleepingMenu;
		class PBWakingUpTimerMenu;
		class IglooPromptMenu;
		class InGameAchievementMenu;
		class MiniMapMenu;
		class TitleEventMenu;
		class AchievementNotificationMenu;
		class InGameButtonMenu;

		class CPReactionSystem;
		class WpWarningSystem;
	}


	/**
	 * @brief インゲームUIの管理クラス
	 * @detail Layoutの生成・所有・更新・描画をまとめ、
	 *         BattleManagerへのUI通知functionの配線を担う。
	 *         InGameSceneはこのクラスを保持するだけでよい。
	 */
	class InGameUIManager
	{
	private:
		InGameUIManager();
		~InGameUIManager();


	public:
		/**
		 * @brief 初期化
		 * @detail Layoutの生成とBattleManagerへの配線を行う
		 * @param daddyPenguin プレイヤーポインタ（睡眠クマ探索の基準座標に使用）
		 */
		void Initialize(actor::DaddyPenguin* daddyPenguin);

		/**
		 * @brief カウントダウンフェーズの更新
		 */
		void UpdateCountDown();

		/**
		 * @brief プレイ中フェーズの更新
		 */
		void UpdatePlaying();

		/**
		 * @brief フィニッシュフェーズの更新
		 */
		void UpdateFinishing();

		/**
		 * @brief カウントダウンフェーズの描画
		 * @param rc レンダーコンテキスト
		 */
		void RenderCountDown(RenderContext& rc);

		/**
		 * @brief プレイ中フェーズの描画
		 * @param rc レンダーコンテキスト
		 */
		void RenderPlaying(RenderContext& rc);

		/**
		 * @brief フィニッシュフェーズの描画
		 * @param rc レンダーコンテキスト
		 */
		void RenderFinishing(RenderContext& rc);

		/**
		 * @brief ポーズ画面の描画
		 * @param rc レンダーコンテキスト
		 */
		void RenderPause(RenderContext& rc);

		/**
		 * @brief サウンドオプション画面の描画
		 * @param rc レンダーコンテキスト
		 */
		void RenderSoundOption(RenderContext& rc);

		/**
		 * @brief チュートリアル画面の描画
		 * @param rc レンダーコンテキスト
		 */
		void RenderTutorial(RenderContext& rc);


		//------------------------------------------------------------
		// 外部から参照が必要なMenuのゲッター
		//------------------------------------------------------------

		/** @brief カウントダウンMenuを取得 */
		inline ui::CountDownMenu* GetCountDownMenu() const { return m_countDownMenu; }
		/** @brief フィニッシュMenuを取得 */
		inline ui::FinishMenu* GetFinishMenu() const { return m_finishMenu; }
		/** @brief ポーズMenuを取得 */
		inline ui::PauseScreenMenu* GetPauseMenu() const { return m_pauseMenu; }
		/** @brief サウンドオプションMenuを取得 */
		inline ui::SoundOptionMenu* GetSoundOptionMenu() const { return m_soundOptionMenu; }
		/** @brief チュートリアルMenuを取得 */
		inline ui::TutorialMenu* GetTutorialMenu() const { return m_tutorialMenu; }
		/** @brief 子ペンギンリアクションシステムを取得 */
		inline ui::CPReactionSystem* GetCPReactionSystem() const { return m_cpReactionSystem; }
		/** @brief WpWarningSystemを取得 */
		inline ui::WpWarningSystem* GetWpWarningSystem() const { return m_wpWarningSystem; }

		/**
		 * @brief エネミー1体分の探索Layoutを生成して登録する
		 * @detail LoadPhase::Enemy でエネミー生成後に呼ぶ
		 * @param enemy 対象エネミーのポインタ
		 */
		void AddSearchLayout(actor::Enemy* enemy);


	private:
		/**
		 * @brief BattleManagerへUI通知functionを登録する
		 * @param daddyPenguin 睡眠クマ探索の基準座標に使用
		 */
		void RegisterObservers(actor::DaddyPenguin* daddyPenguin);


	private:
		//------------------------------------------------------------
		// Layout（所有権を持つ）
		//------------------------------------------------------------
		ui::Layout* m_countDownLayout = nullptr;
		ui::Layout* m_timerLayout = nullptr;
		ui::Layout* m_finishLayout = nullptr;
		ui::Layout* m_remainingChildLayout = nullptr;
		ui::Layout* m_pauseLayout = nullptr;
		ui::Layout* m_soundOptionLayout = nullptr;
		ui::Layout* m_tutorialLayout = nullptr;
		ui::Layout* m_enemySleepingLayout = nullptr;
		ui::Layout* m_pbWakingUpTimerLayout = nullptr;
		ui::Layout* m_iglooPromptLayout = nullptr;
		ui::Layout* m_achievementLayout = nullptr;
		ui::Layout* m_miniMapLayout = nullptr;
		ui::Layout* m_titleEventLayout = nullptr;
		ui::Layout* m_achievementNotificationLayout = nullptr;
		ui::Layout* m_inGameButtonLayout = nullptr;
		std::vector<ui::Layout*> m_searchLayouts;


		ui::CPReactionSystem* m_cpReactionSystem = nullptr;
		ui::WpWarningSystem* m_wpWarningSystem = nullptr;


		//------------------------------------------------------------
		// Menu（Layoutから取得したポインタ。所有権はLayoutが持つ）
		//------------------------------------------------------------
		ui::CountDownMenu* m_countDownMenu = nullptr;
		ui::InGameTimerMenu* m_timerMenu = nullptr;
		ui::FinishMenu* m_finishMenu = nullptr;
		ui::RemainingChildMenu* m_remainingChildMenu = nullptr;
		ui::PauseScreenMenu* m_pauseMenu = nullptr;
		ui::SoundOptionMenu* m_soundOptionMenu = nullptr;
		ui::TutorialMenu* m_tutorialMenu = nullptr;
		ui::EnemySleepingMenu* m_enemySleepingMenu = nullptr;
		ui::PBWakingUpTimerMenu* m_pbWakingUpTimerMenu = nullptr;
		ui::IglooPromptMenu* m_iglooPromptMenu = nullptr;
		ui::InGameAchievementMenu* m_achievementMenu = nullptr;
		ui::MiniMapMenu* m_miniMapMenu = nullptr;
		ui::TitleEventMenu* m_titleEventMenu = nullptr;
		ui::AchievementNotificationMenu* m_achievementNotificationMenu = nullptr;
		ui::InGameButtonMenu* m_inGameButtonMenu = nullptr;
		std::vector<ui::SearchMenu*> m_searchMenus;


		//============================================//
		// シングルトン関連
		//============================================//
	public:
		/** @brief インスタンスを生成する */
		static void CreateInstance()
		{
			if (m_instance) return;
			m_instance = new InGameUIManager();
		}


		/** @brief インスタンスを取得する */
		static InGameUIManager* GetInstance()
		{
			return m_instance;
		}


		/** @brief インスタンスを破棄する */
		static void DestroyInstance()
		{
			delete m_instance;
			m_instance = nullptr;
		}


	private:
		static InGameUIManager* m_instance;
	};
}