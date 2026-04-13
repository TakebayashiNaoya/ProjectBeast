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
		class SearchMenu;
		class RemainingChildMenu;
		class EnemySleepingMenu;
		class PBWakingUpTimerMenu;
		class IglooPromptMenu;
	}


	/**
	 * @brief インゲームUIの管理クラス
	 * @detail Layoutの生成・所有・更新・描画をまとめ、
	 *         BattleManagerへのUI通知functionの配線を担う。
	 *         InGameSceneはこのクラスを保持するだけでよい。
	 */
	class InGameUIManager
	{
	public:
		InGameUIManager();
		~InGameUIManager();

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
		ui::Layout* m_enemySleepingLayout = nullptr;
		ui::Layout* m_pbWakingUpTimerLayout = nullptr;
		ui::Layout* m_iglooPromptLayout = nullptr;
		std::vector<ui::Layout*> m_searchLayouts;


		//------------------------------------------------------------
		// Menu（Layoutから取得したポインタ。所有権はLayoutが持つ）
		//------------------------------------------------------------
		ui::CountDownMenu* m_countDownMenu = nullptr;
		ui::InGameTimerMenu* m_timerMenu = nullptr;
		ui::FinishMenu* m_finishMenu = nullptr;
		ui::RemainingChildMenu* m_remainingChildMenu = nullptr;
		ui::PauseScreenMenu* m_pauseMenu = nullptr;
		ui::SoundOptionMenu* m_soundOptionMenu = nullptr;
		ui::EnemySleepingMenu* m_enemySleepingMenu = nullptr;
		ui::PBWakingUpTimerMenu* m_pbWakingUpTimerMenu = nullptr;
		ui::IglooPromptMenu* m_iglooPromptMenu = nullptr;
		std::vector<ui::SearchMenu*> m_searchMenus;
	};
}