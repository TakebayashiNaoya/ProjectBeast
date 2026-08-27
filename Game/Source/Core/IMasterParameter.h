/**
 * @file IMasterParameter.h
 * @brief パラメーター基底クラス
 */
#pragma once
#include "AppParameterMacro.h"
#include "Json/json.hpp"


namespace app
{
	namespace core
	{
		/** 基底クラス。必ず継承すること！ */
		struct IMasterParameter
		{
#ifdef APP_PARAM_HOT_RELOAD
			virtual void Load(const nlohmann::json& j) {};
			virtual void Load(std::istream& stream) {};
			std::string m_path;         // パラメーターのファイルパス（ホットリロード用）
			time_t m_lastWriteTime;     // 最終更新時刻
#endif // APP_PARAM_HOT_RELOAD

			virtual ~IMasterParameter() = default;
		};
	}
}