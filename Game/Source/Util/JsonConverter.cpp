/**
 * @file JsonLoadingInfomation.cpp
 * @brief jsonファイルの読み込みに使用する
 * @author 藤谷
 */
#include "stdafx.h"
#include "JsonConverter.h"

#include "Curve.h"

#include <fstream>

#define NO_CONTAINS K2_ASSERT(false, "キーが存在しません。");
#define VALUE_DIFFER K2_ASSERT(false, "値が無効です。");

namespace app
{
	namespace util
	{
		const bool JsonConverter::InvalidBool = false;
		const int JsonConverter::InvalidInt = -1;
		const uint32_t JsonConverter::InvalidUInt32 = 0;
		const float JsonConverter::InvalidFloat = FLT_MAX;
		const std::string JsonConverter::InvalidString = "";
		const Vector2 JsonConverter::InvalidVector2 = Vector2(FLT_MAX, FLT_MAX);
		const Vector3 JsonConverter::InvalidVector3 = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		const Vector4 JsonConverter::InvalidVector4 = Vector4(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
		const FloatRange JsonConverter::InvalidFloatRange = FloatRange{ FLT_MAX, FLT_MAX };


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
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_boolean()) VALUE_DIFFER;

			bool result = json.contains(key) && json[key].is_boolean();

			if (result) return json[key].get<bool>();

			return InvalidBool;
		}


		int JsonConverter::ToInt(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_number_integer()) VALUE_DIFFER;

			bool result = json.contains(key) && json[key].is_number_integer();

			if (result) return json[key].get<int>();

			return InvalidInt;
		}


		uint32_t JsonConverter::ToUInt32(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_number_unsigned()) VALUE_DIFFER;

			bool result = json.contains(key) && json[key].is_number_unsigned();

			if (result) return json[key].get<uint32_t>();

			return InvalidUInt32;
		}


		float JsonConverter::ToFloat(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_number()) VALUE_DIFFER;

			bool result = json.contains(key) && json[key].is_number();

			if (result) return json[key].get<float>();

			return InvalidFloat;
		}


		std::string JsonConverter::ToString(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_string()) VALUE_DIFFER;

			bool result = json.contains(key) && json[key].is_string();

			if (result) return json[key].get<std::string>();

			return InvalidString;
		}


		Vector2 JsonConverter::ToVector2(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_array() || json[key].size() != 2) VALUE_DIFFER;

			const auto& array = json[key];
			bool result = json.contains(key) && json[key].is_array() && json[key].size() == 2;

			for (size_t i = 0; i < json[key].size(); i++)
			{
				if (!array[i].is_number()) VALUE_DIFFER;

				result = result && array[i].is_number();
			}

			if (result) return Vector2(
				array[0].get<float>(),
				array[1].get<float>()
			);

			return InvalidVector2;
		}


		Vector3 JsonConverter::ToVector3(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_array() || json[key].size() != 3) VALUE_DIFFER;

			const auto& array = json[key];
			bool result = json.contains(key) && json[key].is_array() && json[key].size() == 3;

			for (size_t i = 0; i < json[key].size(); i++)
			{
				if (!array[i].is_number()) VALUE_DIFFER;

				result = result && array[i].is_number();
			}

			if (result) return Vector3(
				array[0].get<float>(),
				array[1].get<float>(),
				array[2].get<float>()
			);

			return InvalidVector3;
		}


		Vector3 JsonConverter::ToVector3(const nlohmann::json& json)
		{
			if (!json.is_array() || json.size() != 3) VALUE_DIFFER;

			bool result = json.is_array() && json.size() == 3;

			for (size_t i = 0; i < json.size(); i++)
			{
				if (!json[i].is_number()) VALUE_DIFFER;

				result = result && json[i].is_number();
			}

			if (result) return Vector3(
				json[0].get<float>(),
				json[1].get<float>(),
				json[2].get<float>()
			);

			return InvalidVector3;
		}


		Vector4 JsonConverter::ToVector4(const nlohmann::json& json, const char* key, bool isConvert)
		{
			constexpr float minValue = 0.0f;
			constexpr float maxValue = 1.0f;
			constexpr float convertValue = 255.0f;

			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_array() || json[key].size() != 4) VALUE_DIFFER;

			const auto& array = json[key];

			bool result = json.contains(key) && json[key].is_array() && json[key].size() == 4;

			for (size_t i = 0; i < json[key].size(); i++)
			{
				if (!array[i].is_number()) VALUE_DIFFER;

				result = result && array[i].is_number();
			}

			if (!result) return InvalidVector4;


			Vector4 color(
				array[0].get<float>(),
				array[1].get<float>(),
				array[2].get<float>(),
				array[3].get<float>()
			);

			if (isConvert)
			{
				color.x = clamp(color.x / convertValue, minValue, maxValue);
				color.y = clamp(color.y / convertValue, minValue, maxValue);
				color.z = clamp(color.z / convertValue, minValue, maxValue);
				color.w = clamp(color.w / convertValue, minValue, maxValue);
			}

			return color;
		}


		FloatRange JsonConverter::ToFloatRange(const nlohmann::json& json, const char* key)
		{
			if (!json.contains(key)) NO_CONTAINS;
			if (!json[key].is_array() || json[key].size() != 2) VALUE_DIFFER;

			const auto& array = json[key];

			bool result = json.contains(key) && json[key].is_array() && json[key].size() == 2;

			for (size_t i = 0; i < json[key].size(); i++)
			{
				if (!array[i].is_number()) VALUE_DIFFER;

				result = result && array[i].is_number();
			}

			if (result) return FloatRange{
				array[0].get<float>(),
				array[1].get<float>()
			};

			return InvalidFloatRange;
		}
	}
}
