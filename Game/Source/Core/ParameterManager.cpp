/**
 * @file ParameterManager.cpp
 * @brief パラメーター管理
 * @author 藤谷
 */
#include "stdafx.h"
#include "IMasterParameter.h"
#include "ParameterManager.h"


namespace app
{
	namespace core
	{
		/** シングルトンインスタンス初期化 */
		ParameterManager* ParameterManager::m_instance = nullptr;


		ParameterManager::ParameterManager()
		{
			m_parameterMap.clear();
		}


		ParameterManager::~ParameterManager()
		{
			for (auto paramPair : m_parameterMap)
			{
				for (const auto& param : paramPair.second)
				{
					delete param;
				}
			}
			m_parameterMap.clear();
		}


		void ParameterManager::Update()
		{
#ifdef APP_PARAM_HOT_RELOAD
			for (auto paramPair : m_parameterMap)
			{
				for (const auto& param : paramPair.second)
				{
					if (util::JsonConverter::CheckFileModified(param->m_path, param->m_lastWriteTime))
					{
						nlohmann::json jsonRoot;
						if (!util::JsonConverter::IsLoadJsonFile(jsonRoot, param->m_path.c_str())) return;

						ParameterVector parameters;

						for (auto& j : jsonRoot)
						{
							param->m_lastWriteTime = util::JsonConverter::GetFileLastWriteTime(param->m_path.c_str());
							param->Load(j);
						}
					}
				}
			}
#endif // APP_PARAM_HOT_RELOAD
		}
	}
}


