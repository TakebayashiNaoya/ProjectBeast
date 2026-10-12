/**
 * @file Profiler.h
 * @brief CPU処理負荷の計測（簡易階層プロファイラー）
 * @details 計測したい処理に BEAST_PROFILE_SCOPE("処理名") を1行書くだけで、
 *          その処理のCPU時間をImGuiのプロファイラーウィンドウに階層表示する。
 *
 * 「使い方」
 *  void Player::Update()
 *  {
 *      BEAST_PROFILE_FUNCTION();                 // 関数全体を計測（名前は関数名）
 *
 *      {
 *          BEAST_PROFILE_SCOPE(u8"プレイヤー移動");  // スコープを抜けるまでを計測
 *          Move();
 *      }
 *      BEAST_PROFILE_SCOPE(u8"プレイヤーアニメ");     // ここから関数の終わりまでを計測
 *      Animation();
 *  }
 *
 * 「注意」
 *  - 処理名には文字列リテラル（静的な文字列）を渡すこと。ポインタを保持するため、
 *    std::string::c_str() など一時的な文字列は渡してはいけない。
 *  - 計測はメインスレッドのみ。他スレッドからの呼び出しは無視される。
 *  - BEAST_ENABLE_PROFILER が未定義のビルドではマクロは空になり、負荷はゼロ。
 */
#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

struct IDXGIAdapter3;


/**
 * @brief プロファイラーを有効にするかの定義
 * @details 既定ではデバッグビルド（_DEBUG / K2_DEBUG）でのみ有効。
 *          リリースでも計測したい場合は、プリプロセッサ定義に BEAST_ENABLE_PROFILER を追加する。
 */
#if !defined(BEAST_ENABLE_PROFILER) && (defined(_DEBUG) || defined(K2_DEBUG))
#define BEAST_ENABLE_PROFILER
#endif


namespace nsBeastEngine
{
	/**
	 * @brief CPU処理時間を階層的に計測するプロファイラー（シングルトン）
	 * @details 直接使わず、BEAST_PROFILE_SCOPE などのマクロ経由で使う。
	 *          計測値は一定フレームごとに平均・最大を集計して表示する（数値の瞬きを防止）。
	 */
	class Profiler
	{
	public:
		/**
		 * @brief インスタンスの取得
		 * @return プロファイラーのインスタンス
		 */
		static Profiler& Get();

		/**
		 * @brief フレームの区切り
		 * @details ゲームループの先頭で毎フレーム1回呼ぶ。前フレームの計測結果が確定する。
		 *          最初に呼んだスレッドをメインスレッドと見なす。
		 */
		void BeginFrame();

		/**
		 * @brief 計測区間の開始
		 * @param name 処理名（文字列リテラルなど寿命が静的な文字列）
		 */
		void BeginSample(const char* name);

		/**
		 * @brief 計測区間の終了
		 */
		void EndSample();

		/**
		 * @brief 時間を測らずに「回数」だけを足す
		 * @details 現在の計測区間の子に name のノードを作り、回数に count を加える。
		 *          エフェクトの個数など、時間ではない量を「回数」列で見たいときに使う。
		 * @param name  項目名（文字列リテラルなど寿命が静的な文字列）
		 * @param count 足す数
		 */
		void AddCount(const char* name, const uint32_t count);

		/**
		 * @brief 別の場所で測った時間を、現在の計測区間の子として足す
		 * @details 計測マクロを置けない下位のライブラリ（k2EngineLowなど）で測った時間を表示する用。
		 * @param name 項目名（文字列リテラルなど寿命が静的な文字列）
		 * @param ms   足す時間（ミリ秒）
		 */
		void AddTime(const char* name, const double ms);

		/**
		 * @brief 別の場所で測った時間で、入れ子にできる区間を開始する
		 * @details AddTime() と同じく時間を足したうえで、その区間を計測中にする。
		 *          EndExternalSample() で閉じるまでの AddTime() / BeginExternalSample() はこの区間の子になる。
		 *          GPUの計測結果を入れ子で表示する用。
		 * @param name 項目名（文字列リテラルなど寿命が静的な文字列）
		 * @param ms   足す時間（ミリ秒）
		 */
		void BeginExternalSample(const char* name, const double ms);
		/** @brief BeginExternalSample() で開始した区間を閉じる */
		void EndExternalSample();

		/**
		 * @brief 計測結果をImGuiウィンドウに描画する
		 * @details ImGui::NewFrame() と ImGui::Render() の間で呼ぶこと
		 */
		void DrawImGui();

