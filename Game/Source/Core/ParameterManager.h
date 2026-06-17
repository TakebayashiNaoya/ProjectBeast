/**
 * @file ParameterManager.h
 * @brief パラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Core/AppParameterMacro.h"
#include "Source/Util/JsonConverter.h"
#include <fstream>


namespace app
{
	namespace core
	{
		/** 前方宣言 */
		class IMasterParameter;

		/**
		 * パラメーター管理クラス
		 */
		class ParameterManager : public Noncopyable
		{
		public:
			/**
			 * パラメーターを更新
			 */
			void Update();


		public:
			/**
			 * @brief パラメーター読み込み
			 * @tparam T パラメーター型
			 * @param path ファイルパス
			 * @param func JSONからパラメーター型に変換する関数
			 */
			template <typename T>
			void LoadParameter(const char* path, const std::function<void(const nlohmann::json& json, T& p)>& func)
			{

				nlohmann::json jsonRoot;

				if (!util::JsonConverter::IsLoadJsonFile(jsonRoot, path)) return;

				ParameterVector parameters;
				for (auto& j : jsonRoot) {
					auto parameter = new T;

#ifdef APP_PARAM_HOT_RELOAD
					parameter->m_path = std::string(path);
					parameter->m_lastWriteTime = util::JsonConverter::GetFileLastWriteTime(path);
					parameter->load = func;
#endif // APP_PARAM_HOT_RELOAD

					func(j, *parameter);
					parameters.push_back(parameter);
				}

				m_parameterMap.emplace(T::ID(), parameters);
			}


			/**
			 * @brief パラメーター読み込み（バイナリ）
			 * @tparam T パラメーター型
			 * @param path ファイルパス
			 */
			template <typename T>
			void LoadParameterBinary(const char* path)
			{
				std::ifstream ifs(path, std::ios::binary);
				if (!ifs.is_open()) return;

				uint32_t count = 0;
				ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
				if (ifs.fail()) return;

				constexpr size_t BASE_SIZE = sizeof(core::IMasterParameter);
				constexpr size_t DATA_SIZE = sizeof(T) - BASE_SIZE;

				ParameterVector parameters;
				for (uint32_t i = 0; i < count; ++i) {
					auto* parameter = new T;

#ifdef APP_PARAM_HOT_RELOAD
					parameter->m_path = std::string(path);
					parameter->m_lastWriteTime = util::JsonConverter::GetFileLastWriteTime(path);
#endif
					// vptrやホットリロード用の変数（std::string等）を上書きしないように、
					// ベースクラスのサイズ分だけポインタを進めた位置からデータを丸呑みする
					char* dataHead = reinterpret_cast<char*>(parameter) + BASE_SIZE;
					ifs.read(dataHead, DATA_SIZE);

					parameters.push_back(parameter);
				}

				m_parameterMap.emplace(T::ID(), parameters);

				//printf("DATA_SIZE = %zu\n", DATA_SIZE);
			}


			/**
			 * @brief パラメーターアンロード
			 * @tparam T パラメーター型
			 */
			template <typename T>
			void UnloadParameter()
			{
				// 確保済みのパラメーターがあれば解放。
				auto it = m_parameterMap.find(T::ID());
				if (it != m_parameterMap.end()) {
					auto& parameters = it->second;
					for (auto* p : parameters) {
						delete p;
					}
					m_parameterMap.erase(it);
				}
			}


		public:
			/**
			 * @brief パラメーターを１つ取得する
			 * @tparam T パラメーター型
			 */
			template <typename T>
			const T* GetParameter(const int index = 0) const
			{
				const auto parameters = GetParameters<T>();
				if (parameters.size() == 0) { return nullptr; }
				if (parameters.size() <= index) { return nullptr; }
				return parameters[index];
			}
			/**
			 * @brief パラメーターを全て取得する
			 * @tparam T パラメーター型
			 */
			template <typename T>
			inline const std::vector<T*> GetParameters() const
			{
				std::vector<T*> parameters;
				auto it = m_parameterMap.find(T::ID());
				if (it != m_parameterMap.end()) {
					for (const auto& parameter : it->second) {
						parameters.push_back(static_cast<T*>(parameter));
					}
				}
				return parameters;
			}
			/**
			 * @brief パラメーターを全て処理する
			 */
			template <typename T>
			void ForEach(std::function<void(const T&)> func) const
			{
				const std::vector<T*> parameters = GetParameters<T>();
				for (const T* paramter : parameters) {
					func(*paramter);
				}
			}


		public:
			/**
			 * @brief インスタンスを生成
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new ParameterManager();
				}
			}

			/**
			 * @brief インスタンスを取得
			 * @return シングルトンインスタンスのポインタ
			 */
			static ParameterManager* Get()
			{
				return m_instance;
			}


			/**
			 * @brief インスタンスを破棄
			 */
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


		private:

			using ParameterVector = std::vector<IMasterParameter*>;

			using ParameterMap = std::map<uint32_t, ParameterVector>;


		private:
			ParameterManager();
			~ParameterManager();


			/** パラメーターマップ */
			ParameterMap m_parameterMap;


			/** シングルトンインスタンス */
			static ParameterManager* m_instance;
		};
	}
}