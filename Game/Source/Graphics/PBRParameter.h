/**
 * @file PBRParameter.h
 * @brief PBR補正パラメーター管理
 * @author 竹林
 */
#pragma once
#include "Source/Core/IMasterParameter.h"
#include "Graphics/ModelRender.h"


namespace app
{
	namespace graphics
	{
		/**
		 * @brief PBR補正パラメーター
		 * @details オブジェクトごとのPBRライティング補正値を保持する
		 */
		struct MasterPBRParameter : public core::IMasterParameter
		{
			appParameter(MasterPBRParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			/** オブジェクト識別名 */
			std::string name;
			/** PBR補正値 */
			nsBeastEngine::PBRParam pbrParam;
		};
	}
}