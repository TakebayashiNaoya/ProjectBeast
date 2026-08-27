/**
 * @file DecalProfiler.cpp
 * @brief デカールの処理負荷を実測するための計測クラス
 */
#include "stdafx.h"
#include "DecalProfiler.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <numeric>
#include <sstream>


namespace {
	/** @brief 環境変数を文字列で取得する。未設定なら空文字 */
	std::string GetEnv(const char* name) {
		char*  value = nullptr;
		size_t length = 0;
		if (_dupenv_s(&value, &length, name) != 0 || value == nullptr) return std::string();
		std::string result(value);
		free(value);
		return result;
	}

	/** @brief 環境変数を整数で取得する。未設定なら既定値 */
	int GetEnvInt(const char* name, int defaultValue) {
		const std::string s = GetEnv(name);
		if (s.empty()) return defaultValue;
		return std::atoi(s.c_str());
	}

	/** @brief DecalKind に対応する表示名。DecalKind の宣言順と揃えること */
	const char* KIND_NAMES[app::effect::DECAL_KIND_NUM] = { "Snow", "Grass", "Rock", "Bear" };

	/** @brief ソート済みの配列からパーセンタイル値を取り出す */
	double Percentile(const std::vector<double>& sorted, double ratio) {
		if (sorted.empty()) return 0.0;
		const size_t index = static_cast<size_t>(ratio * (sorted.size() - 1));
		return sorted[index];
	}
}


namespace app {
	namespace effect {
		double DecalProfiler::NowMs() {
			static LARGE_INTEGER frequency = [] {
				LARGE_INTEGER f;
				QueryPerformanceFrequency(&f);
				return f;
			}();
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			return static_cast<double>(counter.QuadPart) * 1000.0 / static_cast<double>(frequency.QuadPart);
		}


		DecalProfiler::DecalProfiler() {
			m_isEnabled = (GetEnvInt("DECAL_PERF", 0) != 0);
			if (!m_isEnabled) return;

			m_tag = GetEnv("DECAL_PERF_TAG");
			if (m_tag.empty()) m_tag = "run";
			m_quitAfterFrames = GetEnvInt("DECAL_PERF_QUIT", 0);

			// 実行ごとに別ファイルへ出す。上書きで前の計測を失わないようにする
			std::time_t  now = std::time(nullptr);
			std::tm      tm{};
			localtime_s(&tm, &now);
			std::ostringstream oss;
			oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
			m_sessionId = oss.str();

			m_outDir = "Logs/decal_perf";

			m_wallFrameMs.reserve(60 * 60 * 10);
			m_initMs.reserve(4096);
		}


		DecalProfiler::~DecalProfiler() {
			if (!m_isEnabled) return;
			Flush();
			WriteSummary();
			if (m_csv.is_open()) m_csv.close();
		}


		void DecalProfiler::OpenOutput() {
			if (m_isOutputOpened) return;
			m_isOutputOpened = true;

			std::error_code ec;
			std::filesystem::create_directories(m_outDir, ec);

			const std::string path = m_outDir + "/" + m_sessionId + "_" + m_tag + ".csv";
			m_csv.open(path, std::ios::out | std::ios::trunc);
			if (!m_csv.is_open()) return;

			m_csv << "frame,phase,phaseName,wallFrameMs,gameDeltaMs,"
				<< "spawnCalls,spawnNoGround,spawnNoSlot,spawnAccepted,evictions,"
				<< "modelInits,modelFirstInits,modelReuses,"
				<< "spawnMs,raycastMs,modelInitMs,updateMs,renderMs,"
				<< "activeDecals,poolSize,drawCalls\n";
		}


		void DecalProfiler::RecordSpawnAccepted(double elapsedMs, double raycastMs) {
			if (!m_isEnabled) return;
			m_frame.spawnAccepted++;
			m_frame.spawnMs += elapsedMs;
			m_frame.raycastMs += raycastMs;
		}


		void DecalProfiler::RecordModelInit(double elapsedMs, bool isFirstInit, DecalKind fromKind, DecalKind toKind) {
			if (!m_isEnabled) return;
			m_frame.modelInits++;
			m_frame.modelInitMs += elapsedMs;
			if (isFirstInit) {
				m_frame.modelFirstInits++;
			}
			else {
				// 初回生成は「種類の切り替え」ではないので遷移表には数えない
				const int from = static_cast<int>(fromKind);
				const int to = static_cast<int>(toKind);
				if (from >= 0 && from < DECAL_KIND_NUM && to >= 0 && to < DECAL_KIND_NUM) {
					m_transition[from][to]++;
				}
			}
			m_initMs.push_back(elapsedMs);
		}


