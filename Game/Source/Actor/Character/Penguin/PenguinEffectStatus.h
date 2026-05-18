/**
 * @file PenguinEffectStatus.h
 * @brief ペンギンのエフェクトステータス
 * @author 立山
 */
#pragma once
#include "Source/Actor/ActorStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ペンギンの演出・エフェクト用ステータスクラス
		 */
		class PenguinEffectStatus : public ActorStatus
		{
		public:
			inline const Vector3& GetSplashEffectScale() const { return m_splashEffectScale; }
			inline float GetEffectOffsetForward() const { return m_effectOffsetForward; }
			inline float GetSplashEffectInterval() const { return m_splashEffectInterval; }
			inline float GetMinMoveVelocitySq() const { return m_minMoveVelocitySq; }
			inline float GetMinSplashScaleRatio() const { return m_minSplashScaleRatio; }
			inline float GetMaxSplashScaleRatio() const { return m_maxSplashScaleRatio; }
			inline float GetMinSpeed() const { return m_minSpeed; }
			inline float GetMaxSpeed() const { return m_maxSpeed; }
			inline const Vector3& GetLandingEffectScale() const { return m_landingEffectScale; }
			inline const Vector3& GetSlideEffectScale() const { return m_slideEffectScale; }
			inline float GetSlideEffectInterval() const { return m_slideEffectInterval; }


		public:
			void Setup() override;

			void Update() override;


		public:
			PenguinEffectStatus();
			~PenguinEffectStatus() override;


		protected:
			Vector3 m_splashEffectScale;
			float m_effectOffsetForward;
			float m_splashEffectInterval;
			float m_minMoveVelocitySq;
			float m_minSplashScaleRatio;
			float m_maxSplashScaleRatio;
			float m_minSpeed;
			float m_maxSpeed;
			Vector3 m_landingEffectScale;
			Vector3 m_slideEffectScale;
			float m_slideEffectInterval;
		};
	}
}