		/** @brief ウィンドウの表示状態を設定 */
		inline void SetVisible(const bool isVisible) { m_isVisible = isVisible; }

		/** @brief ウィンドウの表示状態を取得 */
		inline bool IsVisible() const { return m_isVisible; }

		/** @brief ウィンドウの表示フラグの参照（ImGui::MenuItem に渡す用） */
		inline bool* GetVisibleFlag() { return &m_isVisible; }


	private:
		/**
		 * @brief 計測区間1つ分のノード
		 * @details 「親ノード + 処理名」の組で1ノードを持つ。同じ処理名で呼び出し元が違えば別ノードになる。
		 */
		struct Node
		{
			const char* name = nullptr;  /**< 処理名 */
			int parent = -1;             /**< 親ノードのインデックス（ルートは-1） */
			std::vector<int> children;   /**< 子ノードのインデックス */

			/** 現在のフレームの計測値 */
			int64_t beginTicks = 0;      /**< 計測開始時刻 */
			int64_t frameTicks = 0;      /**< 今フレームの合計時間 */
			uint32_t frameCalls = 0;     /**< 今フレームの呼び出し回数 */

			/** 集計中の値（INTERVAL_FRAMES フレーム分） */
			double sumMs = 0.0;          /**< 合計時間 */
			double peakMs = 0.0;         /**< 1フレームの最大時間 */
			uint32_t sumCalls = 0;       /**< 合計呼び出し回数 */

			/** 表示用に確定した値 */
			double avgMs = 0.0;          /**< 1フレームあたりの平均時間 */
			double maxMs = 0.0;          /**< 1フレームの最大時間 */
			double selfMs = 0.0;         /**< 子を除いた自身の平均時間 */
			float avgCalls = 0.0f;       /**< 1フレームあたりの平均呼び出し回数 */
		};

		/** 全ノード（0番はフレーム全体を表すルート） */
		std::vector<Node> m_nodes;
		/** 計測中のノードのスタック */
		std::vector<int> m_stack;
		/** フレーム時間の履歴（グラフ表示用のリングバッファ） */
		std::vector<float> m_frameHistory;
		/** 履歴の次の書き込み位置 */
		int m_historyOffset = 0;

		/** QueryPerformanceCounter の周波数 */
		int64_t m_ticksPerSecond = 0;
		/** メインスレッドのID（0は未確定） */
		uint32_t m_mainThreadId = 0;
		/** 集計中のフレーム数 */
		uint32_t m_intervalFrameCount = 0;
		/** BeginSample と EndSample の対応が崩れたフレームがあった */
		bool m_hasMismatch = false;

		/** ウィンドウ表示フラグ */
		bool m_isVisible = true;
		/** 集計の一時停止フラグ（表示値を固定して見やすくする用） */
		bool m_isPaused = false;
		/** 子ノードを重い順に並べるか */
		bool m_isSortByTime = false;
		/** 次のフレーム区切りで計測データを破棄する */
		bool m_isResetRequested = false;
		/** 最後にファイルへ保存した結果（ウィンドウに表示する） */
		std::string m_lastSaveMessage;


	private:
		Profiler();

		/** @brief 現在時刻を取得 */
		int64_t GetTicks() const;
		/** @brief 呼び出し元がメインスレッドか */
		bool IsMainThread() const;
		/** @brief ノードを全て破棄してルートだけの状態に戻す */
		void ResetNodes();
		/** @brief 今フレームの計測値を集計に加える */
		void AccumulateFrame();
		/** @brief 集計値を表示用の値に確定させる */
		void FinalizeInterval();
		/** @brief 子ノードを検索し、無ければ作る */
		int FindOrAddChild(const int parentIndex, const char* name);
		/**
		 * @brief 重いフレームの内訳を Logs/profiler_spikes.txt に書き出す
		 * @param frameMs そのフレームの時間
		 */
		void WriteSpike(const double frameMs);
		/** @brief WriteSpike() 用に、ノードとその子孫のこのフレームの時間を書き出す */
		void WriteSpikeNode(FILE* fp, const int nodeIndex, const int depth, const double msPerTick) const;
		/** 書き出したスパイクの数（書きすぎ防止） */
		int m_spikeCount = 0;
		/** 最後にスパイクを書き出した時刻（書きすぎ防止） */
		int64_t m_lastSpikeTicks = 0;

