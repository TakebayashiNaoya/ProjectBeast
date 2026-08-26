/**
 * @file PenguinEffectStatus.cpp
 * @brief ペンギンのエフェクトステータスクラス
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
			//const char* PARAMETER_FILE_PATH = "Assets/parameter/character/penguin/PenguinEffectParameter.json";

			const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/penguin/PenguinEffectParameter.bin";
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
			, m_slideFrostEffectScale(Vector3::One)
			, m_slideEffectInterval(0.0f)
			, m_minSlideFrostEffectScaleRatio(0.0f)
			, m_maxSlideFrostEffectScaleRatio(0.0f)
			, m_slideLineEffectScale(Vector3::One)
			, m_slideLineOffsetForward(0.0f)
		{
			// 外部ファイルを読み込み
			core::ParameterManager::Get()->LoadParameterBinary<MasterPenguinEffectParameter>(
				PARAMETER_BINARY_FILE_PATH
			);
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
			m_slideFrostEffectScale = parameter->slideFrostEffectScale;
			m_slideEffectInterval = parameter->slideEffectInterval;
			m_minSlideFrostEffectScaleRatio = parameter->minSlideFrostScaleRatio;
			m_maxSlideFrostEffectScaleRatio = parameter->maxSlideFrostScaleRatio;
			m_slideLineEffectScale = parameter->slideLineEffectScale;
			m_slideLineOffsetForward = parameter->slideLineEffectOffsetForward;
		}


		void PenguinEffectStatus::Update()
		{
			Setup();
		}
	}
}