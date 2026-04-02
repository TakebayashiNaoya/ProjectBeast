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

		// @todo for test
		m_constantBuffer.light = *g_sceneLight->GetLight();

		// カラーバッファフォーマットを設定する
		// Ocean::Render()が呼ばれるパスの実際のRTVフォーマットに合わせる
		std::array<DXGI_FORMAT, MAX_RENDERING_TARGET> colorBufferFormat = {
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
		};

		// OceanMeshを初期化する
		m_oceanMesh.Init(
			"Assets/shader/Ocean.fx",
			"VSMain",
			"PSMain",
			&m_constantBuffer,
			sizeof(m_constantBuffer),
			colorBufferFormat,
			L"Assets/modelData/Ocean/Vol_36_5_Base_Color.DDS",
			L"Assets/modelData/Ocean/Vol_36_5_Normal.DDS",
			L"Assets/modelData/Ocean/Vol_36_5_Roughness.DDS"
		);

		g_renderingEngine->SetOcean(this);

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
		m_oceanMesh.Draw(rc, CalcWorldMatrix());
	}

}