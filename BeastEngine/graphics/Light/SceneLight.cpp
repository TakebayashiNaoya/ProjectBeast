#include "BeastEnginePreCompile.h"
#include "SceneLight.h"


namespace nsBeastEngine
{
	SPointLight* SceneLight::NewPointLight()
	{
		/** ライトの数が上限に達していたら作らない */
		if (m_light.m_numPointLig >= MAX_POINT_LIGHT) return nullptr;

		/** 昇順でライトを登録 */
		for (int i = 0; i <= MAX_POINT_LIGHT; i++)
		{
			/** 使用されていないライトがあったら使用中にして登録する */
			if (m_light.m_pointLight[i].m_isUse == false)
			{
				/* ライトを使用中にする */
				m_light.m_pointLight[i].Use();
				/* ライトの数を増やす */
				m_light.m_numPointLig++;
				/** 使用中にしたポイントライトのアドレスを返す */
				return &m_light.m_pointLight[i];
			}
		}
	}


	SSpotLight* SceneLight::NewSpotLight()
	{
		/** ライトの数が上限に達していたら作らない */
		if (m_light.m_numSpotLig >= MAX_SPOT_LIGHT) return nullptr;

		/** 昇順でライトを登録 */
		for (int i = 0; i <= MAX_SPOT_LIGHT; i++)
		{
			/** 使用されていないライトがあったら使用中にして登録する */
			if (m_light.m_spotLight[i].m_isUse == false)
			{
				/* ライトを使用中にする */
				m_light.m_spotLight[i].Use();
				/* ライトの数を増やす */
				m_light.m_numSpotLig++;
				/** 使用中にしたスポットライトのアドレスを返す */
				return &m_light.m_spotLight[i];
			}
		}
	}


	void SceneLight::Init()
	{
		/** ディレクションライトの設定 */
		m_light.m_drectionLight.SetDirection(1.0f, -1.0f, 1.0f);
		m_light.m_drectionLight.SetColor(1.7f, 1.7f, 1.7f);
		/** カメラの位置の登録 */
		m_light.m_cameraPos = g_camera3D->GetPosition();
		m_light.m_drectionLight.m_LVP = g_camera3D->GetViewProjectionMatrix();
		/** 環境光の設定 */
		m_light.SetAmbientLight(0.5f, 0.5f, 0.5f);
	}


	void SPointLight::Update()
	{
		m_positionInView = m_position;
		g_camera3D->GetViewMatrix().Apply(m_positionInView);
	}


	void SSpotLight::Update()
	{
		m_positionInView = m_position;
		g_camera3D->GetViewMatrix().Apply(m_positionInView);
	}


	void SceneLight::Update()
	{
		Matrix LVP;
		Vector3 upAxis = { 0.0f,1.0f,0.0f };

		/** ライトをカメラと見立てたビュー行列を計算する */
		Matrix viewMatrix;
		Vector3 lightPos = m_lightPos;
		Vector3 lightTarget = m_light.m_drectionLight.m_direction + lightPos;
		viewMatrix.MakeLookAt(lightPos, lightTarget, upAxis);

		/** プロジェクション行列を計算する */
		Matrix projMatrix;
		float shadowNear = g_camera3D->GetNear() - 4000;
		float shadowFar = g_camera3D->GetFar() + 6000;
		projMatrix.MakeOrthoProjectionMatrix(4000, 4000, shadowNear, shadowFar);
		LVP = viewMatrix * projMatrix;
		m_light.m_drectionLight.UpdateLVP(LVP);
	}
}