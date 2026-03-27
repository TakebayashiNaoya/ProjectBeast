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
		 * @brief 範囲を保持するための構造体
		 */
		struct FloatRange {
			float min;
			float max;
		};


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

			/** デバッグ用の関数群 */
#ifdef APP_DEBUG
			/**
			 * @brief ファイルの最終更新日時を取得
			 */
			static inline time_t GetFileLastWriteTime(const char* path)
			{
				struct stat result;
				// stat関数でファイル情報を取得 (0なら成功)
				if (stat(path, &result) == 0) {
					return result.st_mtime;
				}
				return 0;
			}
			/**
			 * @brief ファイルが更新されたかどうか
			 * @param filePath ファイルパス
			 */
			static inline bool CheckFileModified(const std::string filePath, const time_t time)
			{
				return util::JsonConverter::GetFileLastWriteTime(filePath.c_str()) > time;
			}

#endif // APP_DEBUG
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
					json[0].get<float>(),
					json[1].get<float>(),
					json[2].get<float>()
				);
			}
			/**
			 * @brief jsonからFloatRange(Min, Max)を読み込む
			 * @param json 読み込むjsonファイル(配列を想定)
			 * @return 読み込んだFloatRange
			 */
			static inline FloatRange ToFloatRange(const nlohmann::json& json)
			{
				return { json[0].get<float>(), json[1].get<float>() };
			}


		private:
			// インスタンス化を禁止
			JsonConverter() = delete;
			~JsonConverter() = delete;
		};
	}
}