		/** @brief ノードとその子孫をImGuiのテーブル行として描画する */
		void DrawNode(const int nodeIndex, const double frameMs);
		/**
		 * @brief 表示中の計測結果をテキストファイルに保存する
		 * @details Logs/profiler_日時.txt に、ツリーを字下げしたタブ区切りで書き出す
		 */
		void SaveToFile();
		/** @brief ノードとその子孫をファイルに1行ずつ書き出す */
		void WriteNode(FILE* fp, const int nodeIndex, const int depth, const double frameMs) const;
	};




	/**
	 * @brief スコープの開始から終了までを計測するRAIIオブジェクト
	 * @details BEAST_PROFILE_SCOPE マクロから使う
	 */
	class ProfileScope
	{
	public:
		explicit ProfileScope(const char* name)
		{
			Profiler::Get().BeginSample(name);
		}

		~ProfileScope()
		{
			Profiler::Get().EndSample();
		}

		ProfileScope(const ProfileScope&) = delete;
		ProfileScope& operator=(const ProfileScope&) = delete;
	};



	/**
	 * @brief GPU処理時間を描画パスごとに計測するプロファイラー（シングルトン）
	 * @details 描画コマンドリストにタイムスタンプを打ち、次のフレームの頭で読み戻して
	 *          CPUプロファイラーの「[GPU] 描画 (前フレームのGPU時間)」の下に入れ子で表示する。
	 *          直接使わず、BEAST_PROFILE_GPU_SCOPE などのマクロ経由で使う。
	 * @note 読み戻しは前々フレームの結果を読む（GraphicsEngine::EndRender() が前フレームの完了を待ってから
	 *       次を実行に渡すため、前々フレームは必ず終わっている）。
	 */
	class GpuProfiler
	{
	public:
		/** @brief インスタンスの取得 */
		static GpuProfiler& Get();

		/**
		 * @brief フレームの開始（コマンドリストのリセット後に呼ぶ）
		 * @details 前々フレームの結果をCPUプロファイラーへ送り、このフレームの開始時刻を打つ
		 */
		void BeginFrame(nsK2EngineLow::RenderContext& rc);
		/**
		 * @brief フレームの終了（コマンドリストを閉じる前に呼ぶ）
		 * @details 終了時刻を打ち、タイムスタンプを読み戻し用バッファへ書き出すコマンドを積む
		 */
		void EndFrame(nsK2EngineLow::RenderContext& rc);
		/** @brief 計測区間の開始（name は寿命が静的な文字列） */
		void BeginSample(nsK2EngineLow::RenderContext& rc, const char* name);
		/** @brief 計測区間の終了 */
		void EndSample(nsK2EngineLow::RenderContext& rc);

	private:
		/** 1フレームで打てるタイムスタンプの最大数 */
		static constexpr uint32_t MAX_QUERIES = 2048;

		/** 計測区間1つ分 */
		struct Sample
		{
			const char* name = nullptr;	/**< 区間名 */
			int parent = -1;			/**< 親の区間（-1はフレーム直下） */
			uint32_t beginQuery = 0;	/**< 開始時刻のタイムスタンプ番号 */
			uint32_t endQuery = 0;		/**< 終了時刻のタイムスタンプ番号 */
		};

		GpuProfiler() = default;
		~GpuProfiler();

		/** @brief クエリヒープと読み戻し用バッファを作る（初回のみ） */
		bool Init();
		/** @brief タイムスタンプを1つ打つ。打てなければ false */
		bool WriteTimestamp(nsK2EngineLow::RenderContext& rc, uint32_t& outQuery);
		/** @brief 前フレームの結果をCPUプロファイラーへ送る */
		void ReportResolvedFrame();

		ID3D12QueryHeap* m_queryHeap = nullptr;		/**< タイムスタンプのクエリヒープ */
		/**
		 * 読み戻し用の数（フレーム）
		 * @details CPUとGPUが並行して動くと、次のフレームの頭ではまだ前のフレームのGPU処理が終わっていない。
		 *          2フレーム分を交互に使い、前々フレーム（終わっていることが保証される）の結果を読む
		 */
		static constexpr int NUM_READBACK_FRAMES = 2;

