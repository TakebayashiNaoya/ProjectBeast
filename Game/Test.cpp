#include "stdafx.h"
#include "Test.h"


bool Test::Start()
{
	m_modelRender.Init("Assets/modelData/unityChan.tkm");
	m_spriteRender.Init("Assets/modelData/utc_all2.DDS", 100.0f, 100.0f);
	return true;
}

void Test::Update()
{
	// g_renderingEngine->DisableRaytracing();
	m_modelRender.Update();
}

void Test::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
	m_spriteRender.Draw(rc);
}