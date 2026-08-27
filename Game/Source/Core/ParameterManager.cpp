/**
 * @file ParameterManager.cpp
 * @brief パラメーター管理
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
				if (paramPair.second.empty()) continue;

				// 配列の先頭要素のパスとタイムスタンプでファイルが更新されたかチェック
				auto firstParam = paramPair.second.front();
				if (util::JsonConverter::CheckFileModified(firstParam->m_path, firstParam->m_lastWriteTime))
				{
					const std::string& path = firstParam->m_path;
					const bool isBinary = path.size() > 4 && path.compare(path.size() - 4, 4, ".bin") == 0;

					if (isBinary)
					{
						std::ifstream ifs(path, std::ios::binary);
						if (!ifs.is_open()) continue;

						uint32_t count = 0;
						ifs.read(reinterpret_cast<char*>(&count), sizeof(count));

						time_t newTime = util::JsonConverter::GetFileLastWriteTime(path.c_str());

						// ファイル内の要素数とメモリ上の要素数の小さい方に合わせて同期更新
						size_t readCount = (std::min)(paramPair.second.size(), static_cast<size_t>(count));
						for (size_t i = 0; i < readCount; ++i)
						{
							auto param = paramPair.second[i];
							param->m_lastWriteTime = newTime;
							param->Load(ifs);
						}
					}
					else
					{
						// JSONの場合
						nlohmann::json jsonRoot;
						if (!util::JsonConverter::IsLoadJsonFile(jsonRoot, path.c_str())) continue;

						time_t newTime = util::JsonConverter::GetFileLastWriteTime(path.c_str());

						size_t readCount = (std::min)(paramPair.second.size(), jsonRoot.size());
						for (size_t i = 0; i < readCount; ++i)
						{
							auto param = paramPair.second[i];
							param->m_lastWriteTime = newTime;
							param->Load(jsonRoot[i]);
						}
					}
				}
			}
#endif // APP_PARAM_HOT_RELOAD
		}
	}
}


