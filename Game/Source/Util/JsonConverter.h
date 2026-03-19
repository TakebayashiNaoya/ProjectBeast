/**
 * @file JsonLoadingInfomation.h
 * @brief jsonファイルの読み込みに使用する
 * @author 藤谷
 */
#pragma once
#include "Json/json.hpp"


namespace app
{
	namespace util
	{
		/**
		 * @brief jsonファイルの読み込みに使用する関数群
		 */
		class JsonConverter
		{
		public:
			/**
			 * @brief jsonファイルを読み込めたかどうか
			 * @param json 読み込むjsonファイル
			 * @return 読み込めたかどうか
			 */
			static bool IsLoadJsonFile(nlohmann::json& json, const std::string& filePath);
			/**
			 * @brief jsonからintを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだint
			 */
			static inline int ToInt(const nlohmann::json& json)
			{
				return json.get<int>();
			}
			/**
			 * @brief jsonからuint32_tを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだuint32_t
			 */
			static inline uint32_t ToUInt32(const nlohmann::json& json)
			{
				return json.get<uint32_t>();
			}
			/**
			 * @brief jsonからfloatを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだfloat
			 */
			static inline float ToFloat(const nlohmann::json& json)
			{
				return json.get<float>();
			}
			/**
			 * @brief jsonからstringを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだstring
			 */
			static inline std::string ToString(const nlohmann::json& json)
			{
				return json.get<std::string>();
			}
			/**
			 * @brief jsonからVector3を読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだVector3
			 */
			static inline Vector3 ToVector3(const nlohmann::json& json)
			{
				return Vector3(
					json["x"].get<float>(),
					json["y"].get<float>(),
					json["z"].get<float>()
				);
			}


		private:
			// インスタンス化を禁止
			JsonConverter() = delete;
			~JsonConverter() = delete;
		};
	}
}