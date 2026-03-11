#include "BeastEnginePreCompile.h"
#include "Ocean.h"

namespace
{
	const float BASEREFLECTANCE = 0.05f;
}

namespace nsBeastEngine
{
	bool Ocean::Start()
	{
		BeginGPUEvent("Ocean");

		//SetConstatntBuffer(
		//	g_renderingEngine->GetReflectViewProjectionMatrix(ReflectLayer::enOcean),
		//	g_renderingEngine->GetSceneLight().GetLight(),
		//	g_renderingEngine->GetReflectCamera(ReflectLayer::enOcean).GetPosition(),
		//	BASEREFLECTANCE
		//);

		//g_renderingEngine->SetReflectPlane(m_plane, ReflectLayer::enOcean);

		ModelInitData initData;
		//tkmファイルのファイルパスを指定する。
		initData.m_tkmFilePath = "Assets/modelData/Ocean/ocean.tkm";
		//シェーダーファイルのファイルパスを指定する。
		initData.m_fxFilePath = "Assets/shader/Ocean.fx";
		initData.m_vsEntryPointFunc = "VSMain";
		initData.m_psEntryPointFunc = "PSMain";

		//m_reflectionRenderTarget = &g_renderingEngine->GetPlaneReflectionRenderTarget(ReflectLayer::enOcean);
		//initData.m_expandShaderResoruceView[0] = &m_reflectionRenderTarget->GetRenderTargetTexture();
		//initData.m_expandShaderResoruceView[1] = &g_renderingEngine->GetShadowTexture();

		// @todo for test
		m_constantBuffer.light = *g_sceneLight->GetLight();


		initData.m_expandConstantBuffer = &m_constantBuffer;
		initData.m_expandConstantBufferSize = sizeof(m_constantBuffer);


		m_modelRender.InitOcean(initData, initData.m_tkmFilePath);
		m_modelRender.SetTRS(m_position, g_quatIdentity, m_scale);
		//m_modelRender.ChangeAlbedoMap("", m_reflectionRenderTarget->GetRenderTargetTexture());
		m_modelRender.Update();

		//当たり判定の初期化
		//m_physics.CreateFromModel(m_modelRender.GetModel(), m_modelRender.GetModel().GetWorldMatrix());

		return true;
	}


	void Ocean::Update()
	{
		//SetConstatntBuffer(
		//	g_renderingEngine->GetReflectViewProjectionMatrix(ReflectLayer::enOcean),
		//	g_renderingEngine->GetSceneLight().GetLight(),
		//	g_renderingEngine->GetReflectCamera(ReflectLayer::enOcean).GetPosition(),
		//	BASEREFLECTANCE
		//);
		m_constantBuffer.light = *g_sceneLight->GetLight();
		UpdateWaveOffset();
	}


	void Ocean::Render(RenderContext& rc)
	{
		m_modelRender.Draw(rc);
	}

}
