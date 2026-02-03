/**
 * @file BeastEngine.cpp
 * @brief BeastEngineクラスの実装
 */
#include "BeastEngine.h"
#include "BeastEnginePreCompile.h"


namespace nsBeast
{
	/** 静的メンバ変数の実体を定義 */
	BeastEngine* BeastEngine::m_instance = nullptr;
	BeastEngine* g_beastEngine = nullptr;


	void BeastEngine::CreateInstance(const InitData& initData)
	{
		if (m_instance == nullptr) {
			m_instance = new BeastEngine();
			m_instance->Init(initData);
		}
	}


	void BeastEngine::DeleteInstance()
	{
		delete m_instance;
		m_instance = nullptr;
	}


	BeastEngine::~BeastEngine()
	{
	}


	void BeastEngine::Init(const InitData& initData)
	{
		g_beastEngine = this;

		// K2EngineLowの初期化
		// レイトレ用の設定は一旦ダミーか最小限で渡す
		raytracing::InitData raytracingInitData = { 0 };

		m_k2EngineLow.Init(
			initData.hwnd,
			initData.frameBufferWidth,
			initData.frameBufferHeight,
			raytracingInitData
		);

		/**
		 * TODO: 本来はRenderingEngineの初期化を行うが、K2のレンダラーは使わないのでここではコメントアウトする
		 * m_renderingEngine.Init(initData.isSoftShadow);
		 */

		if (g_camera3D) {
			g_camera3D->SetPosition({ 0.0f, 100.0f, -200.0f }); /** 手前・上に配置 */
			g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });       /** 原点より少し上を見る */
		}
	}


	void BeastEngine::Execute()
	{
		/**
		 * グラフィックスエンジンの「描画コンテキスト」を取得
		 * これを使って「三角形を描け」「色を変えろ」といった命令をGPUに送る
		 */
		auto& renderContext = g_graphicsEngine->GetRenderContext();

		/**
		 * フレーム開始処理
		 * 画面のクリアや、入力情報の更新準備を行う
		 */
		m_k2EngineLow.BeginFrame();

		/**
		 * 更新処理
		 * キャラクターの移動、物理演算、入力判定などを実行
		 * ここで計算した結果をもとに、次の描画処理を行う
		 */
		m_k2EngineLow.ExecuteUpdate();

		// 3. 描画処理 (Render) - ここからフォワードレンダリング！
		// 以前の m_renderingEngine.Execute() の代わりに、自分で描画命令を書きます。

		// 現時点では、モデル描画のコードはまだ書きません。
		// まずは「背景がクリアされたウィンドウが出るか」を確認するため、ここは何もしなくてOKです。
		// 後ほどここに以下の順で処理を追加していきます：
		//  a. 不透明オブジェクトの描画 (Zバッファ書き込み)
		//  b. 半透明オブジェクトの描画 (Zバッファ参照、ブレンド有効)
		//  c. UIの描画

		/**
		 * 不透明オブジェクトの描画
		 * GameObjectManagerに登録されているオブジェクト（ModelRenderを持つもの）を一斉に描画
		 * ここで GPU に対して「モデルの頂点データを描け」という命令が発行される
		 */
		GameObjectManager::GetInstance()->ExecuteRender(renderContext);

		// 半透明オブジェクトやエフェクト（後で実装）
		// EffectEngine::GetInstance()->Draw();

		/** デバッグ用の線画（コライダの枠など）を描画 */
		m_k2EngineLow.DebubDrawWorld();

		/**
		 * フレーム終了処理
		 * バックバッファの内容を画面に転送（Present）して、描画を完了させる
		 */
		m_k2EngineLow.EndFrame();
	}
}


//#include "k2EnginePreCompile.h"
//#include "k2Engine.h"
//#include "dbg/dbgVectorRenderer.h"
//
//namespace nsK2Engine {
//	K2Engine* K2Engine::m_instance = nullptr;
//	SceneLight* g_sceneLight = nullptr;
//	RenderingEngine* g_renderingEngine = nullptr;
//	CollisionObjectManager* g_collisionObjectManager = nullptr;
//	K2Engine* g_k2Engine = nullptr;
//	void K2Engine::Init(const InitData& initData)
//	{
//		g_k2Engine = this;
//		g_engine = &m_k2EngineLow;
//		g_collisionObjectManager = &m_collisionObjectManager;
//		g_renderingEngine = &m_renderingEngine;
//
//		raytracing::InitData raytracintInitData;
//		raytracintInitData.m_expandShaderResource = &m_renderingEngine.GetRaytracingLightData();
//		raytracintInitData.m_expandShaderResourceSize = sizeof(m_renderingEngine.GetRaytracingLightData());
//#ifdef COPY_RAYTRACING_FRAMEBUFFER
//		// フレームバッファにコピーする場合は、レイトレの出力先のカラーバッファのフォーマットを
//		// フレームバッファと同じにする。
//		raytracintInitData.m_outputColorBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
//#endif // #ifdef COPY_RAYTRACING_FRAMEBUFFER
//		m_k2EngineLow.Init(
//			initData.hwnd,
//			initData.frameBufferWidth,
//			initData.frameBufferHeight,
//			raytracintInitData
//		);
//		m_renderingEngine.Init(initData.isSoftShadow);
//		g_camera3D->SetPosition({ 0.0f, 100.0f, -200.0f });
//		g_camera3D->SetTarget({ 0.0f, 50.0f, 0.0f });
//#ifdef K2_DEBUG
//		// デバッグベクトルレンダラーを作成する。
//		m_vectorRenderer = NewGO<nsDbg::VectorRenderer>(254, "VectorRender");
//#endif // #ifdef K2_DEBUG
//	}
//	K2Engine::~K2Engine()
//	{
//#ifdef K2_DEBUG
//		DeleteGO(m_vectorRenderer);
//#endif
//		g_renderingEngine = nullptr;
//		g_collisionObjectManager = nullptr;
//		g_engine = nullptr;
//	}
//
//	void K2Engine::Execute()
//	{
//		auto& renderContext = g_graphicsEngine->GetRenderContext();
//
//		g_engine->BeginFrame();
//
//		g_engine->ExecuteUpdate();
//		// レンダリングエンジンの更新。
//		m_renderingEngine.Update();
//
//		g_engine->ExecuteRender();
//		//レンダリングエンジンを実行。		
//		m_renderingEngine.Execute(renderContext);
//
//		//当たり判定描画。
//		g_engine->DebubDrawWorld();
//
//		//////////////////////////////////////
//		//絵を描くコードを書くのはここまで！！！
//		//////////////////////////////////////
//		g_engine->EndFrame();
//	}
//#ifdef K2_DEBUG
//	void K2Engine::DrawVector(const Vector3& vector, const Vector3& origin, const char* name)
//	{
//		nsDbg::VectorRenderer::SRenderData renderData;
//		renderData.origin = origin;
//		renderData.vector = vector;
//		renderData.name = name;
//		m_vectorRenderer->AddVector(renderData);
//	}
//	void K2Engine::SetDrawVectorEnable()
//	{
//		m_vectorRenderer->EnableRender();
//	}
//	void K2Engine::SetDrawVectorDisable()
//	{
//		m_vectorRenderer->DisableRender();
//	}
//#endif // #ifdef K2_DEBUG
//}