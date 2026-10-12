/**
 * @file Profiler.cpp
 * @brief CPU処理負荷の計測（簡易階層プロファイラー）の実装
 */
#include "BeastEnginePreCompile.h"
#include "Profiler.h"
#include "imgui.h"
#include <dxgi1_4.h>
#include <psapi.h>
#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>


namespace nsBeastEngine
{
	namespace
	{
		/** ルートノード（フレーム全体）のインデックス */
		constexpr int ROOT_INDEX = 0;
		/** ルートノードの表示名 */
		const char* const ROOT_NAME = u8"フレーム全体";
		/** 平均・最大を集計するフレーム数（数値のメッチヘが揃うよう一定間隔で更新する） */
		constexpr uint32_t INTERVAL_FRAMES = 30;
		/** フレーム時間グラフの履歴数 */
		constexpr int HISTORY_COUNT = 240;
		/** フレーム時間グラフの縦軸の最小上限（60FPSの2フレーム分） */
		constexpr float GRAPH_MIN_SCALE_MS = 1000.0f / 30.0f;
		/** フレーム時間グラフの縦軸の余白係数 */
		constexpr float GRAPH_SCALE_MARGIN = 1.2f;
		/** フレーム時間グラフの高さ */
		constexpr float GRAPH_HEIGHT = 60.0f;
		/** 計測結果の保存先フォルダ（作業ディレクトリからの相対パス） */
		const char* const SAVE_DIRECTORY = "Logs";
		/** この時間を超えたフレームは、内訳を Logs/profiler_spikes.txt に書き出す */
		constexpr double SPIKE_THRESHOLD_MS = 20.0;
		/** スパイクを書き出す最短の間隔（秒） */
		constexpr double SPIKE_MIN_INTERVAL_SEC = 0.5;
		/** 1回の起動で書き出すスパイクの最大数 */
		constexpr int SPIKE_MAX_COUNT = 200;
		/** スパイクの内訳に書く最小の時間（これより短い処理は省く） */
		constexpr double SPIKE_MIN_NODE_MS = 0.3;

		/** フレームに占める割合がこれ以上なら黄色で強調 */
		constexpr double WARNING_RATE = 0.10;
		/** フレームに占める割合がこれ以上なら赤で強調 */
		constexpr double DANGER_RATE = 0.25;
		/** 数値列の幅 */
		constexpr float VALUE_COLUMN_WIDTH = 64.0f;
		/** ウィンドウの初期位置・サイズ */
		constexpr float WINDOW_POS_X = 640.0f;
		constexpr float WINDOW_POS_Y = 20.0f;
		constexpr float WINDOW_WIDTH = 620.0f;
		constexpr float WINDOW_HEIGHT = 640.0f;

		/** 強調色 */
		const ImVec4 WARNING_COLOR = ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
		const ImVec4 DANGER_COLOR = ImVec4(1.0f, 0.35f, 0.3f, 1.0f);


		/**
		 * @brief フレームに占める割合に応じた色で数値を描画する
		 * @param format 書式
		 * @param value  表示する値
		 * @param rate   フレームに占める割合（色分けに使う）
		 */
		void DrawRateColoredValue(const char* format, const double value, const double rate)
		{
			if (rate >= DANGER_RATE) {
				ImGui::TextColored(DANGER_COLOR, format, value);
			} else if (rate >= WARNING_RATE) {
				ImGui::TextColored(WARNING_COLOR, format, value);
			} else {
				ImGui::Text(format, value);
			}
		}
	}


	Profiler& Profiler::Get()
	{
		static Profiler instance;
		return instance;
	}


	Profiler::Profiler()
		: m_frameHistory(HISTORY_COUNT, 0.0f)
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		m_ticksPerSecond = frequency.QuadPart;

