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
			const char* PARAMETER_FILE_PATH = "Assets/parameter/Graphics/PBRParameter.bin";
		}

		/** シングルトンインスタンス初期化 */
		PBRStatus* PBRStatus::m_instance = nullptr;


		PBRStatus::PBRStatus()
		{
			core::ParameterManager::Get()->LoadParameterBinary<MasterPBRParameter>(PARAMETER_FILE_PATH);
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
				// parameter->name が char[32] でも、std::string との比較は暗黙的に行われるため問題なし
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