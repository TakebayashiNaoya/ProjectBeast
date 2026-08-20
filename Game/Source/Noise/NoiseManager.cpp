/**
 * @file NoiseManager.cpp
 * @brief 音（ノイズ）イベントを管理し、エネミーの検知処理を補助するマネージャー
 * @author 竹林
 */
#include "stdafx.h"
#include "NoiseManager.h"


namespace app
{
	/** シングルトンインスタンスの初期化 */
	NoiseManager* NoiseManager::m_instance = nullptr;


	NoiseParameter NoiseManager::GetDefaultParameter(EnNoiseType type) const
	{
		switch (type)
		{
		case EnNoiseType::Sneak:
			return { 100.0f, 1.0f, 200.0f };
		case EnNoiseType::Dash:
			return { 260.0f, 1.8f, 400.0f };
		case EnNoiseType::Slide:
			return { 250.0f, 1.9f, 500.0f };
		case EnNoiseType::Fall:
			return { 400.0f, 1.0f, 500.0f };
		case EnNoiseType::ClumsyCRY:
			return { 7000.0f,0.5f,1000.0f };
		case EnNoiseType::NaughtyPoke:
			return { 2000.0f, 0.5f, 1000.0f };
		default:
			return { 0.0f, 0.0f, 0.0f };
		}
	}


	void NoiseManager::AddNoise(const Vector3& position, EnNoiseType type, int penguinId)
	{
		NoiseParameter param = GetDefaultParameter(type);
		AddNoise(position, param.volume, param.falloffRatio, param.range, { type, penguinId });
	}


	void NoiseManager::AddNoise(const Vector3& position, float intensity, float attenuationRate, float radius,
		const NoiseSource& source)
	{
		m_noises.push_back({ position, intensity, attenuationRate, radius, source });
	}


	float NoiseManager::CalculateTotalNoiseAt(const Vector3& listenerPos, Vector3& outLoudestPosition) const
	{
		NoiseSource dummy;
		return CalculateTotalNoiseAt(listenerPos, outLoudestPosition, dummy);
	}


	float NoiseManager::CalculateTotalNoiseAt(const Vector3& listenerPos, Vector3& outLoudestPosition,
		NoiseSource& outLoudestSource) const
	{
		float totalNoise = 0.0f;
		float maxHeardIntensity = -1.0f;

		for (const auto& noise : m_noises)
		{
			/** リスナーと音源の距離を計算 */
			Vector3 vector = listenerPos - noise.position;
			float distance = vector.Length();

			/** 最大到達距離を超えていたら足切り */
			if (distance > noise.range) {
				continue;
			}

			/** 減衰率を適用して実際に届く音量を計算 */
			float heardVolume = noise.volume - (distance * noise.falloffRatio);
			heardVolume = max(0.0f, heardVolume);

			if (heardVolume > 0.0f)
			{
				totalNoise += heardVolume;

				/** 最も大きく聞こえた音の座標と、その音を出した主を記録 */
				if (heardVolume > maxHeardIntensity) {
					maxHeardIntensity = heardVolume;
					outLoudestPosition = noise.position;
					outLoudestSource = noise.source;
				}
			}
		}

		return totalNoise;
	}


	void NoiseManager::ClearNoises()
	{
		m_noises.clear();
	}
}