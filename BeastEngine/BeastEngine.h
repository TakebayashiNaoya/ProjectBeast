/**
 * @file BeastEngine.h
 * @brief BeastEngineクラスのヘッダー
 * @author 竹林尚哉
 */
#pragma once


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


		// RenderingEngine m_renderingEngine; // ← これはK2のレンダラーなので一旦削除！


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
			HWND hwnd;				/**	ウィドウハンドル */
			UINT frameBufferWidth;	/**	フレームバッファの幅 */
			UINT frameBufferHeight;	/**	フレームバッファの高さ */
		};
		/**
		 * @brief BeastEngineの初期化
		 * @param initData 初期化データ
		 */
		void Init(const InitData& initData);
		/**
		 * @brief エンジンの処理を実行
		 */
		void Execute();


	private:
		/** K2EngineLowの実体 */
		K2EngineLow m_k2EngineLow;
		/** レンダリングエンジン */
		RenderingEngine m_renderingEngine;


		/**
		 * @brief シングルトンパターンの実装
		 */
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
	extern RenderingEngine* g_renderingEngine;
}