		/** 読み戻し待ちのフレーム1つ分 */
		struct ResolvedFrame
		{
			bool isValid = false;			/**< 読み戻し待ちか */
			uint32_t frameBeginQuery = 0;	/**< 開始タイムスタンプ番号 */
			uint32_t frameEndQuery = 0;		/**< 終了タイムスタンプ番号 */
			uint32_t queryCount = 0;		/**< タイムスタンプ数 */
			std::vector<Sample> samples;	/**< 計測区間 */
			int64_t executeQpc = 0;			/**< このフレームのコマンドを実行に渡した時刻（次のフレームの頭で記録） */
		};

		ID3D12Resource* m_readbackBuffers[NUM_READBACK_FRAMES] = {};	/**< タイムスタンプの読み戻し先 */
		ResolvedFrame m_resolvedFrames[NUM_READBACK_FRAMES];			/**< 読み戻し待ちのフレーム */
		int m_writeFrame = 0;											/**< 次に書き込む読み戻し先 */
		int m_lastWrittenFrame = -1;									/**< 最後に書き込んだ読み戻し先 */
		uint64_t m_timestampFrequency = 0;			/**< タイムスタンプの周波数（1秒あたりのカウント） */
		bool m_isInitialized = false;				/**< 初期化済みか */
		bool m_isInitFailed = false;				/**< 初期化に失敗したか（以降は何もしない） */

		bool m_isFrameOpen = false;					/**< フレームの計測中か */
		uint32_t m_queryCount = 0;					/**< このフレームで打ったタイムスタンプ数 */
		uint32_t m_frameBeginQuery = 0;				/**< フレーム開始時刻のタイムスタンプ番号 */
		uint32_t m_frameEndQuery = 0;				/**< フレーム終了時刻のタイムスタンプ番号 */
		std::vector<Sample> m_samples;				/**< このフレームの計測区間 */
		std::vector<int> m_openSamples;				/**< 計測中の区間（-1は打てなかった区間） */

	};


	/**
	 * @brief CPU時間とGPU時間を同時に計測するRAIIオブジェクト
	 * @details BEAST_PROFILE_GPU_SCOPE マクロから使う
	 */
	class GpuProfileScope
	{
	public:
		GpuProfileScope(nsK2EngineLow::RenderContext& rc, const char* name)
			: m_cpuScope(name)
			, m_rc(rc)
		{
			GpuProfiler::Get().BeginSample(rc, name);
		}

		~GpuProfileScope()
		{
			GpuProfiler::Get().EndSample(m_rc);
		}

		GpuProfileScope(const GpuProfileScope&) = delete;
		GpuProfileScope& operator=(const GpuProfileScope&) = delete;

	private:
		ProfileScope m_cpuScope;			/**< CPU側の計測（GPU側より先に始まり後に終わる） */
		nsK2EngineLow::RenderContext& m_rc;	/**< タイムスタンプを打つコマンドリスト */
	};



	/**
	 * @brief RAM・VRAM の最大使用量を記録する（シングルトン）
	 * @details 動作環境（必要メモリ）を調べる用。プロファイラーの有無に関係なく常に動く。
	 *          数フレームごとに使用量を調べて最大値を覚え、WriteReport() で Logs/memory_peak.txt に追記する。
	 *  - RAM : プロセスのピークワーキングセット（実メモリ）とピークコミット（確保したメモリ）
	 *  - VRAM: GPU専用メモリ（local）と共有メモリ（non-local）の使用量
	 */
	class MemoryPeakTracker
	{
	public:
		/** @brief インスタンスの取得 */
		static MemoryPeakTracker& Get();

		/** @brief 毎フレーム呼ぶ（数フレームごとに使用量を調べる） */
		void Update();
		/**
		 * @brief ここまでの最大値を Logs/memory_peak.txt に追記する
		 * @param tag 記録のきっかけ（ステージ名など）
		 */
		void WriteReport(const char* tag);

		/** @brief RAM（ピークワーキングセット）の最大値（MB） */
		double GetPeakWorkingSetMB() const { return m_peakWorkingSetMB; }
		/** @brief RAM（ピークコミット）の最大値（MB） */
		double GetPeakCommitMB() const { return m_peakCommitMB; }
		/** @brief VRAM（GPU専用メモリ）の最大値（MB） */
		double GetPeakVramLocalMB() const { return m_peakVramLocalMB; }
		/** @brief VRAM（共有メモリ）の最大値（MB） */
		double GetPeakVramNonLocalMB() const { return m_peakVramNonLocalMB; }

	private:
		MemoryPeakTracker() = default;
		~MemoryPeakTracker();

		/** @brief 今の使用量を調べて最大値を更新する */
		void Sample();

