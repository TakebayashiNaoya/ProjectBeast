/**
 * @file TutorialController.h
 * @brief チュートリアルステージのトリガー・矢印・ウィンドウ管理
 * @author 竹林
 */
#pragma once
#include <queue>
#include <array>
#include "Source/UI/Layout.h"
#include "Source/UI/Menus/TutorialWindowMenu.h"
#include "Source/UI/DangerArrow/DangerArrowMenu.h"
#include "Source/UI/System/SystemPacket.h"


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
		BearNest,             // シロクマの巣
		Igloo,                // イグルー（かまくら）
		Ocean,                // 海（水中接触で発火、矢印なし）
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
	 *      窓が開いている間はゲームをポーズする
	 *   3. 矢印管理：未完了ターゲット（Ocean以外）を指す edge/overhead 矢印を表示し、
	 *      チュートリアル完了後に消す
	 *
	 * ■ 呼び出し側（TutorialInGameScene）への要件
	 *   - OnLoadComplete()    → Initialize(daddyPenguin)
	 *   - OnUpdatePlaying()   → Update()    ← 通常フレーム（ポーズ中は呼ばれない）
	 *   - OnRenderPlaying()   → Render(rc)  ← 矢印描画
	 *   - OnPauseUpdate()     → PauseUpdate() （true を返す間はデフォルトポーズをスキップ）
	 *   - OnPauseRender()     → PauseRender(rc)
	 */
	class TutorialController
	{
	public:
		/** @brief 初期化。OnLoadComplete() から呼ぶ */
		void Initialize(actor::DaddyPenguin* daddy);

		/** @brief 通常フレーム更新（近接検知・矢印更新）。OnUpdatePlaying() から呼ぶ */
		void Update();

		/** @brief 矢印描画。OnRenderPlaying() から呼ぶ */
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
		/** 近接トリガーをチェックしてキューに積む */
		void CheckProximityTriggers();

		/**
		 * @brief ターゲット種別の最近傍インスタンス座標を取得
		 * @return 座標が取得できたら true（Ocean は常に false）
		 */
		bool GetNearestTargetPosition(EnTutorialTarget type, Vector3& outPos) const;

		/** キューの先頭ウィンドウを開いてポーズをかける */
		void TryOpenNextWindow();

		/** 矢印の位置・角度・表示状態を計算して反映する */
		void UpdateArrows();


	private:
		/** 矢印の情報 */
		struct ArrowInfo
		{
			Vector2 screenPos;
			float   angleRad = 0.0f;
			bool    visible = false;
		};

		ArrowInfo CalcEdgeArrow(const Vector2& worldScreenPos) const;
		ArrowInfo CalcOverheadArrow(const Vector2& worldScreenPos) const;


	private:
		static constexpr int TARGET_COUNT = static_cast<int>(EnTutorialTarget::Max);

		/** 矢印を持つターゲット（Ocean を除く 9 種） */
		static constexpr int ARROW_COUNT = TARGET_COUNT - 1;
		static const EnTutorialTarget ARROW_TARGETS[ARROW_COUNT];

		/** ターゲットごとのウィンドウ JSON パス */
		static const char* const WINDOW_JSON_PATHS[TARGET_COUNT];

		/** プレイヤーとの距離がこれ以下でトリガー（ワールド単位） */
		static constexpr float TRIGGER_RADIUS = 200.0f;

		/** 2D矢印の配置円半径（スクリーン座標px） */
		static constexpr float CIRCLE_RADIUS = 300.0f;
		/** 2D矢印の円中心Yオフセット */
		static constexpr float CIRCLE_CENTER_Y = -80.0f;
		/**	頭上矢印の上方向オフセット（スクリーン座標px） */
		static constexpr float OVERHEAD_OFFSET_Y = 50.0f;
		/** 上向きDDSをターゲット方向へ向けるZ回転オフセット（ = -π/2） */
		static constexpr float ARROW_ROT_OFFSET = -1.5707963f;
		/** 頭上矢印の回転（下向き = π） */
		static constexpr float OVERHEAD_ANGLE = 3.1415927f;


	private:
		actor::DaddyPenguin* m_daddy = nullptr;

		/** 近接トリガーが発火済みか（キュー投入済み・表示済みを含む） */
		bool m_triggered[TARGET_COUNT] = {};
		/** チュートリアルウィンドウが閉じられたか（矢印を消すフラグ） */
		bool m_completed[TARGET_COUNT] = {};

		/** 表示待ちターゲットのキュー */
		std::queue<EnTutorialTarget> m_queue;

		/** 現在表示中のウィンドウインデックス（-1 = 無し） */
		int  m_currentTargetIdx = -1;
		bool m_isWindowOpen = false;

		/** ターゲットごとのウィンドウ Layout */
		ui::Layout m_windowLayouts[TARGET_COUNT];

		/** 矢印 UI パケット（9 本） */
		std::array<ui::SystemPacket<ui::DangerArrowMenu>, ARROW_COUNT> m_arrowPackets;
	};
}
