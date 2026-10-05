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
#include <vector>


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
		/** @brief ノードとその子孫をImGuiのテーブル行として描画する */
		void DrawNode(const int nodeIndex, const double frameMs);
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
} // namespace nsBeastEngine


#define BEAST_PROFILE_CONCAT_IMPL(a, b) a##b
#define BEAST_PROFILE_CONCAT(a, b) BEAST_PROFILE_CONCAT_IMPL(a, b)

#if defined(BEAST_ENABLE_PROFILER)
/** スコープの終わりまでを「name」として計測する */
#define BEAST_PROFILE_SCOPE(name) ::nsBeastEngine::ProfileScope BEAST_PROFILE_CONCAT(beastProfileScope_, __LINE__)(name)
/** 関数全体を関数名で計測する（関数の先頭に書く） */
#define BEAST_PROFILE_FUNCTION() BEAST_PROFILE_SCOPE(__FUNCTION__)
/** フレームの区切り（ゲームループの先頭に1回だけ書く） */
#define BEAST_PROFILE_FRAME() ::nsBeastEngine::Profiler::Get().BeginFrame()
/** 計測結果のウィンドウ描画（ImGuiのフレーム内で1回だけ書く） */
#define BEAST_PROFILE_DRAW_IMGUI() ::nsBeastEngine::Profiler::Get().DrawImGui()
#else
#define BEAST_PROFILE_SCOPE(name) ((void)0)
#define BEAST_PROFILE_FUNCTION() ((void)0)
#define BEAST_PROFILE_FRAME() ((void)0)
#define BEAST_PROFILE_DRAW_IMGUI() ((void)0)
#endif
