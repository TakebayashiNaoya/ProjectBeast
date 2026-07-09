/**
 * @file ReplayScene.h
 * @brief プレイログを再生するリプレイシーン
 * @author 竹林
 */
#pragma once
#include "IScene.h"

#include "Json/json.hpp"
#include <string>
#include <vector>


namespace app
{
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
		 * @brief セッション選択UIを描画する
		 * @details DebugWindow はデバッグビルドでしか描画されないため使わず、
		 *          ビルド構成によらず常に見えるよう自前でImGuiウィンドウを描画する
		 */
		void DrawUI();


	private:
		/** 選択可能なセッション一覧（新しい順） */
		std::vector<SessionEntry> m_sessions;

		/** 読み込み済みセッションID（未選択なら空） */
		std::string m_loadedSessionId;

		/** 読み込んだ tick データ */
		std::vector<nlohmann::json> m_ticks;

		/** Bボタンでタイトルへ戻る要求が来ているか */
		bool m_backToTitle = false;
	};
}
