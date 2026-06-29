/**
 * @file InGameUIManager.h
 * @brief インゲームUIの生成・更新・描画・配線を管理するクラス
 * @author 竹林
 */
#pragma once
#include "Source/UI/System/SystemPacket.h"


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
		class DebufMenu;

		class CPReactionSystem;
		class WpWarningSystem;
		class DangerArrowSystem;
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


		void PlayingSetting();

		/**
		 * @brief プレイ中フェーズの更新
		 */
		void UpdatePlaying();

		/**
		 * @brief フィニッシュフェーズの更新
		 */
		void UpdateFinishing();

		/**
		 * @brief ポーズ中のアチーブメントホットリロード反映
		 * @detail ポーズ中は UpdatePlaying() が止まるため、achievementPacket だけ別途更新する
		 */
		void UpdateAchievementHotReload();

		/**
		 * @brief ポーズフェーズの更新（ホットリロード対応）
		 * @detail Layout::Update() を経由することでJSONのホットリロードを有効にする
		 */
		void UpdatePause();

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
		//inline ui::CountDownMenu* GetCountDownMenu() const { return m_countDownMenu; }
		inline ui::CountDownMenu* GetCountDownMenu() const { return m_countDownPacket->GetMenu(); }
		/** @brief フィニッシュMenuを取得 */
		//inline ui::FinishMenu* GetFinishMenu() const { return m_finishMenu; }
		inline ui::FinishMenu* GetFinishMenu() const { return m_finishPacket->GetMenu(); }
		/** @brief ポーズMenuを取得 */
		//inline ui::PauseScreenMenu* GetPauseMenu() const { return m_pauseMenu; }
		inline ui::PauseScreenMenu* GetPauseMenu() const { return m_pausePacket->GetMenu(); }
		/** @brief サウンドオプションMenuを取得 */
		//inline ui::SoundOptionMenu* GetSoundOptionMenu() const { return m_soundOptionMenu; }
		inline ui::SoundOptionMenu* GetSoundOptionMenu() const { return m_soundOptionPacket->GetMenu(); }
		/** @brief チュートリアルMenuを取得 */
		//inline ui::TutorialMenu* GetTutorialMenu() const { return m_tutorialMenu; }
		inline ui::TutorialMenu* GetTutorialMenu() const { return m_tutorialPacket->GetMenu(); }
		/** @brief 子ペンギンリアクションシステムを取得 */
		inline ui::CPReactionSystem* GetCPReactionSystem() const { return m_cpReactionSystem.get(); }
		/** @brief WpWarningSystemを取得 */
		inline ui::WpWarningSystem* GetWpWarningSystem() const { return m_wpWarningSystem.get(); }
		/** @brief 救助数Menuを取得 */
		inline ui::RemainingChildMenu* GetRemainingChildMenu() const { return m_remainingChildPacket->GetMenu(); }
		/** @brief デバフMenuを取得 */
		inline ui::DebufMenu* GetDebufMenu() const { return m_debufPacket->GetMenu(); }
		/** @brief イグループプロンプトMenuを取得 */
		inline ui::IglooPromptMenu* GetIglooPromptMenu() const { return m_iglooPromptPacket->GetMenu(); }



		void SetMiniMapIconNum(ui::EnMiniMapIconType type, uint8_t num);
		void InitializeMapIcon();

		/**
		 * @brief エネミー1体分の探索Layoutを生成して登録する
		 * @detail LoadPhase::Enemy でエネミー生成後に呼ぶ
		 * @param enemy 対象エネミーのポインタ
		 */
		void AddSearchLayout(actor::Enemy* enemy);

		/**
		 * @brief BattleManagerへUI通知functionを登録する
		 * @param daddyPenguin 親ペンギンのポインタ
		 */
		void RegisterObservers(actor::DaddyPenguin* daddyPenguin);


	private:
		//------------------------------------------------------------
		// Packet（所有権を持つ）
		//------------------------------------------------------------
		ui::UIPacket<ui::CountDownMenu> m_countDownPacket;
		ui::UIPacket<ui::InGameTimerMenu> m_timerPacket;
		ui::UIPacket<ui::FinishMenu> m_finishPacket;
		ui::UIPacket<ui::RemainingChildMenu> m_remainingChildPacket;
		ui::UIPacket<ui::PauseScreenMenu> m_pausePacket;
		ui::UIPacket<ui::SoundOptionMenu> m_soundOptionPacket;
		ui::UIPacket<ui::TutorialMenu> m_tutorialPacket;
		ui::UIPacket<ui::EnemySleepingMenu> m_enemySleepingPacket;
		ui::UIPacket<ui::PBWakingUpTimerMenu> m_pbWakingUpTimerPacket;
		ui::UIPacket<ui::IglooPromptMenu> m_iglooPromptPacket;
		ui::UIPacket<ui::InGameAchievementMenu> m_achievementPacket;
		ui::UIPacket<ui::MiniMapMenu> m_miniMapPacket;
		ui::UIPacket<ui::TitleEventMenu> m_titleEventPacket;
		ui::UIPacket<ui::AchievementNotificationMenu> m_achievementNotificationPacket;
		ui::UIPacket<ui::InGameButtonMenu> m_inGameButtonPacket;
		ui::UIPacket<ui::DebufMenu> m_debufPacket;
		std::vector<ui::UIPacket<ui::SearchMenu>> m_searchPackets;

		std::unique_ptr<ui::CPReactionSystem> m_cpReactionSystem;
		std::unique_ptr<ui::WpWarningSystem> m_wpWarningSystem;
		std::unique_ptr<ui::DangerArrowSystem> m_dangerArrowSystem;


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