/**
 * @file TutorialController.h
 * @brief チュートリアルステージのトリガー・ウィンドウ管理
 * @author 竹林
 */
#pragma once
#include "Source/UI/Layout.h"
#include "Source/UI/Menus/TutorialWindowMenu.h"
#include "Source/UI/Modules/System/SystemPacket.h"
#include <queue>


namespace app
{
	namespace actor { class DaddyPenguin; }


	/**
	 * @brief チュートリアルのトリガー種別
	 * @details 値は配列インデックスとして直接使用するため変更しないこと
	 */
	enum class EnTutorialTarget : uint8_t
	{
		PenguinSerious = 0,   // まじめ
		PenguinClingy,        // 甘えん坊
		PenguinNaughty,       // やんちゃ
		PenguinClumsy,        // おっちょこちょい
		PenguinCaring,        // 世話焼き
		Bear,                 // シロクマ
		Igloo,                // イグルー（かまくら）
		Whirlpool,            // 渦潮
		Max
	};


	/**
	 * @brief チュートリアルステージのコントローラー
	 *
	 * ■ 役割
	 *   1. 近接トリガー検知：プレイヤーが各ターゲット種別に一定距離まで近づいたら
	 *      未表示チュートリアルをキューに積む（複数同時→順番に表示）
	 *   2. ウィンドウ管理：キューから順に TutorialWindowMenu を開閉し、
	 *      一定時間（TUTORIAL_WINDOW_DISPLAY_TIME）表示した後、自動的に閉じる。
	 *      ゲームはポーズせず、通常の Update() 内で進行する。
	 *
	 * ■ 呼び出し側（TutorialInGameScene）への要件
	 *   - OnLoadComplete()    → Initialize(daddyPenguin)
	 *   - OnUpdatePlaying()   → Update()    ← 通常フレーム（ポーズ中は呼ばれない）
	 *   - OnRenderPlaying()   → Render(rc)  ← ウィンドウ描画
	 *   - OnPauseUpdate()     → PauseUpdate() （true を返す間はデフォルトポーズをスキップ）
	 *   - OnPauseRender()     → PauseRender(rc)
	 */
	class TutorialController
	{
	public:
		/** @brief 初期化。OnLoadComplete() から呼ぶ */
		void Initialize(actor::DaddyPenguin* daddy);

		/** @brief 通常フレーム更新（近接検知）。OnUpdatePlaying() から呼ぶ */
		void Update();

		/** @brief ウィンドウ描画。OnRenderPlaying() から呼ぶ */
		void Render(RenderContext& rc);

		/**
		 * @brief ポーズ中の更新（ウィンドウアニメーション・入力）
		 * @return true = チュートリアルウィンドウが処理中（通常ポーズをスキップ）
		 */
		bool PauseUpdate();

		/**
		 * @brief ポーズ中の描画（ウィンドウ）
		 * @return true = チュートリアルウィンドウが描画中（通常ポーズをスキップ）
		 */
		bool PauseRender(RenderContext& rc);


	private:
		/**
		 * @brief ターゲット種別の最近傍インスタンス座標を取得
		 * @return 座標が取得できたら true（Ocean は常に false）
		 */
		bool GetNearestTargetPosition(EnTutorialTarget type, Vector3& outPos) const;

		/** キューの先頭ウィンドウを開いてポーズをかける */
		void TryOpenNextWindow();

		/** クローズアニメーション完了を検知したときの完了処理（達成/ログ/次のウィンドウ準備） */
		void FinishCurrentWindow();


	private:
		/** @brief トリガー対象の総数（Max を含む配列サイズ） */
		static constexpr int TARGET_COUNT = static_cast<int>(EnTutorialTarget::Max);

		/**
		 * @brief 近接トリガー対象の一覧（Ocean を除く）
		 * @details この配列を変更した場合、WINDOW_JSON_PATHS も必ず更新すること。
		 *          PROXIMITY_TARGET_COUNT はこの配列から自動計算されるため手動更新不要。
		 */
		static constexpr EnTutorialTarget PROXIMITY_TARGETS[] = {
			EnTutorialTarget::PenguinSerious,
			EnTutorialTarget::PenguinClingy,
			EnTutorialTarget::PenguinNaughty,
			EnTutorialTarget::PenguinClumsy,
			EnTutorialTarget::PenguinCaring,
			EnTutorialTarget::Bear,
			EnTutorialTarget::Igloo,
			EnTutorialTarget::Whirlpool,
		};
		/** PROXIMITY_TARGETS の要素数（自動計算） */
		static constexpr int PROXIMITY_TARGET_COUNT = static_cast<int>(
			sizeof(PROXIMITY_TARGETS) / sizeof(EnTutorialTarget));

		/** ターゲットごとのウィンドウ JSON パス */
		static const char* const WINDOW_JSON_PATHS[TARGET_COUNT];

		/** プレイヤーとの距離がこれ以下でトリガー（ワールド単位） */
		static constexpr float TRIGGER_RADIUS = 200.0f;

		/** ウィンドウを表示し続ける時間（秒）。経過したら自動で閉じる */
		static constexpr float TUTORIAL_WINDOW_DISPLAY_TIME = 4.0f;


	private:
		actor::DaddyPenguin* m_daddy = nullptr;

		/** 近接トリガーが発火済みか（キュー投入済み・表示済みを含む） */
		bool m_triggered[TARGET_COUNT] = {};

		/** 表示待ちターゲットのキュー */
		std::queue<EnTutorialTarget> m_queue;

		/** 現在表示中のウィンドウインデックス（-1 = 無し） */
		int  m_currentTargetIdx = -1;
		bool m_isWindowOpen = false;

		/** 現在表示中ウィンドウの経過時間 */
		float m_windowDisplayTimer = 0.0f;

		/** ターゲットごとのウィンドウ Layout */
		ui::Layout m_windowLayouts[TARGET_COUNT];
	};
}
