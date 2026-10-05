/**
 * @file Profiler.cpp
 * @brief CPU処理負荷の計測（簡易階層プロファイラー）の実装
 */
#include "BeastEnginePreCompile.h"
#include "Profiler.h"
#include "imgui.h"
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
}
