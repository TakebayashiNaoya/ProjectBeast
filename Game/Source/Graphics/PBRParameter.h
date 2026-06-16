/**
 * @file PBRParameter.h
 * @brief PBR補正パラメーター管理
 * @author 竹林
 */
#pragma once
#include "Graphics/ModelRender.h"
#include "Source/Core/IMasterParameter.h"


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

			void Load(std::istream& stream) override
			{
				loadBinary(stream, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			/** オブジェクト識別名 */
			std::string name;
			/** PBR補正値 */
			nsBeastEngine::PBRParam pbrParam;
		};
	}
}