/**
 * @file ParameterManager.h
 * @brief パラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Json/json.hpp"
#include "Source/Core/AppParameterMacro.h"
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
		private:

			using ParameterVector = std::vector<IMasterParameter*>;

			using ParameterMap = std::map<uint32_t, ParameterVector>;


		public:
#ifdef APP_PARAM_HOT_RELOAD
			/**
			 * @brief ファイル更新日時取得
			 */
			static time_t GetFileLastWriteTime(const char* path);


			/**
			 * @brief ファイル更新チェック
			 */
			static bool CheckFileModified(const IMasterParameter* param);

#endif // APP_PARAM_HOT_RELOAD



		private:
			ParameterManager();
			~ParameterManager();


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
				std::ifstream file(path);
				if (!file.is_open()) {
					return;
				}

				nlohmann::json jsonRoot;
				file >> jsonRoot;

				ParameterVector parameters;
				for (auto& j : jsonRoot) {
					auto parameter = new T;

#ifdef APP_PARAM_HOT_RELOAD
					parameter->m_path = std::string(path);
					parameter->m_lastWriteTime = GetFileLastWriteTime(path);
					parameter->load = func;
#endif

					func(j, *parameter);
					parameters.push_back(parameter);
				}

				m_parameterMap.emplace(T::ID(), parameters);
			}


			/**
			 * @brief パラメーターアンロード
			 * @tparam T パラメーター型
			 */
			template <typename T>
			void UnloadParameter()
			{
				m_parameterMap.erase(T::ID());
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
				if (parameters.size() < index) { return nullptr; }
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


		private:
			/** パラメーターマップ */
			ParameterMap m_parameterMap;


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
			/** シングルトンインスタンス */
			static ParameterManager* m_instance;
		};
	}
}