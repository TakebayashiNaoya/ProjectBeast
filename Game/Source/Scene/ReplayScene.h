/**
 * @file ReplayScene.h
 * @brief プレイログを再生するリプレイシーン
 * @author 竹林
 */
#pragma once
#include "IScene.h"
#include "Source/Camera/CameraCommon.h"
#include "Nature/INatureObject.h"

#include "Json/json.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace app
{
	namespace camera
	{
		class ReplayCamera;
	}

	namespace nature
	{
		class Whirlpool;
	}

	namespace actor
	{
		class DaddyPenguin;
		class Enemy;
		class ChildPenguin;
	}

	/**
	 * @brief プレイログを再生するリプレイシーン
	 * @details 渦潮（nature::Whirlpool）はGBuffer・ライティング・フォワードパスの「後」に
	 *          描画される専用タイミング（RenderingEngine::RenderNatureObjects）でしか
	 *          正しく表示されないため、ReplayScene自身が nsBeastEngine::INatureObject を
	 *          実装し、g_renderingEngine に登録することでそのタイミングに乗る。
	 */
	class ReplayScene : public IScene, public nsBeastEngine::INatureObject
	{
		appScene(ReplayScene);


	public:
		ReplayScene();
		~ReplayScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate() override;
		void Render(RenderContext& rc) override;

		/** @brief INatureObject 側の描画（渦潮専用。RenderingEngineから正しいタイミングで呼ばれる） */
		void Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	private:
		/** @brief Logs/ 以下の1セッション分の情報 */
		struct SessionEntry
		{
			std::string id;
			std::string stage;
			float       durationSec = 0.0f;
			int         rescued = 0;
			float       score = 0.0f;
		};


	private:
		/** Logs/ ディレクトリをスキャンしてセッション一覧を作る */
		void ScanSessions();

		/** 指定セッションの ticks.jsonl を読み込む */
		void LoadSession(const std::string& sessionId);

		/**
		 * @brief 背景（ステージ地形・海・空）を読み込む
		 * @details InGameSceneBase の LoadPhase::Stage 〜 Ocean と同じ手順を、
		 *          ゲームプレイ用マネージャー（BattleManager等）を生成せずに行う。
		 *          既に同じステージが読み込み済みなら何もしない。
		 * @param stageName "Tutorial" / "Normal" / "Easy" / "Hard"
		 */
		void LoadBackground(const std::string& stageName);

		/** 背景（ステージ地形・海・空）を破棄する */
		void UnloadBackground();

		/**
		 * @brief セッション選択UIを描画する
		 * @details DebugWindow はデバッグビルドでしか描画されないため使わず、
		 *          ビルド構成によらず常に見えるよう自前でImGuiウィンドウを描画する
		 */
		void DrawUI();

		/**
		 * @brief 任意の再生位置へシークする
		 * @details 二分探索で該当するtickペアを探し直す。早送り・巻き戻し・
		 *          タイムラインバーのドラッグなど、位置が連続的に進むとは限らない
		 *          操作からも安全に呼べる。
		 * @param playbackTime シーク先の再生位置（tick単位。"frame"の値と同じスケール）
		 */
		void SeekToTime(float playbackTime);

		/** @brief 読み込み済みログの最大再生位置（tick単位、最終tickの"frame"の値）を取得する */
		float GetMaxPlaybackTime() const;

		/** @brief 現在の再生位置（m_currentTickIndex・アルファ）をゴースト・カメラ・HUDへ反映する */
		void ApplyCurrentTick();

		/**
		 * @brief カメラ情報が記録されていないログ用のフォールバックカメラ更新
		 * @details m_noCameraDataMode に応じて「親ペンギン追従」か「インスペクター（自由視点）」を行う。
		 *          再生中かどうかに関わらず毎フレーム呼ぶ（一時停止中も自由に見回せるようにするため）。
		 */
		void UpdateFallbackCamera();

		/**
		 * @brief インスペクターモードの自由視点カメラを更新する
		 * @details CameraController.cpp の DebugCamera（APP_DEBUG限定）と同じ操作感を
		 *          ビルド構成によらず使えるようにReplayScene内に複製したもの。
		 *          左スティックで平行移動、右スティックでターゲット中心に回転、
		 *          RB1+左スティックYでFOV調整。
		 */
		void UpdateInspectorCamera();

		/**
		 * @brief 記録された "id" に対応するシロクマの表示スロットを探す。無ければ新規に確保する
		 * @details spawn/despawnで配列の並びが変わっても、同じidは常に同じスロット
		 *          （＝同じActorインスタンス）に対応させることで、
		 *          別々の実体同士の座標を誤って補間してしまうのを防ぐ。
		 *          一度確保したスロットは再利用しない（idは使い捨て）。
		 * @param id 対応付けたい記録上のid
		 * @return 対応するスロットのインデックス
		 */
		size_t AcquireBearSlot(int id);

		/**
		 * @brief 記録された "id" に対応する子ペンギンの表示スロットを探す。無ければ新規に確保する
		 * @param id      対応付けたい記録上のid
		 * @param typeStr 新規スロット確保時に使うタイプ名（"Serious"等）
		 * @return 対応するスロットのインデックス
		 */
		size_t AcquirePenguinSlot(int id, const std::string& typeStr);

		/** @brief 渦潮版の AcquireBearSlot */
		size_t AcquireWhirlpoolSlot(std::vector<int>& slotIds, std::vector<bool>& slotActive,
			std::unordered_map<int, size_t>& slotIndexById, int id);


	private:
		/** 選択可能なセッション一覧（新しい順） */
		std::vector<SessionEntry> m_sessions;

		/** 読み込み済みセッションID（未選択なら空） */
		std::string m_loadedSessionId;

		/**
		 * @brief 直近のLoadSession()で発生した読み込みエラーメッセージ（無ければ空）
		 * @details ticks.jsonl(.cmp)が開けない・展開に失敗した等の場合に設定される。
		 *          0tickで正常に記録された空セッションとは区別する（そちらはエラーにしない）。
		 */
		std::string m_lastLoadError;

		/** 読み込んだ tick データ */
		std::vector<nlohmann::json> m_ticks;

		/**
		 * @brief このログの実際の記録レート（tick/秒）
		 * @details 記録間隔は仕様変更で何度か変わっている（0.1秒間隔 → 毎フレーム等）ため、
		 *          固定値を仮定せず、tickの"t"（TimeManagerの残り時間＝実時間で1秒に1減る）と
		 *          "frame"（tick通し番号）から、ログ読み込み時に自動算出する。
		 */
		float m_effectiveTicksPerSecond = 10.0f;

		/** Bボタンでタイトルへ戻る要求が来ているか */
		bool m_backToTitle = false;


	private:
		/** 再生中かどうか */
		bool m_isPlaying = false;

		/** 再生経過時間（秒。tickの "frame" フィールドから換算） */
		float m_playbackTime = 0.0f;

		/** 現在参照しているtickのインデックス（m_ticks[m_currentTickIndex] <= m_playbackTime） */
		size_t m_currentTickIndex = 0;

		/** 再生速度倍率 */
		float m_playbackSpeed = 1.0f;

		/**
		 * @brief 親ペンギンのゴースト表示用Actor
		 * @details 本物の actor::DaddyPenguin をそのまま使う。
		 *          AI・ステートマシンのUpdate()は一切呼ばず、UpdateModelOnly()で
		 *          座標・姿勢の反映とモデルの非同期ロードだけを行う
		 *          （スキン・アニメーション・見た目を本物と完全に一致させるため）。
		 */
		std::unique_ptr<actor::DaddyPenguin> m_parentActor;
		/** 親ペンギンが直前に再生していた記録上のstate（変化を検知してPlayAnimationし直すため） */
		std::string m_parentLastState;

		/** シロクマのゴーストActorプール（idごとに1体、使い捨てで増える） */
		std::vector<std::unique_ptr<actor::Enemy>> m_bearActors;
		/** m_bearActors[i] が表示しているidの対応表（同じ長さ） */
		std::vector<int> m_bearSlotIds;
		/** m_bearActors[i] を今フレーム描画すべきか */
		std::vector<bool> m_bearSlotActive;
		/** m_bearActors[i] が直前に再生していた記録上のstate（同じ長さ） */
		std::vector<std::string> m_bearLastState;
		/** 記録上のid → m_bearSlotIds上のインデックスの逆引き（AcquireBearSlotをO(1)にするため） */
		std::unordered_map<int, size_t> m_bearSlotIndexById;

		/** 子ペンギンのゴーストActorプール（idごとに1体、使い捨てで増える） */
		std::vector<std::unique_ptr<actor::ChildPenguin>> m_penguinActors;
		/** m_penguinActors[i] が表示しているidの対応表（同じ長さ） */
		std::vector<int> m_penguinSlotIds;
		/** m_penguinActors[i] を今フレーム描画すべきか */
		std::vector<bool> m_penguinSlotActive;
		/** m_penguinActors[i] が直前に再生していた記録上のstate（同じ長さ） */
		std::vector<std::string> m_penguinLastState;
		/** 記録上のid → m_penguinSlotIds上のインデックスの逆引き（AcquirePenguinSlotをO(1)にするため） */
		std::unordered_map<int, size_t> m_penguinSlotIndexById;

		/** 渦潮のゴーストプール（idごとに1体、使い捨てで増える） */
		std::vector<std::unique_ptr<nature::Whirlpool>> m_whirlpoolModels;
		/** m_whirlpoolModels[i] が表示しているidの対応表（同じ長さ） */
		std::vector<int> m_whirlpoolSlotIds;
		/** m_whirlpoolModels[i] を今フレーム描画すべきか */
		std::vector<bool> m_whirlpoolSlotActive;
		/** 記録上のid → m_whirlpoolSlotIds上のインデックスの逆引き（AcquireWhirlpoolSlotをO(1)にするため） */
		std::unordered_map<int, size_t> m_whirlpoolSlotIndexById;

		/**
		 * @brief 渦のUV回転角度（ラジアン、全渦潮共通で進める）
		 * @details 本来は渦潮ごとにUpdate()内で個別に進むが、見た目だけの再現なので
		 *          共通の1つで十分（同じ速度で回るため見分けがつかない）
		 */
		float m_whirlpoolUvRotation = 0.0f;

		/** リプレイ再生用のカメラコントローラー */
		std::shared_ptr<camera::ReplayCamera> m_replayCamera;

		/** @brief カメラ情報が記録されていないログでのカメラの挙動 */
		enum class NoCameraDataMode
		{
			FollowParent, /** 親ペンギンに追従 */
			Inspector,    /** 自由視点（ゲームパッドで手動操作） */
		};

		/** 読み込み中のログにカメラ情報が1件でも記録されているか */
		bool m_hasCameraData = false;

		/** カメラ情報が無いログでの挙動選択（UIから切り替え可能） */
		NoCameraDataMode m_noCameraDataMode = NoCameraDataMode::FollowParent;

		/** インスペクターモードで操作中のカメラ状態（モード切替時に現在のカメラから初期化） */
		camera::CameraData m_inspectorCameraData;

		/**
		 * @brief 親ペンギン追従カメラの、ターゲット（親ペンギン）からの相対オフセット
		 * @details 右スティックでこのオフセットを回転させることで、追従したまま視点を回せる。
		 *          ターゲット自体は毎フレーム親ペンギンの現在座標に追従する。
		 */
		Vector3 m_followCameraOffset = Vector3(0.0f, 120.0f, -220.0f);


	private:
		/** 空（NewGO/DeleteGOで管理するゲームオブジェクト） */
		SkyCube* m_skyCube = nullptr;

		/** 背景（ステージ地形・海・空）を読み込み済みか */
		bool m_backgroundLoaded = false;

		/** 現在読み込んでいる背景のステージ名（別ステージのログに切り替わったら読み直す） */
		std::string m_loadedStageName;
	};
}
