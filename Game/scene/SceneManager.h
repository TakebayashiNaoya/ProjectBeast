/**
 * SceneManager.h
 * シーンの管理をするクラス
 */
#pragma once
#include"Scene/IScene.h"
#include<functional>
#include<map>


 /**
  * ゲーム中にnewするオブジェクト
  */
class SceneManagerObject : public IGameObject
{
public:
	SceneManagerObject();
	~SceneManagerObject();

	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc);
};




/**************************/


/**
 * シーンの管理をするクラス
 */
class SceneManager
{

	friend class SceneManagerObject;


public:
	void Update();
	void Render(RenderContext& rc);


public:
	/**
	 * シングルトンインスタンスを生成
	 */
	static void CreateInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new SceneManager();
		}
	}


	/**
	 * シングルトンインスタンスを取得
	 */
	static SceneManager* GetInstance()
	{
		return m_instance;
	}


	/**
	 * シングルトンインスタンスを破棄
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
	 * SceneMapにシーンを追加するテンプレート関数
	 * 追加する場合は、SceneManager.cppのコンストラクタで呼び出す
	 */
	template<typename T>
	void AddSceneMap()
	{
		m_sceneMap.emplace(T::ID(), []()
			{
				return new T();
			});
	}


	/**
	 * シーンを生成する関数
	 */
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

