/**
 * @file BeastEngine.h
 * @brief BeastEngineクラスのヘッダー
 */
#pragma once
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

namespace nsBeastEngine
{
	/** 前方宣言 */
	class SceneLight;
	class RenderingEngine;

	/**
	 * @brief BeastEngine
	 * @details 自作エンジン
	 */
	class BeastEngine
	{
	private:
		BeastEngine() {}
		~BeastEngine();

	public:
		/**
		 * @brief 初期化データの構造体
		 * @details k2EngineLowに渡す
		 */
		struct InitData
		{
			HWND hwnd;              /**	ウィドウハンドル */
			UINT frameBufferWidth;  /**	フレームバッファの幅 */
			UINT frameBufferHeight; /**	フレームバッファの高さ */
		};

		/**
		 * @brief BeastEngineの初期化
		 * @param initData 初期化データ
		 */
		void Init(const InitData& initData);

		/**
		 * @brief フレーム前半の処理
		 * @details BeginFrame〜ExecuteRenderまでを実行する
		 *          Application::Render()の前に呼ぶ
		 */
		void BeginExecute();

		/**
		 * @brief フレーム後半の処理
		 * @details RenderingEngine::Execute〜EndFrameまでを実行する
		 *          Application::Render()の後に呼ぶ
		 */
		void EndExecute();


		//============================================//
		// ポーズ関連
		//============================================//

	public:
		/**
		 * @brief ポーズフラグを設定
		 * @details ポーズ中はBeginExecute()内でGameObjectManagerとEffectEngineの更新を止める
		 * @param isPause ポーズフラグ
		 */
		inline void SetPause(const bool isPause) { m_isPause = isPause; }

		/**
		 * @brief ポーズ中かどうかを取得
		 * @return ポーズ中ならtrue
		 */
		inline bool IsPause() const { return m_isPause; }


	private:
		/** ImGui用のSRVヒープを追加 */
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_imguiSrvHeap;
		/** K2EngineLowの実体 */
		K2EngineLow m_k2EngineLow;
		/** レンダリングエンジン */
		RenderingEngine m_renderingEngine;
		/** ポーズフラグ */
		bool m_isPause = false;


		//============================================//
		// シングルトン関連
		//============================================//

	public:
		/**
		 * @brief インスタンスの作成
		 * @param initData 初期化データ
		 */
		static void CreateInstance(const InitData& initData);
		/**
		 * @brief インスタンスの破棄
		 */
		static void DeleteInstance();
		/**
		 * @brief インスタンスの取得
		 * @return BeastEngineのインスタンス
		 */
		static BeastEngine* GetInstance() { return m_instance; }

	private:
		/** BeastEngineのインスタンス */
		static BeastEngine* m_instance;
	};

	/** グローバルにアクセスできる変数 */
	extern BeastEngine* g_beastEngine;
	extern SceneLight* g_sceneLight;
	extern RenderingEngine* g_renderingEngine;
} // namespace nsBeastEngine