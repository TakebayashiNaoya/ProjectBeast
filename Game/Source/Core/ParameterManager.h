/**
 * ParameterManager.h
 * パラメーター管理
 * ステータスなどの数値を外部ファイルから読み込んで使用する
 */
#pragma once
#include "json/json.hpp"
#include "Util/CRC32.h"
#include <fstream>
#include <iostream>

 /**
  * NOTE: すべてのパラメーターに付ける
  */
#define appParameter(name)\
public:\
 static constexpr uint32_t ID() { return Hash32(#name); }


  /** 基底クラス。必ず継承すること！ */
struct IMasterParameter {};



/** defineの使用終了 */
#undef appParameter


/**
 * パラメーター管理クラス
 */
class ParameterManager
{
private:
	// 複数パラメーターがあっても良いように
	using ParameterVector = std::vector<IMasterParameter*>;
	// 各パラメーターごとに保持する
	using ParameterMap = std::map<uint32_t, ParameterVector>;

private:
	ParameterMap m_parameterMap;  // パラメーターを保持

private:
	ParameterManager();
	~ParameterManager();

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

		std::vector<IMasterParameter*> parameters;
		for (const auto& j : jsonRoot) {
			T* parameter = new T();
			func(j, *parameter);
			parameters.push_back(static_cast<IMasterParameter*>(parameter));
		}

		m_parameterMap.emplace(T::ID(), parameters);
	}

	/// <summary>
	/// パラメーター解放
	/// </summary>
	template <typename T>
	void UnloadParameter()
	{
		auto it = m_parameterMap.find(T::ID());
		if (it != m_parameterMap.end()) {
			auto& parameters = it->second;
			for (auto* p : parameters) {
				delete p;
			}
			m_parameterMap.erase(it);
		}
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
			for (auto* parameter : it->second) {
				parameters.push_back(static_cast<T*>(parameter));
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