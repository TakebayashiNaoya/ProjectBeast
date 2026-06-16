/**
 * @file PBRStatus.cpp
 * @brief PBR補正パラメーターのステータスクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "PBRStatus.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace graphics
	{
		namespace
		{
			/** PBR補正パラメーターのファイルパス */
			//const char* PARAMETER_FILE_PATH = "Assets/parameter/Graphics/PBRParameter.json";
			const char* PARAMETER_FILE_PATH = "Assets/parameter/Graphics/PBRParameter.bin";
		}

		/** シングルトンインスタンス初期化 */
		PBRStatus* PBRStatus::m_instance = nullptr;


		PBRStatus::PBRStatus()
		{
			// 外部ファイルを読み込み
			//core::ParameterManager::Get()->LoadParameter<MasterPBRParameter>(PARAMETER_FILE_PATH, [](const nlohmann::json& j, MasterPBRParameter& parameter)
			//	{
			//		parameter.name = j["name"].get<std::string>();
			//		parameter.pbrParam.m_dirLightScale = j["dirLightScale"].get<float>();
			//		parameter.pbrParam.m_ambientScale = j["ambientScale"].get<float>();
			//		parameter.pbrParam.m_metallicOffset = j["metallicOffset"].get<float>();
			//		parameter.pbrParam.m_smoothOffset = j["smoothOffset"].get<float>();
			//	});


			core::ParameterManager::Get()->LoadParameterBinary<MasterPBRParameter>(PARAMETER_FILE_PATH, [](std::istream& stream, MasterPBRParameter& parameter)
				{
					// name の読み込み
					char nameBuffer[32] = { 0 };
					stream.read(nameBuffer, sizeof(nameBuffer));
					parameter.name = std::string(nameBuffer);
					stream.read(reinterpret_cast<char*>(&parameter.pbrParam.m_dirLightScale), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.pbrParam.m_ambientScale), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.pbrParam.m_metallicOffset), sizeof(float));
					stream.read(reinterpret_cast<char*>(&parameter.pbrParam.m_smoothOffset), sizeof(float));
				});
		}


		PBRStatus::~PBRStatus()
		{
			// 使用終了
			core::ParameterManager::Get()->UnloadParameter<MasterPBRParameter>();
		}


		nsBeastEngine::PBRParam PBRStatus::GetPBRParam(const std::string& name) const
		{
			// 名前が一致するパラメーターを線形探索する
			const auto parameters = core::ParameterManager::Get()->GetParameters<MasterPBRParameter>();
			for (const auto* parameter : parameters)
			{
				if (parameter->name == name)
				{
					return parameter->pbrParam;
				}
			}

			// 見つからなかった場合はデフォルト値を返す
			return nsBeastEngine::PBRParam{};
		}
	}
}