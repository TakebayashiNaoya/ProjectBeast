#include "stdafx.h"
#include "Test.h"


bool Test::Start()
{
	m_modelRender.Init("Assets/modelData/unityChan.tkm");
	m_spriteRender.Init("Assets/modelData/utc_all2.DDS", 100.0f, 100.0f);
	m_fontRender.SetText(L"BeastEngine Test!");
	m_fontRender.SetPosition(100.0f, 100.0f); // 画面の見やすい位置に
	m_fontRender.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	InitSky();
	return true;
}

void Test::Update()
{
	if (g_pad[0]->IsPress(enButtonB)) {
		m_pos.x -= 1.0f;
	}
	if (g_pad[0]->IsPress(enButtonA)) {
		m_pos.x += 1.0f;
	}
	m_modelRender.SetPosition(m_pos);
	// g_renderingEngine->DisableRaytracing();
	m_modelRender.Update();
}

void Test::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
	m_spriteRender.Draw(rc);
	m_fontRender.Draw(rc);
}

void Test::InitSky()
{
	// 現在の空を破棄。
	DeleteGO(m_skyCube);

	m_skyCube = NewGO<nsBeastEngine::SkyCube>(0, "skycube");
	m_skyCube->SetType((nsBeastEngine::EnSkyCubeType)m_skyCubeType);
	m_skyCube->SetScale(100.0f);

	// 環境光の計算のためのIBLテクスチャをセットする。
	//nsBeastEngine::g_renderingEngine->nsBeastEngine::SetAmbientByIBLTexture(m_skyCube->GetTextureFilePath(), 0.1f);
}
