/**
 * @file RenderingEngine.cpp
 * @brief RenderingEngineクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "RenderingEngine.h"


namespace nsBeastEngine
{
	RenderingEngine::RenderingEngine()
	{
		g_renderingEngine = this;
	}


	RenderingEngine::~RenderingEngine()
	{
		g_renderingEngine = nullptr;
	}


	void RenderingEngine::Init()
	{
		// 現段階では空でOKです。
		// 後々、ここにレンダーターゲットやシャドウマップの初期化が入ります。
	}


	void RenderingEngine::AddRenderObject(IRenderer* renderObject)
	{
		if (renderObject != nullptr) {
			m_renderObjects.push_back(renderObject); // リストに追加
		}
	}

	void RenderingEngine::Execute(RenderContext& rc)
	{
		// 1. 登録されたオブジェクトの2D描画を順に実行
		for (auto* obj : m_renderObjects) {
			obj->OnRender2D(rc);
		}

		// 2. 描画が終わったらリストを空にして、次のフレームに備える
		m_renderObjects.clear();
	}
}