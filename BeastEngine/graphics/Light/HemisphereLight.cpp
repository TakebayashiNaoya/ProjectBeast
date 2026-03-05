/**
 * @file HemisphereLight.h
 * @brief 半球ライトの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "HemisphereLight.h"


namespace nsBeastEngine
{
	HemisphereLight::HemisphereLight()
		: m_hemisphereLight(nullptr)
	{
		/** 半球ライトを取得 */
		//m_hemisphereLight = g_sceneLight->GetHemisphereLight();
	}
}