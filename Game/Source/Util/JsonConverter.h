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
			/** 無効なbool値 */
			static const bool InvalidBool;
			/** 無効なint値 */
			static const int InvalidInt;
			/** 無効なuint32_t値 */
			static const uint32_t InvalidUInt32;
			/** 無効なfloat値 */
			static const float InvalidFloat;
			/** 無効なstring値 */
			static const std::string InvalidString;
			/** 無効なVector2値 */
			static const Vector2 InvalidVector2;
			/** 無効なVector3値 */
			static const Vector3 InvalidVector3;
			/** 無効なVector4値 */
			static const Vector4 InvalidVector4;
			/** 無効なFloatRange値 */
			static const FloatRange InvalidFloatRange;



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
			 * @brief jsonからboolを読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだbool
			 */
			static bool ToBool(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからintを読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだint
			 */
			static int ToInt(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからuint32_tを読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだuint32_t
			 */
			static uint32_t ToUInt32(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからfloatを読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだfloat
			 */
			static float ToFloat(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからstringを読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだstring
			 */
			static std::string ToString(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからVector2を読み込む
			 * @param json読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだVector2
			 */
			static Vector2 ToVector2(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからVector3を読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @return 読み込んだVector3
			 */
			static Vector3 ToVector3(const nlohmann::json& json, const char* key);

			/**
			 * @brief jsonからVector3を読み込む
			 * @param json 読み込むjsonファイル
			 * @return 読み込んだVector3
			 */
			static Vector3 ToVector3(const nlohmann::json& json);

			/**
			 * @brief jsonからVector4を読み込む
			 * @param json 読み込むjsonファイル
			 * @param key 読み込むキー
			 * @param isConvert 0-255の値を0.0-1.0に変換するかどうか
			 * @return 読み込んだVector4
			 */
			static Vector4 ToVector4(const nlohmann::json& json, const char* key, bool isConvert = true);

			/**
			 * @brief jsonからFloatRange(Min, Max)を読み込む
			 * @param json 読み込むjsonファイル(配列を想定)
			 * @param key 読み込むキー
			 * @return 読み込んだFloatRange
			 */
			static FloatRange ToFloatRange(const nlohmann::json& json, const char* key);


		private:
			// インスタンス化を禁止
			JsonConverter() = delete;
			~JsonConverter() = delete;
		};
	}
}