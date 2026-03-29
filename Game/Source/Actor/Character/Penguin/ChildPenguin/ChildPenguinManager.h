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
		class DaddyPenguin;


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
			// 陣形・追従管理
			//============================================//

		public:
			/**
			 * @brief 親ペンギンの現在座標を取得する
			 */
			Vector3 GetDaddyPosition() const;
			/**
			 * @brief 親ペンギンを設定（GameSceneなどで呼び出す）
			 * @param daddy 親ペンギンのポインタ
			 */
			void SetDaddyPenguin(DaddyPenguin* daddy) { m_daddyPenguin = daddy; }

			/**
			 * @brief 隊列（フォロー状態）に参加する
			 * @param penguin 参加する子ペンギンのポインタ
			 */
			void AddFollower(ChildPenguin* penguin);

			/**
			 * @brief 隊列から離脱する（はぐれた時など）
			 * @param penguin 離脱する子ペンギンのポインタ
			 */
			void RemoveFollower(ChildPenguin* penguin);


		private:
			/**
			 * @brief 陣形の座標を計算する
			 */
			void CalculateFormationPositions();

			/**
			 * @brief 隊列をソートし、各自に目標座標を割り当てる
			 */
			void SortAndAssignFollowers();


		private:
			/** 親ペンギンのポインタ（GameSceneなどで設定される） */
			DaddyPenguin* m_daddyPenguin = nullptr;

			/** 現在、親に追従している子ペンギンのリスト（隊列） */
			std::vector<ChildPenguin*> m_followers;

			/** 計算された陣形の目標座標（最大100個） */
			std::vector<Vector3> m_formationPositions;

			/** 陣形調整用のパラメータ */
			const int MAX_FORMATION_COUNT = 100;		/** 隊列の最大数 */
			const float FORMATION_BASE_RADIUS = 30.0f;  /** 一番内側の円の半径 */
			const float FORMATION_RADIUS_STEP = 20.0f;  /** 円が外側になるごとの増加量 */
			const float FORMATION_MIN_DISTANCE = 15.0f; /** ペンギン同士の最低間隔 */




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

			/**
			 * @brief 追従命令と待機命令を切り替える（トグル）
			 */
			void ToggleCommand()
			{
				if (m_command == EnPenguinCommand::Follow)
				{
					m_command = EnPenguinCommand::Wait;
				}
				else
				{
					m_command = EnPenguinCommand::Follow;
				}
			}


		private:
			/** 子ペンギンへの命令 */
			EnPenguinCommand m_command = EnPenguinCommand::Follow;




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