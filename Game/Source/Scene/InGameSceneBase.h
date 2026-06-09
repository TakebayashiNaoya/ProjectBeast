/**
 * @file InGameSceneBase.h
 * @brief インゲームシーン基底クラス
 * @author 立山、竹林
 */
#pragma once
#include "IScene.h"
#include "Source/Camera/CameraSteering.h"
#include "Source/UI/Menus/CountDownMenu.h"


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


	/** 子ペンギン生成設定 */
	struct PenguinSpawnConfig
	{
		int serious = 0;
		int clingy = 0;
		int naughty = 0;
		int clumsy = 0;
		int caring = 0;
		float spawnRadius = 3000.0f;
	};


	/**
	 * @brief インゲームシーン基底クラス
	 * @detail 各ステージクラスはこのクラスを継承し、
	 *         純粋仮想関数でステージ固有のパラメータを返す。
	 *         共通のロード・ゲームフェーズ処理はこのクラスが担う。
	 */
	class InGameSceneBase : public IScene
	{
	public:
		InGameSceneBase();
		~InGameSceneBase() override;

		bool Start() override;
		void Update() override;
		void PauseUpdate() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;

		bool IsLoaded() const override { return m_loadPhase == LoadPhase::Done; }


	protected:
		//------------------------------------------------------------
		// ステージ固有パラメータ（派生クラスが実装する）
		//------------------------------------------------------------
		/** 制限時間（秒） */
		virtual float GetTimeLimit() const = 0;
		/** 子ペンギン生成設定 */
		virtual PenguinSpawnConfig GetPenguinConfig() const = 0;
		/** ステージの配置JSONパス */
		virtual const char* GetStageJsonPath() const = 0;
		/** 敵の配置JSONパス */
		virtual const char* GetEnemyJsonPath() const = 0;
		/** 渦潮の配置JSONパス */
		virtual const char* GetWhirlpoolPositionsJsonPath() const = 0;
		/** 渦潮のパラメーターJSONパス */
		virtual const char* GetWhirlpoolParameterJsonPath() const = 0;
		/** 海のパラメーターJSONパス */
		virtual const char* GetOceanParameterJsonPath() const = 0;

		//------------------------------------------------------------
		// フック（必要なステージだけオーバーライドする）
		//------------------------------------------------------------
		/** ロード完了時の追加処理 */
		virtual void OnLoadComplete() {}
		/** Playing フェーズの追加更新（TutorialController などを想定） */
		virtual void OnUpdatePlaying() {}
		/** Playing フェーズの追加描画（矢印UIなどを想定） */
		virtual void OnRenderPlaying(RenderContext& /*rc*/) {}

		/** 派生クラスからプレイヤー参照が必要になるケースに備えて protected */
		actor::DaddyPenguin* m_daddyPenguin = nullptr;


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

		void UpdateGamePhase();

		//------------------------------------------------------------
		// アクター
		//------------------------------------------------------------
		camera::CameraSteering m_cameraSteering;

		SkyCube* m_skyCube = nullptr;


		//------------------------------------------------------------
		// ポーズ
		//------------------------------------------------------------
		enum class PauseState { Pause, SoundOption, Tutorial };
		PauseState m_pauseState = PauseState::Pause;

		bool m_isPauseEntered = false;

		//------------------------------------------------------------
		// シーン遷移
		//------------------------------------------------------------
		bool m_nextScene = false;
		bool m_goTitle = false;

		/** 前フレームのカウントダウンタイプを保持（初期値は None） */
		ui::EnCountDownType m_lastCountType = ui::EnCountDownType::None;

		/** ホイッスルを鳴らしたかどうか（Finishing フェーズで1回だけ鳴らすためのフラグ） */
		bool m_isWhistlePlayed = false;
	};
}
