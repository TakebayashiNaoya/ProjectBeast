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
			static int ToInt(const nlohmann::json& json);
			/**
			 * @brief jsonからuint32_tを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだuint32_t
			 */
			static uint32_t ToUInt32(const nlohmann::json& json);
			/**
			 * @brief jsonからfloatを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだfloat
			 */
			static float ToFloat(const nlohmann::json& json);
			/**
			 * @brief jsonからstringを読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだstring
			 */
			static std::string ToString(const nlohmann::json& json);
			/**
			 * @brief jsonからVector3を読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだVector3
			 */
			static Vector3 ToVector3(const nlohmann::json& json);


		private:
			// インスタンス化を禁止
			JsonConverter() = delete;
			~JsonConverter() = delete;
		};
	}
}