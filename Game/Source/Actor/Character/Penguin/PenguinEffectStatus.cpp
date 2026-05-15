/**
 * @file PenguinEffectStatus.cpp
 * @brief ペンギンのエフェクトステータスクラス
 * @author 立山
 */
#include "stdafx.h"
#include "PenguinEffectParameter.h"
#include "PenguinEffectStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			const char* PARAMETER_FILE_PATH = "Assets/parameter/character/penguin/PenguinEffectParameter.json";
		}


		PenguinEffectStatus::PenguinEffectStatus()
			: m_splashEffectScale(Vector3::One)
			, m_effectOffsetForward(0.0f)
			, m_splashEffectInterval(0.0f)
			, m_minMoveVelocitySq(0.0f)
			, m_minSplashScaleRatio(0.0f)
			, m_maxSplashScaleRatio(0.0f)
			, m_minSpeed(0.0f)
			, m_maxSpeed(0.0f)
			, m_landingEffectScale(Vector3::One)
			, m_slideEffectScale(Vector3::One)
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameter<MasterPenguinEffectParameter>(PARAMETER_FILE_PATH, [](const nlohmann::json& j, MasterPenguinEffectParameter& parameter)
				{
					parameter.splashEffectScale.x = j["splashEffectScale"]["x"].get<float>();
					parameter.splashEffectScale.y = j["splashEffectScale"]["y"].get<float>();
					parameter.splashEffectScale.z = j["splashEffectScale"]["z"].get<float>();

					parameter.effectOffsetForward = j["effectOffsetForward"].get<float>();
					parameter.splashEffectInterval = j["splashEffectInterval"].get<float>();
					parameter.minMoveVelocitySq = j["minMoveVelocitySq"].get<float>();
					parameter.minSplashScaleRatio = j["minSplashScaleRatio"].get<float>();
					parameter.maxSplashScaleRatio = j["maxSplashScaleRatio"].get<float>();
					parameter.minSpeed = j["minSpeed"].get<float>();
					parameter.maxSpeed = j["maxSpeed"].get<float>();

					parameter.landingEffectScale.x = j["landingEffectScale"]["x"].get<float>();
					parameter.landingEffectScale.y = j["landingEffectScale"]["y"].get<float>();
					parameter.landingEffectScale.z = j["landingEffectScale"]["z"].get<float>();

					parameter.slideEffectScale.x = j["slideEffectScale"]["x"].get<float>();
					parameter.slideEffectScale.y = j["slideEffectScale"]["y"].get<float>();
					parameter.slideEffectScale.z = j["slideEffectScale"]["z"].get<float>();
				});
		}


		PenguinEffectStatus::~PenguinEffectStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterPenguinEffectParameter>();
		}


		void PenguinEffectStatus::Setup()
		{
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterPenguinEffectParameter>();
			if (!parameter) return;

			m_splashEffectScale = parameter->splashEffectScale;
			m_effectOffsetForward = parameter->effectOffsetForward;
			m_splashEffectInterval = parameter->splashEffectInterval;
			m_minMoveVelocitySq = parameter->minMoveVelocitySq;
			m_minSplashScaleRatio = parameter->minSplashScaleRatio;
			m_maxSplashScaleRatio = parameter->maxSplashScaleRatio;
			m_minSpeed = parameter->minSpeed;
			m_maxSpeed = parameter->maxSpeed;
			m_landingEffectScale = parameter->landingEffectScale;
			m_slideEffectScale = parameter->slideEffectScale;
		}


		void PenguinEffectStatus::Update()
		{
			Setup();
		}
	}
}