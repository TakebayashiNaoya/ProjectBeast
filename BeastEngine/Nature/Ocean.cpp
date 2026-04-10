/**
 * @file Ocean.cpp
 * @brief 海のクラス
 * @author 竹林
 */
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
		// CSに渡す波パラメータを構築し、Draw の冒頭でDispatchされる
		m_oceanMesh.Draw(rc, CalcWorldMatrix(), BuildWaveCb());
	}


	OceanMesh::SWaveConstantBuffer Ocean::BuildWaveCb() const
	{
		OceanMesh::SWaveConstantBuffer waveCb;
		waveCb.waveScroll = m_constantBuffer.waveScroll;
		waveCb.wave1Amplitude = m_constantBuffer.wave1Amplitude;
		waveCb.wave1Frequency = m_constantBuffer.wave1Frequency;
		waveCb.wave2Amplitude = m_constantBuffer.wave2Amplitude;
		waveCb.wave2Frequency = m_constantBuffer.wave2Frequency;
		waveCb.gridHalfSize = OceanMesh::GRID_SIZE * 0.5f;
		waveCb.cellSize = OceanMesh::GRID_SIZE / static_cast<float>(OceanMesh::GRID_DIVISION);
		waveCb.numVertsPerRow = OceanMesh::GRID_DIVISION + 1;
		return waveCb;
	}


	float Ocean::SampleWaveHeight(float worldX, float worldZ) const
	{
		// グリッド設定
		constexpr float gridHalfSize = OceanMesh::GRID_SIZE * 0.5f;
		constexpr float cellSize = OceanMesh::GRID_SIZE / static_cast<float>(OceanMesh::GRID_DIVISION);
		constexpr int   numVertsPerRow = OceanMesh::GRID_DIVISION + 1;

		// ワールド座標をグリッドローカル座標（0〜GRID_DIVISION）に変換する
		const float localX = (worldX + gridHalfSize) / cellSize;
		const float localZ = (worldZ + gridHalfSize) / cellSize;

		// グリッド範囲外はクランプする
		const float clampedX = max(0.0f, min(localX, static_cast<float>(OceanMesh::GRID_DIVISION - 1)));
		const float clampedZ = max(0.0f, min(localZ, static_cast<float>(OceanMesh::GRID_DIVISION - 1)));

		// 左下インデックスと格子内補間比率を求める
		const int   ix = static_cast<int>(clampedX);
		const int   iz = static_cast<int>(clampedZ);
		const float fx = clampedX - static_cast<float>(ix);
		const float fz = clampedZ - static_cast<float>(iz);

		// キャッシュから4隅の波高さを取得する
		const float* cache = m_oceanMesh.GetWaveHeightCache();
		const float  p00 = cache[iz * numVertsPerRow + ix];
		const float  p10 = cache[iz * numVertsPerRow + ix + 1];
		const float  p01 = cache[(iz + 1) * numVertsPerRow + ix];
		const float  p11 = cache[(iz + 1) * numVertsPerRow + ix + 1];

		// バイリニア補間して返す
		const float top = p00 + (p10 - p00) * fx;
		const float bottom = p01 + (p11 - p01) * fx;
		return top + (bottom - top) * fz;
	}
}