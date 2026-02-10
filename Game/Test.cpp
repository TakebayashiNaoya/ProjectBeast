#include "stdafx.h"
#include "Test.h"


bool Test::Start()
{
	m_modelRender.Init("Assets/modelData/unityChan.tkm");

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
}