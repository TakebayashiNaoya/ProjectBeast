/**
 * @file SceneManager.h
 * @brief シーンの管理をするクラス
 * @author 立山
 */
#pragma once
#include"Source/Scene/IScene.h"
#include<functional>
#include<map>


 /**
  * @brief ゲーム中にnewするオブジェクト
  */
class SceneManagerObject : public IGameObject
{
public:
	SceneManagerObject();
	~SceneManagerObject();

	/**
	 * @brief 最初のシーンを設定
	 * @detail Start()にSceneManager::GetInstance()->CreateScene(シーン名::ID());
	 */
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc);
};




/**************************/


/**
 * @brief シーンの管理をするクラス
 */
class SceneManager
{
	/** friend宣言 */
	friend class SceneManagerObject;


public:
	void Update();
	void Render(RenderContext& rc);


public:
	/**
	 * @brief シングルトンインスタンスを生成
	 */
	static void CreateInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new SceneManager();
		}
	}


	/**
	 * @brief シングルトンインスタンスを取得
	 * @return シングルトンインスタンスのポインタ
	 */
	static SceneManager* GetInstance()
	{
		return m_instance;
	}


	/**
	 * @brief シングルトンインスタンスを破棄
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
	/** 次のシーンID */
	uint32_t m_nextSceneId = INVALID_SCENE_ID;


private:
	SceneManager();
	~SceneManager();


private:
	/**
	 * @brief SceneMapにシーンを追加するテンプレート関数
	 * @detail 追加する場合は、SceneManagerのコンストラクタで呼び出す
	 * @return  T型のオブジェクトをnewで生成しポインタを返す
	 */
	template<typename T>
	void AddSceneMap()
	{
		m_sceneMap.emplace(T::ID(), []()
			{
				return new T();
			});
	}


	/** シーンを生成する関数 */
	void CreateScene(const uint32_t id);


private:
	/** シーンマップ */
	std::map<uint32_t, std::unique_ptr<IScene>>m_sceneMap;
	/** 現在のシーン */
	IScene* m_currentScene;


private:
	/** シングルトンインスタンス */
	static SceneManager* m_instance;
};

