/**
 * @file JsonLoadingInfomation.cpp
 * @brief jsonファイルの読み込みに使用する
 * @author 藤谷
 */
#include "stdafx.h"
#include "JsonConverter.h"
#include <fstream>

#define ERROR_ASSERT assert(false, "キーが無効です。");

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


		bool JsonConverter::ToBool(const nlohmann::json& json, const char* key)
		{
			if (json.contains(key) && json[key].is_boolean())
			{
				return json[key].get<bool>();
			}
			ERROR_ASSERT;
			return INVALID_BOOL;
		}


		int JsonConverter::ToInt(const nlohmann::json& json, const char* key)
		{
			if (json.contains(key) && json[key].is_number_integer())
			{
				return json[key].get<int>();
			}
			ERROR_ASSERT;
			return INVALID_INT;
		}


		uint32_t JsonConverter::ToUInt32(const nlohmann::json& json, const char* key)
		{
			if (json.contains(key) && json[key].is_number_unsigned())
			{
				return json[key].get<uint32_t>();
			}
			ERROR_ASSERT;
			return INVALID_UINT32;
		}


		float JsonConverter::ToFloat(const nlohmann::json& json, const char* key)
		{
			if (json.contains(key) && json[key].is_number())
			{
				return json[key].get<float>();
			}
			ERROR_ASSERT;
			return INVALID_FLOAT;
		}


		std::string JsonConverter::ToString(const nlohmann::json& json, const char* key)
		{
			if (json.contains(key) && json[key].is_string())
			{
				return json[key].get<std::string>();
			}
			ERROR_ASSERT;
			return INVALID_STRING;
		}


		Vector2 JsonConverter::ToVector2(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key) || !json[key].is_array() || json[key].size() != 2)
			{
				ERROR_ASSERT;
				return INVALID_VECTOR2;
			}

			const auto& array = json[key];

			for (int i = 0; i < 2; i++)
			{
				if (!array[i].is_number())
				{
					ERROR_ASSERT;
					return INVALID_VECTOR2;
				}
			}

			return Vector2(
				array[0].get<float>(),
				array[1].get<float>()
			);
		}


		Vector3 JsonConverter::ToVector3(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key) || !json[key].is_array() || json[key].size() != 3)
			{
				ERROR_ASSERT;
				return INVALID_VECTOR3;
			}

			const auto& array = json[key];

			for (int i = 0; i < 3; i++)
			{
				if (!array[i].is_number())
				{
					ERROR_ASSERT;
					return INVALID_VECTOR3;
				}
			}

			return Vector3(
				array[0].get<float>(),
				array[1].get<float>(),
				array[2].get<float>()
			);
		}


		Vector3 JsonConverter::ToVector3(const nlohmann::json& json)
		{
			if (!json.is_array() || json.size() != 3)
			{
				ERROR_ASSERT;
				return INVALID_VECTOR3;
			}

			for (int i = 0; i < 3; i++)
			{
				if (!json[i].is_number())
				{
					ERROR_ASSERT;
					return INVALID_VECTOR3;
				}
			}

			return Vector3(
				json[0].get<float>(),
				json[1].get<float>(),
				json[2].get<float>()
			);
		}


		Vector4 JsonConverter::ToVector4(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key) || !json[key].is_array() || json[key].size() != 4)
			{
				ERROR_ASSERT;
				return INVALID_VECTOR4;
			}

			const auto& array = json[key];

			for (int i = 0; i < 4; i++)
			{
				if (!array[i].is_number())
				{
					ERROR_ASSERT;
					return INVALID_VECTOR4;
				}
			}

			return Vector4(
				array[0].get<float>(),
				array[1].get<float>(),
				array[2].get<float>(),
				array[3].get<float>()
			);
		}


		FloatRange JsonConverter::ToFloatRange(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key) || !json[key].is_array() || json[key].size() != 2)
			{
				ERROR_ASSERT;
				return INVALID_FLOAT_RANGE;
			}

			const auto& array = json[key];

			for (int i = 0; i < 2; i++)
			{
				if (!array[i].is_number())
				{
					ERROR_ASSERT;
					return INVALID_FLOAT_RANGE;
				}
			}

			return FloatRange{
				array[0].get<float>(),
				array[1].get<float>()
			};
		}
	}
