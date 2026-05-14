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


		namespace
		{
			// 無効な値を定義

			constexpr bool INVALID_BOOL = false;								// bool型の無効な値はfalseとする
			constexpr int INVALID_INT = INT_MAX;								// int型の無効な値はINT_MAXとする
			constexpr uint32_t INVALID_UINT32 = UINT32_MAX;						// uint32_t型の無効な値はUINT32_MAXとする
			constexpr float INVALID_FLOAT = FLT_MAX;							// float型の無効な値はFLT_MAXとする
			const std::string INVALID_STRING = std::to_string(STRING_NONE);		// string型の無効な値はSTRING_NONEを文字列化したものとする
			const Vector2 INVALID_VECTOR2(FLT_MAX, FLT_MAX);					// Vector2型の無効な値は(FLT_MAX, FLT_MAX)とする
			const Vector3 INVALID_VECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);			// Vector3型の無効な値は(FLT_MAX, FLT_MAX, FLT_MAX)とする
			const Vector4 INVALID_VECTOR4(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);	// Vector4型の無効な値は(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX)とする
			const FloatRange INVALID_FLOAT_RANGE{ FLT_MAX, FLT_MAX };			// FloatRange型の無効な値は{FLT_MAX, FLT_MAX}とする
		}


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
			 * @return 読み込んだVector4
			 */
			static Vector4 ToVector4(const nlohmann::json& json, const char* key);

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