		/** 使用量を調べる間隔（フレーム数） */
		static constexpr int SAMPLE_INTERVAL_FRAMES = 15;

		IDXGIAdapter3* m_adapter = nullptr;	/**< VRAM を調べるアダプター（初回に取得） */
		bool m_isAdapterFailed = false;		/**< アダプターの取得に失敗したか */
		int m_frameCounter = 0;				/**< 調べる間隔を数える */

		double m_peakWorkingSetMB = 0.0;	/**< RAM: ピークワーキングセット */
		double m_peakCommitMB = 0.0;		/**< RAM: ピークコミット */
		double m_peakVramLocalMB = 0.0;		/**< VRAM: GPU専用メモリの最大 */
		double m_peakVramNonLocalMB = 0.0;	/**< VRAM: 共有メモリの最大 */
		double m_vramBudgetMB = 0.0;		/**< VRAM: GPU専用メモリの予算（参考） */
		double m_lastLedgerReportMB = 0.0;	/**< 最後にVRAMの内訳を書き出したときの最大値 */
	public:
		/**
		 * @brief 【計測用】VRAMの内訳（テクスチャ・レンダリングターゲットの台帳）を Logs/vram_breakdown.txt に書き出す
		 * @param tag 書き出すきっかけ
		 */
		void WriteVramBreakdown(const char* tag);
	};
} // namespace nsBeastEngine


#define BEAST_PROFILE_CONCAT_IMPL(a, b) a##b
#define BEAST_PROFILE_CONCAT(a, b) BEAST_PROFILE_CONCAT_IMPL(a, b)

#if defined(BEAST_ENABLE_PROFILER)
/** スコープの終わりまでを「name」として計測する */
#define BEAST_PROFILE_SCOPE(name) ::nsBeastEngine::ProfileScope BEAST_PROFILE_CONCAT(beastProfileScope_, __LINE__)(name)
/** 時間を測らずに「回数」列へ count を足す（個数などを見る用） */
#define BEAST_PROFILE_COUNT(name, count) ::nsBeastEngine::Profiler::Get().AddCount(name, count)
/** 別の場所で測った時間（ミリ秒）を、現在の計測区間の子として足す */
#define BEAST_PROFILE_ADD_TIME(name, ms) ::nsBeastEngine::Profiler::Get().AddTime(name, ms)
/** 関数全体を関数名で計測する（関数の先頭に書く） */
#define BEAST_PROFILE_FUNCTION() BEAST_PROFILE_SCOPE(__FUNCTION__)
/** CPU時間に加えて、描画コマンドのGPU時間も計測する（rc は RenderContext） */
#define BEAST_PROFILE_GPU_SCOPE(rc, name) ::nsBeastEngine::GpuProfileScope BEAST_PROFILE_CONCAT(beastGpuProfileScope_, __LINE__)(rc, name)
/** GPU計測のフレーム開始（コマンドリストのリセット後に1回だけ書く） */
#define BEAST_PROFILE_GPU_BEGIN_FRAME(rc) ::nsBeastEngine::GpuProfiler::Get().BeginFrame(rc)
/** GPU計測のフレーム終了（コマンドリストを閉じる前に1回だけ書く） */
#define BEAST_PROFILE_GPU_END_FRAME(rc) ::nsBeastEngine::GpuProfiler::Get().EndFrame(rc)
/** フレームの区切り（ゲームループの先頭に1回だけ書く） */
#define BEAST_PROFILE_FRAME() ::nsBeastEngine::Profiler::Get().BeginFrame()
/** 計測結果のウィンドウ描画（ImGuiのフレーム内で1回だけ書く） */
#define BEAST_PROFILE_DRAW_IMGUI() ::nsBeastEngine::Profiler::Get().DrawImGui()
#else
#define BEAST_PROFILE_SCOPE(name) ((void)0)
#define BEAST_PROFILE_COUNT(name, count) ((void)0)
#define BEAST_PROFILE_ADD_TIME(name, ms) ((void)0)
#define BEAST_PROFILE_FUNCTION() ((void)0)
#define BEAST_PROFILE_GPU_SCOPE(rc, name) ((void)0)
#define BEAST_PROFILE_GPU_BEGIN_FRAME(rc) ((void)0)
#define BEAST_PROFILE_GPU_END_FRAME(rc) ((void)0)
#define BEAST_PROFILE_FRAME() ((void)0)
#define BEAST_PROFILE_DRAW_IMGUI() ((void)0)
#endif
