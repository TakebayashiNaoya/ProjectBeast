/**
 * @file UIAnimationParameter.h
 * @brief アニメーション情報の外部ファイル管理とホットリロード
 * @author 忽那
 */
#pragma once
#include "Json/json.hpp"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief UIアニメーションの定義構造体
		 * @param key アニメーションの識別子(ハッシュキー)
		 * @param valueType アニメーションの値の種類(float, Vector2, Vector3, Vector4)
		 * @param start 最初の値
		 * @param end 最後の値
		 * @param duration アニメーションの時間(秒)
		 * @param easingType イージングタイプ
		 * @param loopMode ループモード
		 */
		struct UIAnimationDef
		{
			uint32_t key;

			enum class ValueType { Float, Vector2, Vector3, Vector4 };
			ValueType valueType = ValueType::Float;

			// 共通パラメーター。
			float startFloat = 0.0f;
			float endFloat = 0.0f;
			Vector2 startV2 = Vector2::Zero;
			Vector2 endV2 = Vector2::Zero;
			Vector3 startV3 = Vector3::Zero;
			Vector3 endV3 = Vector3::Zero;
			Vector4 startV4 = Vector4::White;
			Vector4 endV4 = Vector4::White;

			float duration = 0.0f;
			app::util::EasingType easingType = app::util::EasingType::Linear;
			app::util::LoopMode loopMode = app::util::LoopMode::Once;
		};

		/**
		 * @brief UIアニメーション情報の管理クラス
		 * @detail JSONファイルから読み込み、ホットリロードを可能にする
		 */
		class UIAnimationParameter : public Noncopyable
		{
		private:
			std::string m_filePath;
			std::unordered_map<uint32_t, UIAnimationDef>m_defMap;

			/** ホットリロード用 */
			uint64_t m_lastModifiedTime = 0;

			/** ホットリロード監視間隔(秒) */
			float m_reloadInterval = 1.0f;
			float m_reloadTimer = 0.0f;
			/** リロード時のコールバック(UIへの再適用etc) */
			std::function<void()>m_onReloaded;


		private:
			UIAnimationParameter() {}
			~UIAnimationParameter(){}


		public:
			/**
			 * @brief シングルトン取得
			 * @return UIAnimationParameterのインスタンス
			 */
			static UIAnimationParameter& Get()
			{
				static UIAnimationParameter instance;
				return instance;
			}


			/**
			 * @brief JSONファイを読み込む
			 * @param filePath JSONファイルのパス
			 * @return 成功したか
			 */
			bool Load(const std::string& filePath)
			{
				m_filePath = filePath;
				m_lastModifiedTime = GetFileModifiedTime(filePath);
				return ParseFile();
			}


			/**
			 * @brief 毎フレーム呼ぶ(ホットリロード監視)
			 * @param deltaTime 前フレームからの経過時間(秒)
			 */
			void Update(float deltaTime)
			{
				m_reloadTimer += deltaTime;
				if (m_reloadTimer < m_reloadInterval)return;
				m_reloadTimer = 0.0f;

				uint64_t currentTime = GetFileModifiedTime(m_filePath);
				if (currentTime != m_lastModifiedTime) {
					m_lastModifiedTime = currentTime;
					if (ParseFile()) {
						if (m_onReloaded) m_onReloaded();
					}
				}
			}

			/**
			 * @brief キーでアニメーションを定義を取得
			 * @param key アニメーションのハッシュキー
			 * @return 見つからなければnullptr
			 */
			const UIAnimationDef* Find(uint32_t key)const
			{
				auto it = m_defMap.find(key);
				if (it != m_defMap.end()) {
					return &it->second;
				}
				return nullptr;
			}


			/**
			 * @brief リロード時のコールバック設定
			 * @param callback コールバック関数
			 */
			void SetOnReloaded(const std::function<void()>& callback)
			{
				m_onReloaded = callback;
			}


			/**
			 * @brief リロード監視間隔を設定
			 * @param sec 監視間隔(秒)
			 */
			void SetReloadInterval(float sec) { m_reloadInterval = sec; }


		private:
			bool ParseFile()
			{
				std::string jsonText;
				if (!ReloadFileToString(m_filePath, jsonText)){
					return false;
				}

				nlohmann::json root;
				// JSONのパースに失敗したらfalseを返す。
				try {
					root = nlohmann::json::parse(jsonText);
				}
				catch (...){
					return false;
				}

				m_defMap.clear();

				// "animations"配列を走査。
				if (!root.contains("animations") || !root["animations"].is_array())
				{
					return false;
				}

				for (const auto& item : root["animations"])
				{
					UIAnimationDef def;
					// 文字列 → ハッシュキー。
					std::string keyStr = item.value("key", "");
					if (keyStr.empty())continue;
					def.key = util::ComputeCrc32(keyStr.c_str());

					// 値の型。
					std::string valTypeStr = item.value("valueType", "float");
					def.valueType = ParseValueType(valTypeStr);

					// 開始・終了値
					ParseValues(item, def);

					// 共通パラメーター
					def.duration = item.value("duration", 0.3f);
					def.easingType = ParseEasingType(item.value("easing", "Linear"));
					def.loopMode = ParseLoopMode(item.value("loop", "Once"));

					m_defMap[def.key] = def;
				}
				return true;
			}



			// -------------------------------------------------
			// ユーティリティ関数
			// ------------------------------------------------


			static UIAnimationDef::ValueType ParseValueType(const std::string& s)
			{
				if (s == "Vector2") return UIAnimationDef::ValueType::Vector2;
				if (s == "Vector3") return UIAnimationDef::ValueType::Vector3;
				if (s == "Vector4") return UIAnimationDef::ValueType::Vector4;
				return UIAnimationDef::ValueType::Float;
			}


			static util::EasingType ParseEasingType(const std::string& s)
			{
				if (s == "EaseIn")return util::EasingType::EaseIn;
				if (s == "EaseOut")return util::EasingType::EaseOut;
				if (s == "EaseInOut")return util::EasingType::EaseInOut;
				return util::EasingType::Linear;
			}


			static util::LoopMode ParseLoopMode(const std::string& s)
			{
				if (s == "Loop")return util::LoopMode::Loop;
				if (s == "PingPong")return util::LoopMode::PingPong;
				return util::LoopMode::Once;
			}


			static void ParseValues(const nlohmann::json& item, UIAnimationDef& def)
			{
				switch (def.valueType)
				{
				case UIAnimationDef::ValueType::Float:
				{
					def.startFloat = item.value("startValue", 0.0f);
					def.endFloat = item.value("endValue", 0.0f);
					break;
				}
				case UIAnimationDef::ValueType::Vector2:
				{
					ParseVec2(item, "startValue", def.startV2);
					ParseVec2(item, "endValue", def.endV2);
					break;
				}
				case UIAnimationDef::ValueType::Vector3:
				{
					ParseVec3(item, "startValue", def.startV3);
					ParseVec3(item, "endValue", def.endV3);
					break;
				}
				case UIAnimationDef::ValueType::Vector4:
				{
					ParseVec4(item, "startValue", def.startV4);
					ParseVec4(item, "endValue", def.endV4);
					break;
				}
				}
			}


			static void ParseVec2(const nlohmann::json& parent, const char* name, Vector2& out)
			{
				if (!parent.contains(name))return;
				const auto& v = parent[name];
				out.x = v.value("x", 0.0f);
				out.y = v.value("y", 0.0f);
			}


			static void ParseVec3(const nlohmann::json& parent, const char* name, Vector3& out)
			{
				if (!parent.contains(name))return;
				const auto& v = parent[name];
				out.x = v.value("x", 0.0f);
				out.y = v.value("y", 0.0f);
				out.z = v.value("z", 0.0f);
			}


			static void ParseVec4(const nlohmann::json& parent, const char* name, Vector4& out)
			{
				if (!parent.contains(name))return;
				const auto& v = parent[name];
				out.x = v.value("x", 0.0f);
				out.y = v.value("y", 0.0f);
				out.z = v.value("z", 0.0f);
				out.w = v.value("w", 0.0f);
			}


			/**
			 * @brief ファイルを文字列として読み込む
			 * @param path ファイルのパス
			 * @para out 読み込んだファイルを格納する文字列
			 * @return 成功したか
			 */
			static bool ReloadFileToString(const std::string& path, std::string& out)
			{
				FILE* fp = fopen(path.c_str(), "rb");
				if (!fp)return false;

				// ファイルサイズを取得してバッファを確保。
				fseek(fp, 0, SEEK_END);
				long size = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				out.resize(size);
				fread(&out[0], 1, size, fp);
				fclose(fp);
				return true;
			}


			/**
			 * @brief ファイル更新時刻を取得
			 * @param path ファイルのパス
			 * @return ファイルの更新時刻(UNIXタイムスタンプ)を返す。取得できない場合は0を返す。
			 */
			static uint64_t GetFileModifiedTime(const std::string& path)
			{
#if defined(_WIN32)
				WIN32_FILE_ATTRIBUTE_DATA data;
				if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data))
				{
					ULARGE_INTEGER ull;
					ull.LowPart = data.ftLastWriteTime.dwLowDateTime;
					ull.HighPart = data.ftLastWriteTime.dwHighDateTime;
					return ull.QuadPart;
				}
#endif
				return 0;
			}
		};
	}
}
