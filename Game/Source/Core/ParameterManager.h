/**
 * ParameterManager.h
 * パラメーター管理
 * ステータスなどの数値を外部ファイルから読み込んで使用する
 */
#pragma once
#include "json/json.hpp"
#include "Source/Util/CRC32.h"
#include <fstream>
#include <iostream>

 /**
  * NOTE: すべてのパラメーターに付ける
  */
#ifdef K2_DEBUG
#define APP_PARAM_HOT_RELOAD
#endif


  /**
  * NOTE: すべてのパラメーターに付ける
  */
#ifdef APP_PARAM_HOT_RELOAD

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}\
std::function<void(const nlohmann::json& j, name& p)> load;

#else

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}

#endif


  /** 基底クラス。必ず継承すること！ */
struct IMasterParameter
{
#ifdef APP_PARAM_HOT_RELOAD
	virtual void Load(const nlohmann::json& j) {};
	std::string m_path;         // パラメーターのファイルパス（ホットリロード用）
	time_t m_lastWriteTime;     // 最終更新時刻
#endif // APP_PARAM_HOT_RELOAD
};




/*********************************************/


/**
 * @brief キャラクターパラメーター
 * @details キャラクターの基本的なパラメーターを保持する
 */
struct MasterCharacterParameter : public IMasterParameter
{
	appParameter(MasterCharacterParameter);

#ifdef APP_PARAM_HOT_RELOAD
	void Load(const nlohmann::json& j) override
	{
		load(j, *this);
	}
#endif // APP_PARAM_HOT_RELOAD


	/** 最大体力 */
	int maxHp;
	/** 初期体力 */
	int hp;
	/** 歩行速度 */
	float walkSpeed;
	/** 走行速度 */
	float runSpeed;
	/** 体の半径 */
	float radius;
	/** 体の高さ */
	float height;
};




/*********************************************/


/**
 * @brief プレイヤーパラメーター
 * @details プレイヤー固有のパラメーターを保持する
 */
struct MasterPlayerParameter : public MasterCharacterParameter
{
	appParameter(MasterPlayerParameter);

#ifdef APP_PARAM_HOT_RELOAD
	void Load(const nlohmann::json& j) override
	{
		load(j, *this);
	}
#endif // APP_PARAM_HOT_RELOAD

	// プレイヤー固有のパラメーターをここに追加していく
};




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
	/**
	 * パラメーター操作
	 */
	void Update()
	{
#ifdef APP_PARAM_HOT_RELOAD
		for (auto paramPair : m_parameterMap)
		{
			for (auto param : paramPair.second)
			{
				if (CheckFileModified(param))
				{
					std::ifstream file(param->m_path);
					if (!file.is_open())
					{
						return;
					}

					nlohmann::json jsonRoot;
					file >> jsonRoot;

					ParameterVector parameters;

					for (const auto& j : jsonRoot)
					{
						param->m_lastWriteTime = GetFileLastWriteTime(param->m_path.c_str());
						param->Load(j);
					}
				}
			}
		}
#endif
	}



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

#ifdef APP_PARAM_HOT_RELOAD
			parameter->m_path = std::string(path);
			parameter->m_lastWriteTime = GetFileLastWriteTime(path);
			parameter->load = func;
#endif

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


#ifdef APP_PARAM_HOT_RELOAD
	/**
	 * ファイル更新日時取得
	 */
	static time_t GetFileLastWriteTime(const char* path)
	{
		struct stat result;
		// stat関数でファイル情報を取得 (0なら成功)
		if (stat(path, &result) == 0) {
			return result.st_mtime;
		}
		return 0;
	}


	/**
	 * ファイル更新チェック
	 */
	static bool CheckFileModified(const IMasterParameter* param)
	{
		if (GetFileLastWriteTime(param->m_path.c_str()) > param->m_lastWriteTime)
		{
			return true;
		}
		return false;
	}

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