/**
 * @file ParameterManager.cpp
 * @brief パラメーター管理
 * @author 藤谷
 */
#include "stdafx.h"
#include "IMasterParameter.h"
#include "ParameterManager.h"
#include <fstream>



namespace app
{
	namespace core
	{
		ParameterManager* ParameterManager::m_instance = nullptr; //初期化


		ParameterManager::ParameterManager()
		{
			m_parameterMap.clear();
		}


		void ParameterManager::Update()
		{
#ifdef APP_PARAM_HOT_RELOAD
			for (auto paramPair : m_parameterMap)
			{
				for (const auto& param : paramPair.second)
				{
					if (CheckFileModified(param.get()))
					{
						std::ifstream file(param->m_path);
						if (!file.is_open())
						{
							return;
						}

						nlohmann::json jsonRoot;
						file >> jsonRoot;

						ParameterVector parameters;

						for (const auto& j : jsonRoot)
						{
							param->m_lastWriteTime = GetFileLastWriteTime(param->m_path.c_str());
							param->Load(j);
						}
					}
				}
			}
#endif
		}


#ifdef APP_PARAM_HOT_RELOAD
		time_t ParameterManager::GetFileLastWriteTime(const char* path)
		{
			struct stat result;
			// stat関数でファイル情報を取得 (0なら成功)
			if (stat(path, &result) == 0) {
				return result.st_mtime;
			}
			return 0;
		}


		bool ParameterManager::CheckFileModified(const IMasterParameter* param)
		{
			if (GetFileLastWriteTime(param->m_path.c_str()) > param->m_lastWriteTime)
			{
				return true;
			}
			return false;
		}
#endif // APP_PARAM_HOT_RELOAD
	}
}


