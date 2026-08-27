/**
 * @file OceanParameter.h
 * @brief 海のパラメーター
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace nature
	{
		/**
		 * @brief 海パラメーター
		 * @note JSON から読み込まれるマスターパラメータです。
		 */
		struct MasterOceanParameter : public core::IMasterParameter
		{
			appParameter(MasterOceanParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}

			void Load(std::istream& stream) override
			{
				constexpr size_t BASE_SIZE = sizeof(core::IMasterParameter);
				constexpr size_t DATA_SIZE = sizeof(MasterOceanParameter) - BASE_SIZE;
				stream.read(reinterpret_cast<char*>(this) + BASE_SIZE, DATA_SIZE);
			}
#endif // APP_PARAM_HOT_RELOAD

			float baseReflectance;	/** 基本反射率 */
			float wave1Amplitude;	/** 波①の振幅 */
			float wave1Frequency;	/** 波①の空間周波数 */
			float wave2Amplitude;	/** 波②の振幅 */
			float wave2Frequency;	/** 波②の空間周波数 */
			float specularPower;	/** スペキュラのPhong指数（大きいほどハイライトが絞られる） */
			float specularScale;	/** スペキュラ強度の倍率（0.0で照り返しを消せる） */
			float ambientScale;		/** 海専用アンビエント強度倍率（他オブジェクトに影響しない） */
			float sizeX = 12500.0f;	/** 海メッシュのX方向サイズ（ワールド単位） */
			float sizeZ = 12500.0f;	/** 海メッシュのZ方向サイズ（ワールド単位） */
		};
	}
}
