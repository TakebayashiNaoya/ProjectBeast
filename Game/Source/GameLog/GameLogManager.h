/**
 * @file GameLogManager.h
 * @brief ゲームプレイログの記録・出力管理クラス
 *
 * @details
 * ゲーム中に発生するイベントと毎フレームティックを JSONL 形式で記録し、
 * ゲーム終了時にファイルへ書き出す。
 *
 * 出力ファイル構成（Logs/<session_id>/）:
 *   session.json     … セッション情報
 *   ticks.jsonl.cmp  … 毎フレームスナップショット (type="tick" + 埋め込みイベント) を
 *                       JSONL化した上でLogCompressionにより圧縮したもの。
 *                       非圧縮のticks.jsonlのまま残っている旧セッションも
 *                       ReplayScene側で引き続き読める
 *
 * 利用側が呼ぶメソッド:
 *   RecordTick()          … 毎フレーム InGameSceneBase から呼ぶ
 *   QueueEvent(json)      … イベント発生時に呼ぶ（次 tick に埋め込まれる）
 *   RecordSpawn(...)      … エンティティ生成時
 *   RecordDespawn(...)    … エンティティ消滅時
 *   SetStageConfig(...)   … ロード完了時に一度だけ。ステージの配合などを session.json へ残す
 *   SetResultDetail(...)  … ゲーム終了時、Flush より前。アチーブメントの結果などを残す
 *   Flush(stage, ...)     … ゲーム終了時に書き出し
 */
#pragma once
#include "Json/json.hpp"
#include <vector>
#include <string>


namespace app
{
	namespace actor
	{
		class DaddyPenguin;
	}


	/**
	 * @brief ゲームプレイログ管理クラス（シングルトン）
	 */
	class GameLogManager
	{
	public:
		/** @brief 毎フレーム InGameSceneBase から呼ぶ。全エンティティ・カメラのスナップショットを記録 */
		void RecordTick(actor::DaddyPenguin* daddy);

		/**
		 * @brief イベントを次の tick に埋め込む
		 * @details 呼び出し側は json オブジェクトに必要なフィールドをセットして渡す。
		 *          "t" フィールドはここで付与するため不要。
		 */
		void QueueEvent(nlohmann::json event);

		/**
		 * @brief エンティティのスポーンを記録（game 開始直後、またはゲーム中に生成される場合）
		 * @param entity   "bear" / "penguin" / "whirlpool"
		 * @param id       エンティティの連番 ID
		 * @param extraData タイプなど追加情報（省略可）
		 * @note extraData の "type" / "t" / "entity" / "id" はレコード共通の項目で上書きされる。
		 *       ペンギンの種別のような独自の情報は別のキー名で渡すこと
		 */
		void RecordSpawn(const std::string& entity, int id,
			nlohmann::json extraData = nlohmann::json::object());

		/**
		 * @brief エンティティのデスポーンを記録
		 * @param entity "bear" / "penguin" / "whirlpool"
		 * @param id     エンティティの連番 ID
		 * @param cause  "bear_kill" / "whirlpool" / "" など
		 */
		void RecordDespawn(const std::string& entity, int id,
			const std::string& cause = "");

		/**
		 * @brief ステージの生成設定を記録する
		 * @param config 子ペンギンの配合・制限時間・敵数などステージ固有のパラメータ
		 * @details ロード完了時に一度だけ呼ぶ。session.json の "stage_config" へそのまま出力される。
		 *          どの配合で取ったログなのかをログ自身に残し、配合違いの比較を可能にするために使う
		 */
		void SetStageConfig(nlohmann::json config);

		/**
		 * @brief リザルトの詳細を記録する
		 * @param detail アチーブメントの達成状況や各種カウンタ
		 * @details Flush() より前に呼ぶ。session.json の "result" へ統合して出力される
		 */
		void SetResultDetail(nlohmann::json detail);

		/**
		 * @brief ゲーム終了時にファイルへ書き出す
		 * @param stage        ステージ名 ("Tutorial" / "Normal" / "Easy" / "Hard")
		 * @param durationSec  プレイ時間（秒）
		 * @param rescued      救出ペンギン数
		 * @param score        スコア
		 */
		void Flush(const std::string& stage,
			float durationSec,
			int   rescued,
			float score);


	private:
		/** ゲーム内時間を取得するヘルパー */
		float GetGameTime() const;

		/** すべてのエンティティのスナップショット JSON を生成 */
		nlohmann::json BuildTickSnapshot(actor::DaddyPenguin* daddy) const;

		/** Quaternion から Yaw 角度（度）を取得 */
		static float QuatToYawDeg(float qx, float qy, float qz, float qw);

		/** セッション ID（ファイル名プレフィックス）を生成 */
		static std::string MakeSessionId();


	private:
		/** 蓄積した tick レコード（MAX_TICKS 件まで記録し以降は無視） */
		std::vector<nlohmann::json> m_ticks;

		/** 次の tick に埋め込む予定のイベントリスト */
		std::vector<nlohmann::json> m_pendingEvents;

		/** ステージの生成設定（session.json の "stage_config" へ出力する） */
		nlohmann::json m_stageConfig;

		/** リザルトの詳細（session.json の "result" へ統合して出力する） */
		nlohmann::json m_resultDetail;

		/** フレーム番号 */
		int m_frameCount = 0;

		/** セッション開始時刻文字列（Flush で使用） */
		std::string m_sessionId;

		/** tick の最大記録件数（60fps 換算で約10分。1プレイのステージ最長時間より十分大きい。超過分は記録しない） */
		static constexpr size_t MAX_TICKS = 36000;


		//============================================//
		// シングルトン関連
		//============================================//
	public:
		static void CreateInstance()
		{
			if (m_instance == nullptr)
				m_instance = new GameLogManager();
		}

		static GameLogManager* GetInstance() { return m_instance; }

		static void DestroyInstance()
		{
			delete m_instance;
			m_instance = nullptr;
		}

	private:
		GameLogManager();
		~GameLogManager() = default;

		static GameLogManager* m_instance;
	};
}