		ResetNodes();
	}


	void Profiler::BeginFrame()
	{
		// 最初にフレーム区切りを呼んだスレッドをメインスレッドとする
		if (m_mainThreadId == 0) {
			m_mainThreadId = GetCurrentThreadId();
		}
		if (!IsMainThread()) {
			return;
		}

		const int64_t now = GetTicks();

		// 前フレームのルートを閉じて集計する（初回フレームはまだ閉じていない）
		if (!m_stack.empty()) {
			// ルート以外が残っている = スコープ漏れでフレーム区切りが呼ばれた
			if (m_stack.size() != 1) {
				m_hasMismatch = true;
			}
			Node& root = m_nodes[ROOT_INDEX];
			root.frameTicks += now - root.beginTicks;
			root.frameCalls++;
			AccumulateFrame();
		}

		// 計測途中にノードを消すとスタックが壊れるため、リセットはフレーム区切りで行う
		if (m_isResetRequested) {
			ResetNodes();
			std::fill(m_frameHistory.begin(), m_frameHistory.end(), 0.0f);
			m_historyOffset = 0;
			m_hasMismatch = false;
			m_isResetRequested = false;
		}

		m_stack.clear();
		m_stack.push_back(ROOT_INDEX);
		m_nodes[ROOT_INDEX].beginTicks = now;
	}


	void Profiler::BeginSample(const char* name)
	{
		// フレーム開始前（初期化中）や他スレッドからの計測は無視する
		if (!IsMainThread() || m_stack.empty()) {
			return;
		}

		const int nodeIndex = FindOrAddChild(m_stack.back(), name);
		m_stack.push_back(nodeIndex);
		// 計測自体の負荷を含めないため時刻の取得は最後に行う
		m_nodes[nodeIndex].beginTicks = GetTicks();
	}


	void Profiler::EndSample()
	{
		const int64_t now = GetTicks();
		if (!IsMainThread()) {
			return;
		}
		// ルートはフレーム区切りでのみ閉じる
		if (m_stack.size() <= 1) {
			if (!m_stack.empty()) {
				m_hasMismatch = true;
			}
			return;
		}

		Node& node = m_nodes[m_stack.back()];
		node.frameTicks += now - node.beginTicks;
		node.frameCalls++;
		m_stack.pop_back();
	}


	void Profiler::AddCount(const char* name, const uint32_t count)
	{
		if (!IsMainThread() || m_stack.empty()) {
			return;
		}

		const int nodeIndex = FindOrAddChild(m_stack.back(), name);
		m_nodes[nodeIndex].frameCalls += count;
	}


	void Profiler::AddTime(const char* name, const double ms)
	{
		if (!IsMainThread() || m_stack.empty()) {
			return;
		}

		const int nodeIndex = FindOrAddChild(m_stack.back(), name);
		Node& node = m_nodes[nodeIndex];
		node.frameTicks += static_cast<int64_t>(ms * static_cast<double>(m_ticksPerSecond) / 1000.0);
		node.frameCalls++;
	}


	void Profiler::BeginExternalSample(const char* name, const double ms)
	{
		if (!IsMainThread() || m_stack.empty()) {
			return;
		}

		AddTime(name, ms);
		m_stack.push_back(FindOrAddChild(m_stack.back(), name));
	}


	void Profiler::EndExternalSample()
	{
		// ルートはフレーム区切りでのみ閉じる
		if (!IsMainThread() || m_stack.size() <= 1) {
			return;
		}
		m_stack.pop_back();
	}


	void Profiler::DrawImGui()
	{
		if (!m_isVisible) {
			return;
		}

		ImGui::SetNextWindowPos(ImVec2(WINDOW_POS_X, WINDOW_POS_Y), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(u8"プロファイラー (CPU)", &m_isVisible)) {
			ImGui::End();
			return;
		}

		// ── フレーム概要 ──────────────────────
		const Node& root = m_nodes[ROOT_INDEX];
		const double frameMs = root.avgMs;
		const double fps = (frameMs > 0.0) ? 1000.0 / frameMs : 0.0;
		ImGui::Text(u8"フレーム 平均 %.2f ms (%.1f FPS)  最大 %.2f ms", frameMs, fps, root.maxMs);
		{
			const MemoryPeakTracker& memory = MemoryPeakTracker::Get();
			ImGui::Text(u8"最大メモリ  RAM %.0f MB (確保 %.0f MB)  VRAM %.0f MB (共有 %.0f MB)",
				memory.GetPeakWorkingSetMB(), memory.GetPeakCommitMB(),
				memory.GetPeakVramLocalMB(), memory.GetPeakVramNonLocalMB());
		}

		// ── フレーム時間グラフ ──────────────────────
		{
			const int latestIndex = (m_historyOffset + HISTORY_COUNT - 1) % HISTORY_COUNT;
			const float historyMax = *std::max_element(m_frameHistory.begin(), m_frameHistory.end());
			const float scaleMax = (std::max)(GRAPH_MIN_SCALE_MS, historyMax * GRAPH_SCALE_MARGIN);
			char overlay[32];
			sprintf_s(overlay, "%.2f ms", m_frameHistory[latestIndex]);
			ImGui::PlotLines("##frameHistory", m_frameHistory.data(), HISTORY_COUNT, m_historyOffset,
				overlay, 0.0f, scaleMax, ImVec2(-FLT_MIN, GRAPH_HEIGHT));
		}

		// ── 操作 ──────────────────────
		ImGui::Checkbox(u8"一時停止", &m_isPaused);
		ImGui::SameLine();
		ImGui::Checkbox(u8"重い順に並べる", &m_isSortByTime);
		ImGui::SameLine();
		if (ImGui::Button(u8"リセット")) {
			m_isResetRequested = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"ファイルに保存")) {
			SaveToFile();
		}
		if (!m_lastSaveMessage.empty()) {
			ImGui::TextDisabled("%s", m_lastSaveMessage.c_str());
		}
		ImGui::TextDisabled(u8"%u フレームごとに集計 / 自身ms = 子の計測区間を除いた時間", INTERVAL_FRAMES);

		if (m_hasMismatch) {
			ImGui::TextColored(DANGER_COLOR, u8"計測区間の開始と終了の対応が崩れたフレームがあります");
		}

		// ── 計測結果 ──────────────────────
		const ImGuiTableFlags tableFlags =
			ImGuiTableFlags_BordersV |
			ImGuiTableFlags_BordersOuterH |
			ImGuiTableFlags_Resizable |
			ImGuiTableFlags_RowBg |
			ImGuiTableFlags_ScrollY;
		// 高さ0 = ウィンドウ下端まで使う
		if (ImGui::BeginTable("##profilerTable", 6, tableFlags, ImVec2(0.0f, 0.0f))) {
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn(u8"処理名", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn(u8"平均ms", ImGuiTableColumnFlags_WidthFixed, VALUE_COLUMN_WIDTH);
			ImGui::TableSetupColumn(u8"最大ms", ImGuiTableColumnFlags_WidthFixed, VALUE_COLUMN_WIDTH);
			ImGui::TableSetupColumn(u8"自身ms", ImGuiTableColumnFlags_WidthFixed, VALUE_COLUMN_WIDTH);
			ImGui::TableSetupColumn(u8"割合", ImGuiTableColumnFlags_WidthFixed, VALUE_COLUMN_WIDTH);
			ImGui::TableSetupColumn(u8"回数", ImGuiTableColumnFlags_WidthFixed, VALUE_COLUMN_WIDTH);
			ImGui::TableHeadersRow();

			DrawNode(ROOT_INDEX, frameMs);

			ImGui::EndTable();
		}

		ImGui::End();
	}


	int64_t Profiler::GetTicks() const
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}


	bool Profiler::IsMainThread() const
	{
		return (m_mainThreadId != 0) && (GetCurrentThreadId() == m_mainThreadId);
	}


	void Profiler::ResetNodes()
	{
		m_nodes.clear();
		m_nodes.emplace_back();
		m_nodes[ROOT_INDEX].name = ROOT_NAME;
		m_stack.clear();
		m_intervalFrameCount = 0;
	}


	void Profiler::AccumulateFrame()
	{
		const double msPerTick = 1000.0 / static_cast<double>(m_ticksPerSecond);
		const double rootMs = static_cast<double>(m_nodes[ROOT_INDEX].frameTicks) * msPerTick;

		// 重いフレームは、平均に埋もれる前にそのフレームの内訳を残す
		if (!m_isPaused && rootMs >= SPIKE_THRESHOLD_MS && m_spikeCount < SPIKE_MAX_COUNT) {
			const int64_t now = GetTicks();
			if (m_lastSpikeTicks == 0 ||
				static_cast<double>(now - m_lastSpikeTicks) >= SPIKE_MIN_INTERVAL_SEC * static_cast<double>(m_ticksPerSecond)) {
				m_lastSpikeTicks = now;
				m_spikeCount++;
				WriteSpike(rootMs);
			}
		}

		for (auto& node : m_nodes) {
			// 一時停止中は表示値を固定するので、今フレームの値は捨てる
			if (!m_isPaused) {
				const double ms = static_cast<double>(node.frameTicks) * msPerTick;
				node.sumMs += ms;
				node.peakMs = (std::max)(node.peakMs, ms);
				node.sumCalls += node.frameCalls;
			}
			node.frameTicks = 0;
			node.frameCalls = 0;
		}

		if (m_isPaused) {
			return;
		}

		m_frameHistory[m_historyOffset] = static_cast<float>(rootMs);
		m_historyOffset = (m_historyOffset + 1) % HISTORY_COUNT;

		m_intervalFrameCount++;
		if (m_intervalFrameCount >= INTERVAL_FRAMES) {
			FinalizeInterval();
		}
	}


	void Profiler::FinalizeInterval()
	{
		const double frameCount = static_cast<double>(m_intervalFrameCount);
		for (auto& node : m_nodes) {
			node.avgMs = node.sumMs / frameCount;
			node.maxMs = node.peakMs;
			node.avgCalls = static_cast<float>(node.sumCalls / frameCount);
			node.sumMs = 0.0;
			node.peakMs = 0.0;
			node.sumCalls = 0;
		}

		// 自身の時間 = 自分の時間 - 子の時間の合計
		for (auto& node : m_nodes) {
			double childrenMs = 0.0;
			for (const int childIndex : node.children) {
				childrenMs += m_nodes[childIndex].avgMs;
			}
			node.selfMs = (std::max)(0.0, node.avgMs - childrenMs);
		}

		m_intervalFrameCount = 0;
	}


	int Profiler::FindOrAddChild(const int parentIndex, const char* name)
	{
		for (const int childIndex : m_nodes[parentIndex].children) {
			const char* childName = m_nodes[childIndex].name;
			// 同じリテラルならポインタ比較だけで済む。別翻訳単位の同じ名前リテラルは文字列比較で拾う
			if (childName == name || std::strcmp(childName, name) == 0) {
				return childIndex;
			}
		}

		Node node;
		node.name = name;
		node.parent = parentIndex;
		m_nodes.push_back(node);

		const int nodeIndex = static_cast<int>(m_nodes.size()) - 1;
		m_nodes[parentIndex].children.push_back(nodeIndex);
		return nodeIndex;
	}


	void Profiler::DrawNode(const int nodeIndex, const double frameMs)
	{
		const Node& node = m_nodes[nodeIndex];
		const double totalRate = (frameMs > 0.0) ? node.avgMs / frameMs : 0.0;
		const double selfRate = (frameMs > 0.0) ? node.selfMs / frameMs : 0.0;
		// 直近の集計期間で一度も呼ばれていない処理（別シーンの処理など）は薄く表示する
		const bool isInactive = (node.avgCalls <= 0.0f);

		ImGui::PushID(nodeIndex);
		ImGui::TableNextRow();

		if (isInactive) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		}

		// 処理名
		ImGui::TableNextColumn();
		ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth;
		if (node.children.empty()) {
			treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}
		const bool isOpen = ImGui::TreeNodeEx("##node", treeFlags, "%s", node.name);

		// 平均ms・最大ms
		ImGui::TableNextColumn();
		DrawRateColoredValue("%.3f", node.avgMs, totalRate);
		ImGui::TableNextColumn();
		ImGui::Text("%.3f", node.maxMs);

		// 自身ms はその処理自体の重さが見やすいので自身の割合で色分けする
		ImGui::TableNextColumn();
		DrawRateColoredValue("%.3f", node.selfMs, selfRate);

		// 割合・回数
		ImGui::TableNextColumn();
		DrawRateColoredValue("%5.1f%%", totalRate * 100.0, totalRate);
		ImGui::TableNextColumn();
		ImGui::Text("%.1f", node.avgCalls);

		if (isInactive) {
			ImGui::PopStyleColor();
		}

		if (isOpen && !node.children.empty()) {
			if (m_isSortByTime) {
				// 描画中にm_nodesは変動しないのでインデックスのコピーを並べ替える
				std::vector<int> sortedChildren = node.children;
				std::sort(sortedChildren.begin(), sortedChildren.end(), [this](const int a, const int b)
					{
						return m_nodes[a].avgMs > m_nodes[b].avgMs;
					});
				for (const int childIndex : sortedChildren) {
					DrawNode(childIndex, frameMs);
				}
			} else {
				for (const int childIndex : node.children) {
					DrawNode(childIndex, frameMs);
				}
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}


	void Profiler::WriteSpike(const double frameMs)
	{
		CreateDirectoryA(SAVE_DIRECTORY, nullptr);
		char path[MAX_PATH];
		sprintf_s(path, "%s/profiler_spikes.txt", SAVE_DIRECTORY);
		FILE* fp = nullptr;
		if (fopen_s(&fp, path, "a") != 0 || fp == nullptr) {
			return;
		}

		SYSTEMTIME time;
		GetLocalTime(&time);
		fprintf(fp, u8"==== %04d-%02d-%02d %02d:%02d:%02d.%03d フレーム %.2f ms（%.1f ms 以上の処理のみ）\n",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			frameMs, SPIKE_MIN_NODE_MS);
		WriteSpikeNode(fp, ROOT_INDEX, 0, 1000.0 / static_cast<double>(m_ticksPerSecond));
		fclose(fp);
	}


	void Profiler::WriteSpikeNode(FILE* fp, const int nodeIndex, const int depth, const double msPerTick) const
	{
		const Node& node = m_nodes[nodeIndex];
		const double ms = static_cast<double>(node.frameTicks) * msPerTick;
		if (nodeIndex != ROOT_INDEX && ms < SPIKE_MIN_NODE_MS) {
			return;
		}
		fprintf(fp, "%*s%s\t%.3f ms\t%u\n", depth * 2, "", node.name, ms, node.frameCalls);

		std::vector<int> sortedChildren = node.children;
		std::sort(sortedChildren.begin(), sortedChildren.end(), [this](const int a, const int b)
			{
				return m_nodes[a].frameTicks > m_nodes[b].frameTicks;
			});
		for (const int childIndex : sortedChildren) {
			WriteSpikeNode(fp, childIndex, depth + 1, msPerTick);
		}
	}


	void Profiler::SaveToFile()
	{
		// 保存先が無ければ作る（既にあれば何もしない）
		CreateDirectoryA(SAVE_DIRECTORY, nullptr);

		SYSTEMTIME time;
		GetLocalTime(&time);
		char path[MAX_PATH];
		sprintf_s(path, "%s/profiler_%04d-%02d-%02d_%02d-%02d-%02d.txt", SAVE_DIRECTORY,
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		FILE* fp = nullptr;
		if (fopen_s(&fp, path, "w") != 0 || fp == nullptr) {
			m_lastSaveMessage = std::string(u8"保存に失敗しました: ") + path;
			return;
		}

		const Node& root = m_nodes[ROOT_INDEX];
		const double frameMs = root.avgMs;
		const double fps = (frameMs > 0.0) ? 1000.0 / frameMs : 0.0;
		fprintf(fp, u8"# フレーム 平均 %.2f ms (%.1f FPS)  最大 %.2f ms\n", frameMs, fps, root.maxMs);
		fprintf(fp, u8"# %u フレームごとの集計 / 重い順 / 一度も呼ばれていない処理は省略\n", INTERVAL_FRAMES);
		{
			const MemoryPeakTracker& memory = MemoryPeakTracker::Get();
			fprintf(fp, u8"# 最大メモリ RAM %.0f MB (確保 %.0f MB) / VRAM %.0f MB (共有 %.0f MB)\n",
				memory.GetPeakWorkingSetMB(), memory.GetPeakCommitMB(),
				memory.GetPeakVramLocalMB(), memory.GetPeakVramNonLocalMB());
		}
		if (m_hasMismatch) {
			fprintf(fp, u8"# 計測区間の開始と終了の対応が崩れたフレームがあります\n");
		}
		fprintf(fp, u8"処理名\t平均ms\t最大ms\t自身ms\t割合%%\t回数\n");
		WriteNode(fp, ROOT_INDEX, 0, frameMs);
		fclose(fp);

		m_lastSaveMessage = std::string(u8"保存しました: ") + path;
	}


	void Profiler::WriteNode(FILE* fp, const int nodeIndex, const int depth, const double frameMs) const
	{
		const Node& node = m_nodes[nodeIndex];
		// 直近の集計期間で一度も呼ばれていない処理（別シーンの処理など）は省く
		if (node.avgCalls <= 0.0f) {
			return;
		}

		const double totalRate = (frameMs > 0.0) ? node.avgMs / frameMs : 0.0;
		fprintf(fp, "%*s%s\t%.3f\t%.3f\t%.3f\t%.1f\t%.1f\n", depth * 2, "", node.name,
			node.avgMs, node.maxMs, node.selfMs, totalRate * 100.0, node.avgCalls);

		// ファイルは見比べやすいよう常に重い順で書き出す
		std::vector<int> sortedChildren = node.children;
		std::sort(sortedChildren.begin(), sortedChildren.end(), [this](const int a, const int b)
			{
				return m_nodes[a].avgMs > m_nodes[b].avgMs;
			});
		for (const int childIndex : sortedChildren) {
			WriteNode(fp, childIndex, depth + 1, frameMs);
		}
	}



	//============================================================
	// GpuProfiler
	//============================================================

	GpuProfiler& GpuProfiler::Get()
	{
		static GpuProfiler instance;
		return instance;
	}


	GpuProfiler::~GpuProfiler()
	{
		for (auto*& readbackBuffer : m_readbackBuffers) {
			if (readbackBuffer != nullptr) {
				readbackBuffer->Release();
				readbackBuffer = nullptr;
			}
		}
		if (m_queryHeap != nullptr) {
			m_queryHeap->Release();
			m_queryHeap = nullptr;
		}
	}


	bool GpuProfiler::Init()
	{
		if (m_isInitialized) return true;
		if (m_isInitFailed) return false;

		ID3D12Device5* device = g_graphicsEngine->GetD3DDevice();
		ID3D12CommandQueue* queue = g_graphicsEngine->GetCommandQueue();
		if (device == nullptr || queue == nullptr || FAILED(queue->GetTimestampFrequency(&m_timestampFrequency))) {
			m_isInitFailed = true;
			return false;
		}

		D3D12_QUERY_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		heapDesc.Count = MAX_QUERIES;
		if (FAILED(device->CreateQueryHeap(&heapDesc, IID_PPV_ARGS(&m_queryHeap)))) {
			m_isInitFailed = true;
			return false;
		}

		const CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_READBACK);
		const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint64_t) * MAX_QUERIES);
		for (auto*& readbackBuffer : m_readbackBuffers) {
			if (FAILED(device->CreateCommittedResource(
				&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readbackBuffer)))) {
				m_isInitFailed = true;
				return false;
			}
		}

		m_isInitialized = true;
		return true;
	}


	void GpuProfiler::BeginFrame(nsK2EngineLow::RenderContext& rc)
	{
		if (!Init()) return;

		// 前のフレームのコマンドを実行に渡した時刻を、そのフレームの記録に残す（開始までの遅れの計算用）
		if (m_lastWrittenFrame >= 0) {
			m_resolvedFrames[m_lastWrittenFrame].executeQpc = g_graphicsEngine->GetLastExecuteQpc();
		}

		ReportResolvedFrame();

		m_queryCount = 0;
		m_samples.clear();
		m_openSamples.clear();
		m_isFrameOpen = WriteTimestamp(rc, m_frameBeginQuery);
	}


	void GpuProfiler::EndFrame(nsK2EngineLow::RenderContext& rc)
	{
		if (!m_isFrameOpen) return;

		// 閉じ忘れの区間があればここで閉じる
		while (!m_openSamples.empty()) {
			EndSample(rc);
		}
		m_isFrameOpen = false;
		if (!WriteTimestamp(rc, m_frameEndQuery)) return;

		rc.GetCommandList()->ResolveQueryData(
			m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, m_queryCount, m_readbackBuffers[m_writeFrame], 0);

		ResolvedFrame& resolved = m_resolvedFrames[m_writeFrame];
		resolved.isValid = true;
		resolved.frameBeginQuery = m_frameBeginQuery;
		resolved.frameEndQuery = m_frameEndQuery;
		resolved.queryCount = m_queryCount;
		resolved.samples.swap(m_samples);
		resolved.executeQpc = 0;
		m_lastWrittenFrame = m_writeFrame;
		m_writeFrame = (m_writeFrame + 1) % NUM_READBACK_FRAMES;
	}


	void GpuProfiler::BeginSample(nsK2EngineLow::RenderContext& rc, const char* name)
	{
		if (!m_isFrameOpen) return;

		Sample sample;
		sample.name = name;
		// 打てなかった区間（-1）は親として扱わず、その上の区間につなげる
		for (auto it = m_openSamples.rbegin(); it != m_openSamples.rend(); ++it) {
			if (*it >= 0) {
				sample.parent = *it;
				break;
			}
		}
		if (!WriteTimestamp(rc, sample.beginQuery)) {
			m_openSamples.push_back(-1);
			return;
		}
		m_samples.push_back(sample);
		m_openSamples.push_back(static_cast<int>(m_samples.size()) - 1);
	}


	void GpuProfiler::EndSample(nsK2EngineLow::RenderContext& rc)
	{
		if (!m_isFrameOpen || m_openSamples.empty()) return;

		const int sampleIndex = m_openSamples.back();
		m_openSamples.pop_back();
		if (sampleIndex < 0) return;

		uint32_t endQuery = 0;
		if (WriteTimestamp(rc, endQuery)) {
			m_samples[sampleIndex].endQuery = endQuery;
		}
		else {
			// 終了を打てなかった区間は0msとして扱う
			m_samples[sampleIndex].endQuery = m_samples[sampleIndex].beginQuery;
		}
	}


	bool GpuProfiler::WriteTimestamp(nsK2EngineLow::RenderContext& rc, uint32_t& outQuery)
	{
		// フレーム終了の分を1つ残しておく
		const uint32_t limit = m_isFrameOpen ? MAX_QUERIES - 1 : MAX_QUERIES;
		if (m_queryCount >= limit) return false;

		outQuery = m_queryCount++;
		rc.GetCommandList()->EndQuery(m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, outQuery);
		return true;
	}


	void GpuProfiler::ReportResolvedFrame()
	{
		// 次に書き込む先 = 一番古い（前々フレームの）記録。これは必ずGPUで終わっている
		ResolvedFrame& resolved = m_resolvedFrames[m_writeFrame];
		if (!resolved.isValid) return;
		resolved.isValid = false;
		ID3D12Resource* readbackBuffer = m_readbackBuffers[m_writeFrame];

		const D3D12_RANGE readRange = { 0, sizeof(uint64_t) * resolved.queryCount };
		void* mapped = nullptr;
		if (FAILED(readbackBuffer->Map(0, &readRange, &mapped)) || mapped == nullptr) return;
		const uint64_t* timestamps = static_cast<const uint64_t*>(mapped);

		const double msPerTick = 1000.0 / static_cast<double>(m_timestampFrequency);
		const auto toMs = [&](const uint32_t beginQuery, const uint32_t endQuery)
			{
				const uint64_t begin = timestamps[beginQuery];
				const uint64_t end = timestamps[endQuery];
				return (end > begin) ? static_cast<double>(end - begin) * msPerTick : 0.0;
			};

		// 「[GPU]」の下に、GPUでの記録順・入れ子のまま並べる
		{
			BEAST_PROFILE_SCOPE(u8"[GPU] 描画 (前々フレームのGPU時間)");
			Profiler& profiler = Profiler::Get();
			profiler.AddTime(u8"GPUフレーム全体", toMs(resolved.frameBeginQuery, resolved.frameEndQuery));

			// タイムスタンプの範囲外の時間（CPUの時刻に換算して求める）
			//  開始までの遅れ: CPUがコマンドを実行に渡してから、GPUが最初のタイムスタンプを打つまで
			//                  （前フレームのPresent、フレーム先頭のクリア、GPUが動き出すまでの遅れ、
			//                    CPUとGPUが並行しているときは前フレームのGPU処理が終わるまでの待ちを含む）
			{
				UINT64 gpuCalibration = 0;
				UINT64 cpuCalibration = 0;
				LARGE_INTEGER qpcFrequency;
				QueryPerformanceFrequency(&qpcFrequency);
				ID3D12CommandQueue* queue = g_graphicsEngine->GetCommandQueue();
				const int64_t executeQpc = resolved.executeQpc;
				if (queue != nullptr && executeQpc != 0
					&& SUCCEEDED(queue->GetClockCalibration(&gpuCalibration, &cpuCalibration)))
				{
					const double qpcPerTick = static_cast<double>(qpcFrequency.QuadPart) / static_cast<double>(m_timestampFrequency);
					const auto gpuToQpc = [&](const uint64_t timestamp)
						{
							return static_cast<double>(cpuCalibration)
								+ (static_cast<double>(timestamp) - static_cast<double>(gpuCalibration)) * qpcPerTick;
						};
					const double msPerQpc = 1000.0 / static_cast<double>(qpcFrequency.QuadPart);
					const double startDelayMs =
						(gpuToQpc(timestamps[resolved.frameBeginQuery]) - static_cast<double>(executeQpc)) * msPerQpc;
					profiler.AddTime(u8"GPU開始までの遅れ (実行指示→最初の記録)", (std::max)(0.0, startDelayMs));
				}
			}

			std::vector<int> pushed;	// いま開いている区間（resolved.samples の番号）
			for (int i = 0; i < static_cast<int>(resolved.samples.size()); ++i) {
				const Sample& sample = resolved.samples[i];
				// 親の区間まで戻る
				while (!pushed.empty() && pushed.back() != sample.parent) {
					profiler.EndExternalSample();
					pushed.pop_back();
				}
				profiler.BeginExternalSample(sample.name, toMs(sample.beginQuery, sample.endQuery));
				pushed.push_back(i);
			}
			while (!pushed.empty()) {
				profiler.EndExternalSample();
				pushed.pop_back();
			}
		}

		const D3D12_RANGE writeRange = { 0, 0 };
		readbackBuffer->Unmap(0, &writeRange);
	}



	//============================================================
	// MemoryPeakTracker
	//============================================================

	MemoryPeakTracker& MemoryPeakTracker::Get()
	{
		static MemoryPeakTracker instance;
		return instance;
	}


	MemoryPeakTracker::~MemoryPeakTracker()
	{
		if (m_adapter != nullptr) {
			m_adapter->Release();
			m_adapter = nullptr;
		}
	}


	void MemoryPeakTracker::Update()
	{
		if (++m_frameCounter < SAMPLE_INTERVAL_FRAMES) return;
		m_frameCounter = 0;
		Sample();
	}


	void MemoryPeakTracker::Sample()
	{
		constexpr double BYTES_PER_MB = 1024.0 * 1024.0;

		// RAM（プロセス全体）。Peak〜 は OS がプロセス開始からの最大値を持っている
		PROCESS_MEMORY_COUNTERS counters = {};
		counters.cb = sizeof(counters);
		if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
			m_peakWorkingSetMB = (std::max)(m_peakWorkingSetMB, counters.PeakWorkingSetSize / BYTES_PER_MB);
			m_peakCommitMB = (std::max)(m_peakCommitMB, counters.PeakPagefileUsage / BYTES_PER_MB);
		}

		// VRAM（アダプターは初回だけ取得して使い回す）
		if (m_adapter == nullptr && !m_isAdapterFailed) {
			m_isAdapterFailed = true;
			ID3D12Device5* device = (g_graphicsEngine != nullptr) ? g_graphicsEngine->GetD3DDevice() : nullptr;
			IDXGIFactory4* factory = nullptr;
			if (device != nullptr && SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
				if (SUCCEEDED(factory->EnumAdapterByLuid(device->GetAdapterLuid(), IID_PPV_ARGS(&m_adapter)))) {
					m_isAdapterFailed = false;
				}
				factory->Release();
			}
		}
		if (m_adapter != nullptr) {
			DXGI_QUERY_VIDEO_MEMORY_INFO local = {};
			DXGI_QUERY_VIDEO_MEMORY_INFO nonLocal = {};
			if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local))) {
				m_peakVramLocalMB = (std::max)(m_peakVramLocalMB, local.CurrentUsage / BYTES_PER_MB);
				m_vramBudgetMB = local.Budget / BYTES_PER_MB;
				// 最大値が前回の書き出しより大きく増えたら、その時点の内訳を残す（ピーク付近の内訳を見るため）
				constexpr double LEDGER_REPORT_STEP_MB = 200.0;
				if (m_peakVramLocalMB >= m_lastLedgerReportMB + LEDGER_REPORT_STEP_MB) {
					m_lastLedgerReportMB = m_peakVramLocalMB;
					nsK2EngineLow::GetGpuMemoryLedger().WriteReport("peak", local.CurrentUsage / BYTES_PER_MB);
				}
			}
			if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &nonLocal))) {
				m_peakVramNonLocalMB = (std::max)(m_peakVramNonLocalMB, nonLocal.CurrentUsage / BYTES_PER_MB);
			}
		}
	}


	void MemoryPeakTracker::WriteVramBreakdown(const char* tag)
	{
		double localMB = 0.0;
		if (m_adapter != nullptr) {
			DXGI_QUERY_VIDEO_MEMORY_INFO local = {};
			if (SUCCEEDED(m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local))) {
				localMB = local.CurrentUsage / (1024.0 * 1024.0);
			}
		}
		nsK2EngineLow::GetGpuMemoryLedger().WriteReport(tag, localMB);
	}


	void MemoryPeakTracker::WriteReport(const char* tag)
	{
		// 最新の値も取り込んでから書く
		Sample();

		CreateDirectoryA("Logs", nullptr);
		FILE* fp = nullptr;
		if (fopen_s(&fp, "Logs/memory_peak.txt", "a") != 0 || fp == nullptr) return;

		SYSTEMTIME time;
		GetLocalTime(&time);
		fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d [%s] RAM peak working set %.0f MB / peak commit %.0f MB, "
			"VRAM peak local %.0f MB (budget %.0f MB) / peak non-local %.0f MB\n",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond,
			(tag != nullptr) ? tag : "",
			m_peakWorkingSetMB, m_peakCommitMB, m_peakVramLocalMB, m_vramBudgetMB, m_peakVramNonLocalMB);
		fclose(fp);
	}
}
