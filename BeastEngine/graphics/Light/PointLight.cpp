/**
 * @file PointLight.cpp
 * @brief ポイントライトの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "PointLight.h"


namespace nsBeastEngine
{
	PointLight::PointLight()
		: m_pointLight(nullptr)
	{
		/** ポイントライトを取得 */
		m_pointLight = g_sceneLight->NewPointLight();
	}


	void PointLight::Init(const Vector3& pos, const Vector3& color, const float& range)
	{
		m_pointLight->SetPosition(pos);
		m_pointLight->SetColor(color);
		m_pointLight->SetRange(range);
	}
}