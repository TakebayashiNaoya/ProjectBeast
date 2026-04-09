/**
 * @file ChildPenguinManager.h
 * @brief 子ペンギンのマネージャー
 * @author 立山、竹林
 */
#pragma once
#include <unordered_set>
#include "ChildPenguinTypes.h"


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


			/**
			 * @brief モデルの行列更新のみ行う（AI・ステートマシンは動かさない）
			 */
			void UpdateModelOnly();




			//============================================//
			// 子ペンギンの生成・削除と管理
			//============================================//

		public:
			/**
			 * @brief タイプ別に子ペンギンを一括生成する
			 * @param seriousNum  まじめタイプの生成数
			 * @param clingyNum   甘えん坊タイプの生成数
			 * @param naughtyNum  やんちゃタイプの生成数
			 * @param clumsyNum   おっちょこちょいタイプの生成数
			 * @param caringNum   世話焼きタイプの生成数
			 * @param spawnRadius スポーン範囲の半径
			 */
			void CreateChildPenguins(
				int seriousNum,
				int clingyNum,
				int naughtyNum,
				int clumsyNum,
				int caringNum,
				float spawnRadius
			);

			/**
			 * @brief 子ペンギンのリストを取得
			 * @return 子ペンギンのリスト
			 */
			const std::vector<actor::ChildPenguin*>& GetChildPenguin()
			{
				return m_childPenguinList;
			}

			/**
			 * @brief 子ペンギンの数を取得
			 * @return 子ペンギンの数
			 */
			int GetChildPenguinNum() const
			{
				return m_childPenguinList.size();
			}

			/**
			 * @brief 子ペンギンを隊列・リストから取り除いてdeleteする
			 * @param penguin 削除する子ペンギンのポインタ
			 * @note PenguinDeadState::Enter()経由でChildPenguinStateMachine::OnDead()から呼ばれる
			 */
			void RemoveAndDestroy(ChildPenguin* penguin);


		private:
			/**
			 * @brief 1体生成してタイプと座標をセットする
			 * @param type        生成するタイプ
			 * @param spawnRadius スポーン範囲の半径
			 */
			void SpawnOne(EnChildPenguinType type, float spawnRadius);

			/**
			 * @brief 円内のランダムなXZ座標を生成する（拒絶サンプリング）
			 * @param radius 円の半径
			 * @return 生成された座標（y=0）
			 */
			Vector3 GenerateRandomSpawnPosition(float radius);

			/**
			 * @brief 指定XZ座標から真上にレイを飛ばして地面のyを返す
			 * @param x X座標
			 * @param z Z座標
			 * @return ヒットした地面のy座標。ヒットしなければ0.0f（海面扱い）
			 */
			float GetGroundY(float x, float z);

			/**
			 * @brief 子ペンギンを生成してリストに追加する
			 * @note SpawnOne()の内部から呼び出す
			 */
			void CreateChildPenguin();


		private:
			/** 子ペンギンのリスト */
			std::vector<actor::ChildPenguin*> m_childPenguinList;
			/** 削除待ちのペンギンを入れるリスト */
			std::vector<ChildPenguin*> m_destroyList;




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

			/**
			 * @brief 整列済み子ペンギンの数を取得
			 * @return 整列済み子ペンギンの数
			 */
			int GetFollowersNum() const
			{
				return m_followers.size();
			}

			/**
			 * @brief 救出済み子ペンギンの数を取得
			 * @details コマンドに関係なく、各子ペンギンの joinDistance 以内にいる数を返す
			 * @return 救出済み子ペンギンの数
			 */
			int GetRescuedNum() const;


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
			const int   MAX_FORMATION_COUNT = 100;   /** 隊列の最大数 */
			const float FORMATION_BASE_RADIUS = 0.0f;  /** 一番内側の円の半径 */
			const float FORMATION_RADIUS_STEP = 20.0f; /** 円が外側になるごとの増加量 */
			const float FORMATION_MIN_DISTANCE = 15.0f; /** ペンギン同士の最低間隔 */




			//============================================//
			// 追従命令と待機命令のフラグ管理
			//============================================//

		public:
			/**
			 * @brief 命令と待機命令の列挙型
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
			// 世話焼き用：問題行動ペンギンの状態管理
			//============================================//

		public:
			/**
			 * @brief 転倒・スリップ中のペンギンを登録する
			 * @param penguin 登録するペンギン
			 */
			void RegisterDowning(ChildPenguin* penguin);

			/**
			 * @brief 転倒・スリップ中の登録を解除する
			 * @param penguin 解除するペンギン
			 */
			void UnregisterDowning(ChildPenguin* penguin);

			/**
			 * @brief 転倒・スリップ中かどうかを取得する
			 * @param penguin 確認するペンギン
			 * @return 転倒・スリップ中ならtrue
			 */
			bool IsDowning(const ChildPenguin* penguin) const;

			/**
			 * @brief 待機命令中に追従しようとしている甘えん坊を登録する
			 * @param penguin 登録するペンギン
			 */
			void RegisterAttempting(ChildPenguin* penguin);

			/**
			 * @brief 追従しようとしている甘えん坊の登録を解除する
			 * @param penguin 解除するペンギン
			 */
			void UnregisterAttempting(ChildPenguin* penguin);

			/**
			 * @brief 待機命令中に追従しようとしているかどうかを取得する
			 * @param penguin 確認するペンギン
			 * @return 追従しようとしているならtrue
			 */
			bool IsAttempting(const ChildPenguin* penguin) const;

			/**
			 * @brief 徘徊中のやんちゃペンギンを登録する
			 * @param penguin 登録するペンギン
			 */
			void RegisterRoaming(ChildPenguin* penguin);

			/**
			 * @brief 徘徊中のやんちゃペンギンの登録を解除する
			 * @param penguin 解除するペンギン
			 */
			void UnregisterRoaming(ChildPenguin* penguin);

			/**
			 * @brief 徘徊中かどうかを取得する
			 * @param penguin 確認するペンギン
			 * @return 徘徊中ならtrue
			 */
			bool IsRoaming(const ChildPenguin* penguin) const;

			/**
			 * @brief 現在いずれかの世話焼きペンギンが担当しているペンギンの集合を取得する
			 * @return 担当済みペンギンの集合
			 */
			const std::unordered_set<ChildPenguin*>& GetAssignedTargets() const
			{
				return m_assignedTargets;
			}

			/**
			 * @brief 世話焼きペンギンの担当ペンギンを登録する
			 * @param penguin 担当するペンギン
			 */
			void RegisterAssigned(ChildPenguin* penguin);

			/**
			 * @brief 世話焼きペンギンの担当ペンギンの登録を解除する
			 * @param penguin 解除するペンギン
			 */
			void UnregisterAssigned(ChildPenguin* penguin);

			/**
			 * @brief 最も近い転倒・スリップ中のペンギンを取得する
			 * @param from       基準座標（世話焼きペンギンの座標）
			 * @param excludeSet 既に他の世話焼きが担当しているペンギンの集合
			 * @param maxRange   探索する最大距離
			 * @return 最も近いペンギン。いなければnullptr
			 */
			ChildPenguin* FindNearestDowning(
				const Vector3& from,
				const std::unordered_set<ChildPenguin*>& excludeSet,
				float maxRange
			) const;

			/**
			 * @brief 最も近い監視が必要なペンギン（甘えん坊・やんちゃ）を取得する
			 * @param from       基準座標（世話焼きペンギンの座標）
			 * @param excludeSet 既に他の世話焼きが担当しているペンギンの集合
			 * @param maxRange   探索する最大距離
			 * @return 最も近いペンギン。いなければnullptr
			 */
			ChildPenguin* FindNearestNeedingSupervision(
				const Vector3& from,
				const std::unordered_set<ChildPenguin*>& excludeSet,
				float maxRange
			) const;


		private:
			/** 転倒・スリップ中のペンギンの集合 */
			std::unordered_set<ChildPenguin*> m_downingPenguins;
			/** 待機命令中に追従しようとしている甘えん坊の集合 */
			std::unordered_set<ChildPenguin*> m_attemptingPenguins;
			/** 徘徊中のやんちゃペンギンの集合 */
			std::unordered_set<ChildPenguin*> m_roamingPenguins;
			/** いずれかの世話焼きが担当しているペンギンの集合 */
			std::unordered_set<ChildPenguin*> m_assignedTargets;




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