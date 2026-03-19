/**
 * @file ChildPenguinManager.h
 * @brief 子ペンギンのマネージャー
 * @author 立山
 */
#pragma once


namespace app
{
	namespace actor
	{

		class ChildPenguin;

		class ChildPenguinManager
		{
		public:
			/**
			 * @brief シングルトンインスタンスを生成
			 * @brief GameSceneのコンストラクタで呼び出す。
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new ChildPenguinManager;
				}
			}


			static ChildPenguinManager* GetInstance()
			{
				return m_instance;
			}


			/**
			 * @brief シングルトンインスタンスを削除
			 * @brief GameSceneのデストラクタで呼び出す
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
			void Start();
			void Update();
			void Render(RenderContext& rc);

			void CreateChildPenguin(const int childPenguinNum);


		public:
			const std::vector<actor::ChildPenguin*>& GetChildPenguiin()
			{
				return m_childPenguinList;
			}


		private:
			ChildPenguinManager();
			~ChildPenguinManager();


		private:
			std::vector<actor::ChildPenguin*>m_childPenguinList;


		private:

			static ChildPenguinManager* m_instance;
		};
	}
}
