/**
 * @file DecalProfiler.h
 * @brief デカールの処理負荷を実測するための計測クラス
 * @author 竹林
 *
 * @details
 * 「デカールが重い」を思い込みではなく数字で確かめるための計測用。
 * 1フレームにつき1行のCSVを `Logs/decal_perf/` へ書き出す。
 *
 * 計測するもの:
 *   - フレームの実時間（EndFrame から次の EndFrame まで）
 *   - DecalManager の Spawn / Update / Render それぞれのCPU時間
 *   - Decal::Spawn が ModelRender::InitFromLoaded を通った回数と、その所要時間
 *   - 種類の遷移（どの種類からどの種類へ切り替わって作り直しが起きたか）
 *   - アクティブなデカール枚数とドローコール数
 *
 * 環境変数で制御する（ゲーム側にUIを足さずに済ませるため）:
 *   DECAL_PERF=1          … 計測を有効にする
 *   DECAL_PERF_TAG=<名前>  … 出力ファイル名に付ける識別子（before / after など）
 *   DECAL_PERF_QUIT=<N>   … Nフレーム計測したら自動でゲームを終了する（0で無効）
 */
#pragma once
#include "Decal.h"
#include <array>
#include <fstream>
#include <string>
#include <vector>


namespace app {
	namespace effect {
		/**
		 * @brief デカールの負荷計測クラス（シングルトン）
		 * @details 環境変数 DECAL_PERF が無い場合は全メソッドが即座に返るため、
		 *          通常のプレイに影響を与えない
		 */
		class DecalProfiler {
		public:
			static DecalProfiler& Get() { static DecalProfiler instance; return instance; }

			/** @brief 計測が有効かどうか */
			bool IsEnabled() const { return m_isEnabled; }

			/** @brief 高分解能タイマーの現在値をミリ秒で取得する */
			static double NowMs();

			//========================================================
			// 計測の記録
			//========================================================

			/** @brief DecalManager::SpawnFootprint の入口で呼ぶ */
			void RecordSpawnCall() { if (m_isEnabled) m_frame.spawnCalls++; }

			/** @brief 地面へのレイキャストが外れて生成を諦めたときに呼ぶ */
			void RecordSpawnNoGround() { if (m_isEnabled) m_frame.spawnNoGround++; }

			/** @brief 使えるスロットが無くて生成を諦めたときに呼ぶ */
			void RecordSpawnNoSlot() { if (m_isEnabled) m_frame.spawnNoSlot++; }

			/**
			 * @brief まだ生きているデカールを追い出して枠を空けたときに呼ぶ
			 * @details プールの枚数が足りているかどうかの判断材料になる。
			 *          多いようならプールを増やす
			 */
			void RecordEviction() { if (m_isEnabled) m_frame.evictions++; }

			/**
			 * @brief 実際に Decal::Spawn まで到達したときに呼ぶ
			 * @param elapsedMs SpawnFootprint 全体の所要時間（ミリ秒）
			 * @param raycastMs そのうちレイキャストに使った時間（ミリ秒）
			 */
			void RecordSpawnAccepted(double elapsedMs, double raycastMs);

			/**
			 * @brief ModelRender::InitFromLoaded を通ったときに呼ぶ
			 * @param elapsedMs InitFromLoaded の所要時間（ミリ秒）
			 * @param isFirstInit そのスロットで初めての生成なら true（種類切り替えではない）
			 * @param fromKind 切り替え前の種類
			 * @param toKind 切り替え後の種類
			 */
			void RecordModelInit(double elapsedMs, bool isFirstInit, DecalKind fromKind, DecalKind toKind);

			/** @brief 種類が一致して InitFromLoaded を回避できたときに呼ぶ */
			void RecordModelReuse() { if (m_isEnabled) m_frame.modelReuses++; }

			/** @brief DecalManager::Update の所要時間を記録する */
			void RecordUpdate(double elapsedMs) { if (m_isEnabled) m_frame.updateMs += elapsedMs; }

			/**
			 * @brief DecalManager::Render の所要時間と描画枚数を記録し、1フレームを閉じる
			 * @param elapsedMs Render の所要時間（ミリ秒）
			 * @param drawCalls 実際に Draw を呼んだ枚数
			 * @param activeDecals アクティブなデカール枚数
			 * @param poolSize プール全体の枚数
			 */
			void EndFrame(double elapsedMs, int drawCalls, int activeDecals, int poolSize);

			/** @brief 現在のベンチマークのフェーズ番号を設定する（CSVの列に出る） */
			void SetPhase(int phase, const char* phaseName);

			/** @brief 溜まっている行を書き出す */
			void Flush();


		private:
			DecalProfiler();
			~DecalProfiler();

			DecalProfiler(const DecalProfiler&) = delete;
			DecalProfiler& operator=(const DecalProfiler&) = delete;

			/** @brief 出力先を開き、CSVのヘッダー行を書く */
			void OpenOutput();

			/** @brief 集計結果のサマリーをテキストで書き出す */
			void WriteSummary();


			/** @brief 1フレーム分の計測値 */
			struct FrameRecord {
				int    spawnCalls = 0;
				int    spawnNoGround = 0;
				int    spawnNoSlot = 0;
				int    spawnAccepted = 0;
				int    evictions = 0;
				int    modelInits = 0;
				int    modelFirstInits = 0;
				int    modelReuses = 0;
				double spawnMs = 0.0;
				double raycastMs = 0.0;
				double modelInitMs = 0.0;
				double updateMs = 0.0;
				double renderMs = 0.0;

				void Clear() { *this = FrameRecord{}; }
			};

			bool m_isEnabled = false;
			bool m_isOutputOpened = false;

			std::ofstream m_csv;
			std::string   m_sessionId;
			std::string   m_tag;
			std::string   m_outDir;
			std::string   m_phaseName = "idle";

			FrameRecord m_frame;

			int    m_frameCount = 0;
			int    m_phase = 0;
			int    m_quitAfterFrames = 0;
			double m_lastFrameEndMs = -1.0;

			/** フレーム実時間の全履歴（パーセンタイル算出用） */
			std::vector<double> m_wallFrameMs;
			/** InitFromLoaded 1回あたりの所要時間の全履歴 */
			std::vector<double> m_initMs;

			/** 種類の遷移回数。[切り替え前][切り替え後] */
			std::array<std::array<int, DECAL_KIND_NUM>, DECAL_KIND_NUM> m_transition{};

			// セッション全体の累計
			long long m_totalSpawnCalls = 0;
			long long m_totalSpawnNoGround = 0;
			long long m_totalSpawnNoSlot = 0;
			long long m_totalSpawnAccepted = 0;
			long long m_totalEvictions = 0;
			long long m_totalModelInits = 0;
			long long m_totalModelFirstInits = 0;
			long long m_totalModelReuses = 0;
			double    m_totalModelInitMs = 0.0;
			double    m_totalSpawnMs = 0.0;
			double    m_totalRaycastMs = 0.0;
			double    m_totalUpdateMs = 0.0;
			double    m_totalRenderMs = 0.0;
		};
	}
}
