/**
 * @file InGameSceneBase.h
 * @brief インゲームシーン基底クラス
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


	/**
	 * @brief 難易度ごとに決まっているステージの静的情報
	 * @details 各インゲームシーンの GetTimeLimit()/GetStageName()/各JSONパスと、
	 *          ステージ選択画面の情報パネルの両方がここを参照する。
	 *          両者に同じ数値を書くと、片方だけ直したときに黙ってズレるため一次資料は1つにする。
	 */
	struct StageInfo
	{
		const char* name;							//ステージ名（ハイスコアの保存キー・ログのステージ名）。
		float       timeLimit;						//制限時間（秒）。
		const char* enemyLayoutJsonPath;			//シロクマ配置JSONのパス。
		const char* whirlpoolPositionsJsonPath;		//渦潮配置JSONのパス。
	};


	/** 難易度3種のステージ情報。ui::EnStageChoices の並び（Easy/Normal/Hard）と一致させること */
	constexpr StageInfo STAGE_INFO_TABLE[] = {
		{
			"Easy", 120.0f,
			"Assets/parameter/character/enemy/EnemyLayout_Easy.json",
			"Assets/parameter/stage/whirlpoolPositions_Easy.json"
		},
		{
			"Normal", 150.0f,
			"Assets/parameter/character/enemy/EnemyLayout_Normal.json",
			"Assets/parameter/stage/whirlpoolPositions_Normal.json"
		},
		{
			"Hard", 180.0f,
			"Assets/parameter/character/enemy/EnemyLayout_Hard.json",
			"Assets/parameter/stage/whirlpoolPositions_Hard.json"
		},
	};

	/** ステージ情報の件数（難易度の数） */
	constexpr int STAGE_INFO_COUNT = static_cast<int>(std::size(STAGE_INFO_TABLE));


	/** 子ペンギン生成設定 */
	struct PenguinSpawnConfig
	{
		int serious = 0;  // まじめ
		int clingy = 0;   // 甘えん坊
		int naughty = 0;  // やんちゃ
		int clumsy = 0;   // おっちょこちょい
		int caring = 0;   // 世話焼き
		float spawnRadius = 3000.0f; // 生成半径
		float groundRayStartY = 3000.0f; // 地面の高さを調べるレイの発射高度（このステージの地形最大高さを安全に超える値）
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
		/** プレイヤーのスポーン座標 */
		virtual Vector3 GetDaddySpawnPos() const = 0;
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
		/** 渦潮のパラメーターバイナリパス */
		virtual const char* GetWhirlpoolParameterBinaryPath() const = 0;
		/** フィーバータイムのパラメーターJSONパス */
		virtual const char* GetFeverParameterJsonPath() const = 0;
		/** 海のパラメーターJSONパス */
		virtual const char* GetOceanParameterJsonPath() const = 0;
		/** 海のパラメーターバイナリパス */
		virtual const char* GetOceanParameterBinaryPath() const = 0;
		/** ログ用ステージ名 ("Tutorial" / "Normal" / "Easy" / "Hard") */
		virtual const char* GetStageName() const = 0;
		/** ステージ別アチーブメント定義JSONのパス */
		virtual const char* GetAchievementJsonPath() const = 0;
		/** ハイトマップ地形のJSONパス（不要なステージはnullptrを返す） */
		virtual const char* GetTerrainJsonPath() const { return nullptr; }

		//------------------------------------------------------------
		// フック（必要なステージだけオーバーライドする）
		//------------------------------------------------------------
		/** ロード完了時の追加処理 */
		virtual void OnLoadComplete() {}
		/** Playing フェーズの追加更新（TutorialController などを想定） */
		virtual void OnUpdatePlaying() {}
		/** Playing フェーズの追加描画（矢印UIなどを想定） */
		virtual void OnRenderPlaying(RenderContext& /*rc*/) {}
		/**
		 * @brief ポーズ中の追加更新
		 * @return true を返すと通常ポーズ画面の更新をスキップする
		 * @detail チュートリアルウィンドウなど独自ポーズを持つ派生クラス向け
		 */
		virtual bool OnPauseUpdate() { return false; }
		/**
		 * @brief ポーズ中の追加描画
		 * @return true を返すと通常ポーズ画面の描画をスキップする
		 */
		virtual bool OnPauseRender(RenderContext& /*rc*/) { return false; }

		/** 派生クラスからプレイヤー参照が必要になるケースに備えて protected */
		actor::DaddyPenguin* m_daddyPenguin = nullptr;


	private:
		/**
		 * @brief 衝撃演出の受け口を登録する
		 * @details 咆哮・かまくら破壊・弾き返し・ウルト発動を、画面揺れ・ラジアルブラー・
		 *          ヒットストップへ振り分ける。通知元（AIやウルト）がカメラや
		 *          ポストエフェクトを直接触らずに済むよう、見せ方はここへ集約する
		 */
		void RegisterImpactObserver();

		//------------------------------------------------------------
		// ロードフェーズ
		//------------------------------------------------------------
		enum class LoadPhase
		{
			None, Stage, StageWait, DecalPrewarm, Daddy, Children, Enemy, Camera, Ocean, MapIcon, Done
		};
		LoadPhase m_loadPhase = LoadPhase::None;
		int m_childIndex = 0;

		//------------------------------------------------------------
		// ステージ紹介動画の撮影モード（BEAST_SHOWCASE）
		//------------------------------------------------------------
		/** 撮影モードが開始済みか（合図ファイルの書き出しとUI無効化を一度だけ行う） */
		bool m_isShowcaseStarted = false;
		/** 撮影モードの経過時間（秒）。一定時間で自動終了する */
		float m_showcaseTimer = 0.0f;

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
