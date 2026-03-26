/**
 * @file ChildPenguinManager.h
 * @brief 子ペンギンのマネージャー
 * @author 立山、竹林
 */
#pragma once


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;


		/**
		 * @brief 子ペンギンのマネージャークラス
		 */
		class ChildPenguinManager
		{

		public:
			void Start();
			void Update();
			void Render(RenderContext& rc);




			//============================================//
			// 子ペンギンの生成と管理
			//============================================//

		public:
			/**
			 * @brief 子ペンギンを生成
			 * @param childPenguinNum 生成する子ペンギンの数
			 */
			void CreateChildPenguin(const int childPenguinNum);

			/**
			 * @brief 子ペンギンのリストを取得
			 * @return 子ペンギンのリスト
			 */
			const std::vector<actor::ChildPenguin*>& GetChildPenguin()
			{
				return m_childPenguinList;
			}


		private:
			/** 子ペンギンのリスト */
			std::vector<actor::ChildPenguin*>m_childPenguinList;




			//============================================//
			// 追従命令と待機命令のフラグ管理
			//============================================//

		public:
			/**
			 * @brief 追従命令と待機命令の列挙型
			 */
			enum class EnPenguinCommand : uint8_t
			{
				Follow = 0,
				Wait,
				None
			};

			/**
			 * @brief 命令を取得
			 * @return 追従命令or待機命令
			 */
			EnPenguinCommand GetCommand() const
			{
				return m_command;
			}

			/**
			 * @brief 命令を設定
			 * @param command 追従命令or待機命令
			 */
			void SetCommand(const EnPenguinCommand command)
			{
				m_command = command;
			}


		private:
			/** 子ペンギンへの命令 */
			EnPenguinCommand m_command;




			//============================================//
			// シングルトン関連
			//============================================//

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
			/**
			 * @brief シングルトンインスタンスを取得
			 * @return シングルトンインスタンスのポインタ
			 */
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


		private:
			ChildPenguinManager();
			~ChildPenguinManager();


		private:
			/** シングルトンインスタンス */
			static ChildPenguinManager* m_instance;
		};
	}
}
