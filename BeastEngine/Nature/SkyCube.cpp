/**
 * @file SkyCube.cpp
 * @brief スカイキューブの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "SkyCube.h"


namespace nsBeastEngine
{
	SkyCube::SkyCube()
		: m_modelRender()
		, m_texture()
		, m_textureFilePaths()
		, m_position(g_vec3Zero)
		, m_scale(g_vec3One * 1000.0f)
		, m_luminance(1.0f)
		, m_isChanged(false)
		, m_type(enSkyCubeType_Day)
	{
		m_textureFilePaths[enSkyCubeType_Day] = L"Assets/modelData/preset/skyCubeMap.dds";
		m_textureFilePaths[enSkyCubeType_Night] = L"Assets/modelData/preset/skyCubeMapNight.dds";
		m_textureFilePaths[enSkyCubeType_Snow] = L"Assets/modelData/preset/skyCubeMapSnow.dds";
		m_textureFilePaths[enSkyCubeType_Snow_2] = L"Assets/modelData/preset/skyCubeMapSnow_2.dds";
		m_textureFilePaths[enSkyCubeType_Wild] = L"Assets/modelData/preset/skyCubeMapWild.dds";
		m_textureFilePaths[enSkyCubeType_Wild_Night] = L"Assets/modelData/preset/skyCubeMapWild_Night.dds";
		m_textureFilePaths[enSkyCubeType_Wild_2] = L"Assets/modelData/preset/skyCubeMapWild_2.dds";
		m_textureFilePaths[enSkyCubeType_Grass] = L"Assets/modelData/preset/skyCubeMapGrass.dds";
		m_textureFilePaths[enSkyCubeType_Euro] = L"Assets/modelData/preset/skyCubeMapEuro.dds";
		m_textureFilePaths[enSkyCubeType_DayToon] = L"Assets/modelData/preset/skyCubeMapDay_Toon.dds";
		m_textureFilePaths[enSkyCubeType_DayToon_2] = L"Assets/modelData/preset/skyCubeMapDay_Toon_02.dds";
		m_textureFilePaths[enSkyCubeType_DayToon_3] = L"Assets/modelData/preset/skyCubeMapDay_Toon_03.dds";
		m_textureFilePaths[enSkyCubeType_DayToon_4] = L"Assets/modelData/preset/skyCubeMapDay_Toon_04.dds";
		m_textureFilePaths[enSkyCubeType_NightToon] = L"Assets/modelData/preset/skyCubeMapNight_Toon.dds";
		m_textureFilePaths[enSkyCubeType_NightToon_2] = L"Assets/modelData/preset/skyCubeMapNight_Toon_02.dds";
		m_textureFilePaths[enSkyCubeType_SunriseToon] = L"Assets/modelData/preset/skyCubeMapSunrise_Toon.dds";
		m_textureFilePaths[enSkyCubeType_SpaceToon_2] = L"Assets/modelData/preset/skyCubeMapSpace_Toon_2.dds";
	}


	bool SkyCube::Start()
	{
		ModelInitData initData;
		/** tkmファイルのファイルパスを指定する */
		initData.m_tkmFilePath = "Assets/modelData/preset/sky.tkm";
		/** シェーダーファイルのファイルパスを指定する */
		initData.m_fxFilePath = "Assets/shader/SkyCubeMap.fx";
		initData.m_vsEntryPointFunc = "VSMain";
		initData.m_psEntryPointFunc = "PSMain";

		/** スカイキューブのテクスチャを読み込む */
		for (int i = 0; i < enSkyCubeType_Num; i++) {
			m_texture[i].InitFromDDSFile(m_textureFilePaths[i]);
		}

		initData.m_expandShaderResoruceView[0] = &m_texture[m_type];
		initData.m_expandConstantBuffer = &m_luminance;
		initData.m_expandConstantBufferSize = sizeof(m_luminance);
		//m_modelRender.GetModel().Init(initData);
		m_modelRender.SetTRS(m_position, g_quatIdentity, m_scale);
		m_modelRender.Update();

		return true;
	}


	void SkyCube::Update()
	{
		if (m_isChanged)
		{
			m_modelRender.SetTRS(m_position, g_quatIdentity, m_scale);
			m_modelRender.Update();
			m_isChanged = false;
		}
	}


	void SkyCube::Render(RenderContext& rc)
	{
		m_modelRender.Draw(rc);
	}
}