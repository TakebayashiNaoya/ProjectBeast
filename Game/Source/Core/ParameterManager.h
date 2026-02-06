/**
 * @file ParameterManager.h
 * @brief パラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Json/json.hpp"
#include "Source/Core/AppParameterMacro.h"


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
			// 複数パラメーターがあっても良いように
			using ParameterVector = std::vector<std::unique_ptr<IMasterParameter>>;
			// 各パラメーターごとに保持する
			using ParameterMap = std::map<uint32_t, ParameterVector>;

		private:
			ParameterMap m_parameterMap;  // パラメーターを保持

		private:
			ParameterManager();
			~ParameterManager() = default;


		public:
			/**
			 * パラメーター操作
			 */
			void Update();


		public:
			/// <summary>
			/// パラメーター読み込み
			/// NOTE: Unloadも呼ぶことを忘れないように
			///       第2引数のラムダ式でテンプレートで指定する型の情報に変換する
			/// </summary>
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
				for (const auto& j : jsonRoot) {
					T* parameter = std::make_unique<T>();

#ifdef APP_PARAM_HOT_RELOAD
					parameter->m_path = std::string(path);
					parameter->m_lastWriteTime = GetFileLastWriteTime(path);
					parameter->load = func;
#endif

					func(j, *parameter);
					parameters.emplace_back(std::move(parameter));
				}

				m_parameterMap.emplace(T::ID(), std::move(parameters));
			}

			/// <summary>
			/// パラメーター解放
			/// </summary>
			template <typename T>
			void UnloadParameter()
			{
				m_parameterMap.erase(T::ID());
			}

			/// <summary>
			/// 1つだけパラメーターを取得する
			/// </summary>
			template <typename T>
			const T* GetParameter(const int index = 0) const
			{
				const auto parameters = GetParameters<T>();
				if (parameters.size() == 0) { return nullptr; }
				if (parameters.size() <= index) { return nullptr; }
				return parameters[index];
			}
			/// <summary>
			/// 複数パラメーターを取得する
			/// </summary>
			template <typename T>
			inline const std::vector<T*> GetParameters() const
			{
				std::vector<T*> parameters;
				auto it = m_parameterMap.find(T::ID());
				if (it != m_parameterMap.end()) {
					for (const auto& parameter : it->second) {
						parameters.push_back(static_cast<T*>(parameter.get()));
					}
				}
				return parameters;
			}
			/// <summary>
			/// パラメーターをラムダ式で回すForEach
			/// </summary>
			template <typename T>
			void ForEach(std::function<void(const T&)> func) const
			{
				const std::vector<T*> parameters = GetParameters<T>();
				for (const T* paramter : parameters) {
					func(*paramter);
				}
			}


#ifdef APP_PARAM_HOT_RELOAD
			/**
			 * ファイル更新日時取得
			 */
			static time_t GetFileLastWriteTime(const char* path);


			/**
			 * ファイル更新チェック
			 */
			static bool CheckFileModified(const IMasterParameter* param);

#endif // APP_PARAM_HOT_RELOAD


			/**
			 * シングルトン用
			 */
		public:
			/// <summary>
			/// インスタンスを作る
			/// </summary>
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new ParameterManager();
				}
			}

			/// <summary>
			/// インスタンスを取得
			/// </summary>
			static ParameterManager& Get()
			{
				return *m_instance;
			}

			/// <summary>
			/// インスタンスを破棄
			/// </summary>
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}

		private:
			static ParameterManager* m_instance; //シングルトンインスタンス
		};
	}
}