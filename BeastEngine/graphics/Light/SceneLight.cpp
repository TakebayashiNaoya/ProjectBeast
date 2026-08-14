/**
 * @file SceneLight.cpp
 * @brief シーンライトの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "SceneLight.h"


namespace nsBeastEngine
{
	void SPointLight::Update()
	{
		m_positionInView = m_position;
		CameraSystem::Get().GetMainCamera().GetViewMatrix().Apply(m_positionInView);
	}


	void SSpotLight::Update()
	{
		m_positionInView = m_position;
		CameraSystem::Get().GetMainCamera().GetViewMatrix().Apply(m_positionInView);
	}


	void SceneLight::Init()
	{
		/** ディレクションライトの設定 */
		m_light.m_directionLight.SetDirection(-1.0f, -3.0f, 1.0f);
		m_light.m_directionLight.SetColor(3.5f, 3.5f, 3.5f);
		/** カメラの位置の登録 */
		m_light.m_cameraPosition = CameraSystem::Get().GetMainCamera().GetPosition();
		//m_light.m_directionLight.m_LVP = CameraSystem::Get().GetMainCamera().GetViewProjectionMatrix();
		/** 環境光の設定 */
		m_light.SetAmbientLight(0.6f, 0.6f, 0.6f);
		/** リムライトの設定 */
		m_light.SetRimLight(0.0f, 0.0f, 0.0f);
	}


	void SceneLight::Update()
	{
		/** カメラの位置・逆ビュープロジェクション行列をメインカメラ基準で更新する */
		SetViewCamera(CameraSystem::Get().GetMainCamera());

		/** ライトをカメラと見立てたビュー行列を計算する */
		Vector3 lightPosition = m_lightPosition;
		Vector3 lightTarget = m_light.m_directionLight.m_direction + lightPosition;
		Matrix viewMatrix;
		viewMatrix.MakeLookAt(lightPosition, lightTarget, Vector3::Up);

		/** プロジェクション行列を計算する */
		float shadowNear = CameraSystem::Get().GetMainCamera().GetNear() - 4000;
		float shadowFar = CameraSystem::Get().GetMainCamera().GetFar() + 6000;
		Matrix projMatrix;
		projMatrix.MakeOrthoProjectionMatrix(4000, 4000, shadowNear, shadowFar);

		/**
		 * ライトビュープロジェクション行列を更新する
		 * シャドウマップを描画する際に使用される
		 */
		 //Matrix LVP;
		 //LVP = viewMatrix * projMatrix;
		 //m_light.m_directionLight.UpdateLVP(LVP);
	}


	//SPointLight* SceneLight::NewPointLight()
	//{
	//	/** ライトの数が上限に達していたら作らない */
	//	if (m_light.m_usedPointLightCount >= MAX_POINT_LIGHT) return nullptr;

	//	/** 昇順でライトを登録 */
	//	for (int i = 0; i <= MAX_POINT_LIGHT; i++)
	//	{
	//		/** 使用されていないライトがあったら使用中にして登録する */
	//		if (m_light.m_pointLight[i].m_isUsed == false)
	//		{
	//			/* ライトを使用中にする */
	//			m_light.m_pointLight[i].SetIsUsed();
	//			/* ライトの数を増やす */
	//			m_light.m_usedPointLightCount++;
	//			/** 使用中にしたポイントライトのアドレスを返す */
	//			return &m_light.m_pointLight[i];
	//		}
	//	}
	//	return nullptr;
	//}


	//SSpotLight* SceneLight::NewSpotLight()
	//{
	//	/** ライトの数が上限に達していたら作らない */
	//	if (m_light.m_usedSpotLightCount >= MAX_SPOT_LIGHT) return nullptr;

	//	/** 昇順でライトを登録 */
	//	for (int i = 0; i <= MAX_SPOT_LIGHT; i++)
	//	{
	//		/** 使用されていないライトがあったら使用中にして登録する */
	//		if (m_light.m_spotLight[i].m_isUsed == false)
	//		{
	//			/* ライトを使用中にする */
	//			m_light.m_spotLight[i].SetIsUsed();
	//			/* ライトの数を増やす */
	//			m_light.m_usedSpotLightCount++;
	//			/** 使用中にしたスポットライトのアドレスを返す */
	//			return &m_light.m_spotLight[i];
	//		}
	//	}
	//	return nullptr;
	//}
}