		void DecalProfiler::SetPhase(int phase, const char* phaseName) {
			if (!m_isEnabled) return;
			m_phase = phase;
			m_phaseName = phaseName ? phaseName : "";
		}


		void DecalProfiler::EndFrame(double elapsedMs, int drawCalls, int activeDecals, int poolSize) {
			if (!m_isEnabled) return;
			OpenOutput();

			m_frame.renderMs += elapsedMs;

			const double now = NowMs();
			const double wallFrameMs = (m_lastFrameEndMs < 0.0) ? 0.0 : (now - m_lastFrameEndMs);
			m_lastFrameEndMs = now;

			const double gameDeltaMs = (g_gameTime != nullptr)
				? g_gameTime->GetFrameDeltaTime() * 1000.0
				: 0.0;

			// 1フレーム目は前フレームとの差が取れないので集計から外す
			if (wallFrameMs > 0.0) m_wallFrameMs.push_back(wallFrameMs);

			if (m_csv.is_open()) {
				m_csv << m_frameCount << ','
					<< m_phase << ','
					<< m_phaseName << ','
					<< std::fixed << std::setprecision(4)
					<< wallFrameMs << ','
					<< gameDeltaMs << ','
					<< m_frame.spawnCalls << ','
					<< m_frame.spawnNoGround << ','
					<< m_frame.spawnNoSlot << ','
					<< m_frame.spawnAccepted << ','
					<< m_frame.evictions << ','
					<< m_frame.modelInits << ','
					<< m_frame.modelFirstInits << ','
					<< m_frame.modelReuses << ','
					<< m_frame.spawnMs << ','
					<< m_frame.raycastMs << ','
					<< m_frame.modelInitMs << ','
					<< m_frame.updateMs << ','
					<< m_frame.renderMs << ','
					<< activeDecals << ','
					<< poolSize << ','
					<< drawCalls << '\n';
			}

			m_totalSpawnCalls += m_frame.spawnCalls;
			m_totalSpawnNoGround += m_frame.spawnNoGround;
			m_totalSpawnNoSlot += m_frame.spawnNoSlot;
			m_totalSpawnAccepted += m_frame.spawnAccepted;
			m_totalEvictions += m_frame.evictions;
			m_totalModelInits += m_frame.modelInits;
			m_totalModelFirstInits += m_frame.modelFirstInits;
			m_totalModelReuses += m_frame.modelReuses;
			m_totalModelInitMs += m_frame.modelInitMs;
			m_totalSpawnMs += m_frame.spawnMs;
			m_totalRaycastMs += m_frame.raycastMs;
			m_totalUpdateMs += m_frame.updateMs;
			m_totalRenderMs += m_frame.renderMs;

			m_frame.Clear();
			m_frameCount++;

			// 途中で強制終了されても直前までは残るように、1秒に1回書き出す
			if ((m_frameCount % 60) == 0 && m_csv.is_open()) m_csv.flush();

			if (m_quitAfterFrames > 0 && m_frameCount >= m_quitAfterFrames) {
				Flush();
				WriteSummary();
				PostQuitMessage(0);
				// 二重に終了要求を出さないよう、以降は計測を止める
				m_quitAfterFrames = 0;
			}
		}


		void DecalProfiler::Flush() {
			if (!m_isEnabled || !m_csv.is_open()) return;
			m_csv.flush();
		}


