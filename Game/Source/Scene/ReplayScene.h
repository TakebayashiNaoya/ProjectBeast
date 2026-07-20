/**
 * @file ReplayScene.h
 * @brief プレイログを再生するリプレイシーン
 * @author 竹林
 */
#pragma once
#include "IScene.h"

#include "Json/json.hpp"
#include <memory>
#include <string>
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

	class ReplayScene : public IScene
	{
		appScene(ReplayScene);


	public:
		ReplayScene();
		~ReplayScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate() override;
		void Render(RenderContext& rc) override;

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

		/** 再生を開始する（読み込み済みログの先頭から） */
		void StartPlayback();

		/** 再生中のtick補間・モデル追従・カメラ追従の更新 */
		void UpdatePlayback(float deltaTime);

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
		size_t AcquireWhirlpoolSlot(std::vector<int>& slotIds, std::vector<bool>& slotActive, int id);


	private:
		/** 選択可能なセッション一覧（新しい順） */
		std::vector<SessionEntry> m_sessions;

		/** 読み込み済みセッションID（未選択なら空） */
		std::string m_loadedSessionId;

		/** 読み込んだ tick データ */
		std::vector<nlohmann::json> m_ticks;

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

		/** 子ペンギンのゴーストActorプール（idごとに1体、使い捨てで増える） */
		std::vector<std::unique_ptr<actor::ChildPenguin>> m_penguinActors;
		/** m_penguinActors[i] が表示しているidの対応表（同じ長さ） */
		std::vector<int> m_penguinSlotIds;
		/** m_penguinActors[i] を今フレーム描画すべきか */
		std::vector<bool> m_penguinSlotActive;
		/** m_penguinActors[i] が直前に再生していた記録上のstate（同じ長さ） */
		std::vector<std::string> m_penguinLastState;

		/** 渦潮のゴーストプール（idごとに1体、使い捨てで増える） */
		std::vector<std::unique_ptr<nature::Whirlpool>> m_whirlpoolModels;
		/** m_whirlpoolModels[i] が表示しているidの対応表（同じ長さ） */
		std::vector<int> m_whirlpoolSlotIds;
		/** m_whirlpoolModels[i] を今フレーム描画すべきか */
		std::vector<bool> m_whirlpoolSlotActive;

		/** リプレイ再生用のカメラコントローラー */
		std::shared_ptr<camera::ReplayCamera> m_replayCamera;


	private:
		/** 空（NewGO/DeleteGOで管理するゲームオブジェクト） */
		SkyCube* m_skyCube = nullptr;

		/** 背景（ステージ地形・海・空）を読み込み済みか */
		bool m_backgroundLoaded = false;

		/** 現在読み込んでいる背景のステージ名（別ステージのログに切り替わったら読み直す） */
		std::string m_loadedStageName;
	};
}
