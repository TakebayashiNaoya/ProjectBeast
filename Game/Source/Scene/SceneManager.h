/**
 * @file SceneManager.h
 * @brief シーンの管理をするクラス
 * @author 立山
 */
#pragma once
#include"Source/Scene/IScene.h"
#include<functional>
#include<map>


namespace app
{
	/**
	 * @brief シーンの管理をするクラス
	 */
	class SceneManager
	{
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


	public:
		/**
		 * @brief Pauseフラグの設定
		 * @details BeastEngineのポーズフラグにも反映し、GameObjectManager/EffectEngineの更新を連動させる
		 * @param isPause Pauseフラグ
		 */
		void SetPause(const bool isPause);


		/**
		 * @brief Pauseフラグの取得
		 */
		bool IsPause() const
		{
			return m_isPause;
		}


	private:
		/** シーン遷移の状態 */
		enum class TransitionState
		{
			Idle,           // 通常のゲームプレイ
			FadingOut,      // FadeOut 進行中（画面が暗くなっている）
			LoadingScene,   // 画面が暗い状態でリソースロード中
			FadingIn,       // FadeIn 進行中（ロード画面が消えている）
		};

		/** 次のシーンID */
		uint32_t m_nextSceneId = INVALID_SCENE_ID;
		/** 遷移状態 */
		TransitionState m_transitionState = TransitionState::Idle;
		/** フェードの持続時間 */
		float m_fadeDuration = 0.0f;


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
		std::map<uint32_t, std::function<app::IScene* ()>>m_sceneMap;
		/** 現在のシーン */
		app::IScene* m_currentScene;


	private:
		/** シングルトンインスタンス */
		static SceneManager* m_instance;


	private:
		bool m_isPause = false;
	};
}