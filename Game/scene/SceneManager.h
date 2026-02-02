#pragma once
#include"Scene/IScene.h"
#include<functional>
#include<map>

//シーン管理を処理するためのゲームオブジェクト
class SceneManagerObject : public IGameObject
{
public:
	SceneManagerObject();
	~SceneManagerObject();

	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc);
};

/*
 *シーン管理クラス
 */
class SceneManager
{

	friend class SceneManagerObject;

private:
	//次のシーンID
	uint32_t m_nextSceneId = INVALID_SCENE_ID;

	//経過時間
	float m_elapsedTime = 0.0f;
	float m_waitTime = 0.0f;

private:
	SceneManager();
	~SceneManager();

public:
	void Update();
	void Render(RenderContext& rc);

private:
	template<typename T>
	void AddSceneMap()
	{
		m_sceneMap.emplace(T::ID(), []()
			{
				return new T();
			});
	}

	void CreateScene(const uint32_t id);

private:
	using SceneMap = std::map<uint32_t, std::function<IScene* ()>>;
	SceneMap m_sceneMap;
	IScene* m_currentScene;

public:

	static void CreateInstance()
	{
		if (m_instance == nullptr)
		{
			m_instance = new SceneManager();
		}
	}

	static SceneManager* GetInstance()
	{
		return m_instance;
	}

	static void DestroyInstance()
	{
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}
private:
	static SceneManager* m_instance;
};