		void DecalProfiler::WriteSummary() {
			if (!m_isEnabled || m_frameCount == 0) return;

			std::error_code ec;
			std::filesystem::create_directories(m_outDir, ec);
			const std::string path = m_outDir + "/" + m_sessionId + "_" + m_tag + "_summary.txt";
			std::ofstream out(path, std::ios::out | std::ios::trunc);
			if (!out.is_open()) return;

			std::vector<double> wall = m_wallFrameMs;
			std::sort(wall.begin(), wall.end());
			std::vector<double> init = m_initMs;
			std::sort(init.begin(), init.end());

			const double wallSum = std::accumulate(wall.begin(), wall.end(), 0.0);
			const double wallMean = wall.empty() ? 0.0 : wallSum / wall.size();
			const double initSum = std::accumulate(init.begin(), init.end(), 0.0);
			const double initMean = init.empty() ? 0.0 : initSum / init.size();

			int over16 = 0, over20 = 0, over33 = 0;
			for (double v : wall) {
				if (v > 16.7) over16++;
				if (v > 20.0) over20++;
				if (v > 33.3) over33++;
			}

			out << std::fixed << std::setprecision(3);
			out << "tag              : " << m_tag << "\n";
			out << "session          : " << m_sessionId << "\n";
			out << "frames           : " << m_frameCount << "\n";
			out << "elapsed sec      : " << (wallSum / 1000.0) << "\n";
			out << "\n[frame time (ms)]\n";
			out << "  mean           : " << wallMean << "\n";
			out << "  median         : " << Percentile(wall, 0.50) << "\n";
			out << "  p90            : " << Percentile(wall, 0.90) << "\n";
			out << "  p95            : " << Percentile(wall, 0.95) << "\n";
			out << "  p99            : " << Percentile(wall, 0.99) << "\n";
			out << "  max            : " << (wall.empty() ? 0.0 : wall.back()) << "\n";
			out << "  over 16.7ms    : " << over16 << " (" << (wall.empty() ? 0.0 : 100.0 * over16 / wall.size()) << "%)\n";
			out << "  over 20.0ms    : " << over20 << " (" << (wall.empty() ? 0.0 : 100.0 * over20 / wall.size()) << "%)\n";
			out << "  over 33.3ms    : " << over33 << " (" << (wall.empty() ? 0.0 : 100.0 * over33 / wall.size()) << "%)\n";

			out << "\n[spawn]\n";
			out << "  SpawnFootprint calls : " << m_totalSpawnCalls << " (" << (1000.0 * m_totalSpawnCalls / (wallSum > 0.0 ? wallSum : 1.0)) << " /sec)\n";
			out << "  rejected: no ground  : " << m_totalSpawnNoGround << "\n";
			out << "  rejected: no slot    : " << m_totalSpawnNoSlot << "\n";
			out << "  accepted             : " << m_totalSpawnAccepted << "\n";
			out << "  evicted a live decal : " << m_totalEvictions << "\n";
			out << "  total spawn ms       : " << m_totalSpawnMs << " (" << (100.0 * m_totalSpawnMs / (wallSum > 0.0 ? wallSum : 1.0)) << "% of wall)\n";
			out << "  of which raycast ms  : " << m_totalRaycastMs << "\n";

			out << "\n[ModelRender::InitFromLoaded]\n";
			out << "  init count           : " << m_totalModelInits << "\n";
			out << "    first init         : " << m_totalModelFirstInits << "\n";
			out << "    kind switch        : " << (m_totalModelInits - m_totalModelFirstInits) << "\n";
			out << "  reuse count          : " << m_totalModelReuses << "\n";
			out << "  init rate            : "
				<< (m_totalSpawnAccepted > 0 ? 100.0 * m_totalModelInits / m_totalSpawnAccepted : 0.0) << "% of accepted spawns\n";
			out << "  total init ms        : " << m_totalModelInitMs << " (" << (100.0 * m_totalModelInitMs / (wallSum > 0.0 ? wallSum : 1.0)) << "% of wall)\n";
			out << "  per init ms mean     : " << initMean << "\n";
			out << "  per init ms median   : " << Percentile(init, 0.50) << "\n";
			out << "  per init ms p95      : " << Percentile(init, 0.95) << "\n";
			out << "  per init ms max      : " << (init.empty() ? 0.0 : init.back()) << "\n";

			out << "\n[DecalManager cost]\n";
			out << "  Update total ms      : " << m_totalUpdateMs << " (" << (100.0 * m_totalUpdateMs / (wallSum > 0.0 ? wallSum : 1.0)) << "% of wall)\n";
			out << "  Render total ms      : " << m_totalRenderMs << " (" << (100.0 * m_totalRenderMs / (wallSum > 0.0 ? wallSum : 1.0)) << "% of wall)\n";

			out << "\n[kind transition matrix] (rows: from, cols: to)\n";
			out << "          ";
			for (int to = 0; to < DECAL_KIND_NUM; ++to) out << std::setw(9) << KIND_NAMES[to];
			out << "\n";
			for (int from = 0; from < DECAL_KIND_NUM; ++from) {
				out << std::setw(9) << KIND_NAMES[from] << " ";
				for (int to = 0; to < DECAL_KIND_NUM; ++to) out << std::setw(9) << m_transition[from][to];
				out << "\n";
			}
			out.close();
		}
	}
}
