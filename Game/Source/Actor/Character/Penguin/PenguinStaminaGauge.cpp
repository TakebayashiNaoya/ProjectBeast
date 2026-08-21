/**
 * @file PenguinStaminaGauge.cpp
 * @brief ジャンプ・スライド共通で使う、スタミナ(オーバーヒート式)ゲージクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "PenguinStaminaGauge.h"


namespace app
{
	namespace actor
	{
		PenguinStaminaGauge::PenguinStaminaGauge(float maxValue, float decreaseSpeed, float recoverSpeed)
			: m_maxValue(maxValue)
			, m_currentValue(maxValue)
			, m_decreaseSpeed(decreaseSpeed)
			, m_recoverSpeed(recoverSpeed)
			, m_isLocked(false)
		{}


		PenguinStaminaGauge::PenguinStaminaGauge()
			: m_maxValue(0.0f)
			, m_currentValue(0.0f)
			, m_decreaseSpeed(0.0f)
			, m_recoverSpeed(0.0f)
			, m_isLocked(false)
		{}


		void PenguinStaminaGauge::Initialize(float maxValue, float decreaseSpeed, float recoverSpeed)
		{
			m_maxValue = maxValue;
			m_decreaseSpeed = decreaseSpeed;
			m_recoverSpeed = recoverSpeed;

			// 満タン・ロック解除の状態にする。
			m_currentValue = maxValue;
			m_isLocked = false;
		}


		void PenguinStaminaGauge::Update(bool isUsing, float deltaTime, float drainScale)
		{
			// 使用中、かつロックされていない場合はゲージを減少させる。
			if (isUsing && !m_isLocked)
			{
				// 消費倍率が負だとゲージが増えてしまうため0で下限を切る。
				const float scale = (drainScale < 0.0f) ? 0.0f : drainScale;
				m_currentValue -= m_decreaseSpeed * scale * deltaTime;

				// 0まで枯渇したら、満タンになるまで使用不可のロック状態にする。
				if (m_currentValue <= 0.0f)
				{
					m_currentValue = 0.0f;
					m_isLocked = true;
				}
				return;
			}

			// 未使用中、またはロック中はゲージを回復させる。
			m_currentValue += m_recoverSpeed * deltaTime;

			if (m_currentValue >= m_maxValue)
			{
				m_currentValue = m_maxValue;
				// 満タンまで回復しきったのでロックを解除する。
				m_isLocked = false;
			}
		}


		void PenguinStaminaGauge::ConsumeAll()
		{
			// すでにロック中（クールダウン中）の場合は何もしない。
			if (m_isLocked) return;

			m_currentValue = 0.0f;
			m_isLocked = true;
		}


		void PenguinStaminaGauge::Reset()
		{
			m_currentValue = m_maxValue;
			m_isLocked = false;
		}
	}
}
