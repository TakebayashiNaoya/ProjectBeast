/**
 * @file SpotLight.cpp
 * @brief スポットライトの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"


namespace nsBeastEngine
{
	SpotLight::SpotLight()
		:m_spotLight(nullptr)
	{
		m_spotLight = g_sceneLight->NewSpotLight();
	}


	void SpotLight::Init(const Vector3& position, const Vector3& color, const float& range, const Vector3& direction, const float angle)
	{
		SetPosition(position);
		SetColor(color);
		SetRange(range);
		SetDirection(direction);
		SetAngle(angle);
	}
}