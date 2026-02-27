/**
 * @file JsonLoadingInfomation.cpp
 * @brief jsonファイルの読み込みに使用する
 * @author 藤谷
 */
#include "stdafx.h"
#include "JsonConverter.h"
#include <fstream>

namespace app
{
	namespace util
	{
		bool JsonConverter::IsLoadJsonFile(nlohmann::json& json, const std::string& filePath)
		{
			// ファイルストリームを開く
			std::ifstream file(filePath);

			// ファイルが開けなかった場合
			if (!file.is_open()) {
				// 読み込み失敗
				return false;
			}

			nlohmann::json jsonTemp;

			// jsonの読み込みを試す
			try
			{
				file >> jsonTemp;
			}
			// 例外が発生した場合
			catch (...)
			{
				// 読み込み失敗
				return false;
			}
			// 読み込んだjsonを保存
			json = std::move(jsonTemp);

			// 読み込み成功
			return true;
		}


		float JsonConverter::ToFloat(const nlohmann::json& json)
		{
			return json.get<float>();
		}


		std::string JsonConverter::ToString(const nlohmann::json& json)
		{
			return json.get<std::string>();
		}


		Vector3 JsonConverter::ToVector3(const nlohmann::json& json)
		{
			return Vector3(
				ToFloat(json[0]),
				ToFloat(json[1]),
				ToFloat(json[2])
			);
		}
	}
}
