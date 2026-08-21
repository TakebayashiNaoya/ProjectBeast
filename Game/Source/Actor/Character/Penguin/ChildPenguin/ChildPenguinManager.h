/**
 * @file ChildPenguinManager.h
 * @brief 子ペンギンのマネージャー
 * @author 立山、竹林
 */
#pragma once
#include "ChildPenguinTypes.h"
#include "Source/Actor/Character/Penguin/Formation/FormationController.h"
#include "Source/Actor/Character/Penguin/Formation/FormationRangeVisualizer.h"
#include "Source/Util/Curve.h"
#include <random>
#include <unordered_set>
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class DaddyPenguin;
		class Enemy;
		class FormationDebugMonitor;


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
			 * @param groundRayStartY 地面の高さを調べるレイの発射高度（ステージの地形最大高さを安全に超える値）
			 */
			void CreateChildPenguins(
				int seriousNum,
				int clingyNum,
				int naughtyNum,
				int clumsyNum,
				int caringNum,
				float spawnRadius,
				float groundRayStartY
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

			/**
			 * @brief フィーバータイム用：上空から1匹だけ子ペンギンを降らせる
			 * @param dropHeight 地面からの投下高度オフセット
			 * @detail タイプはステージ設定（CreateChildPenguinsで渡された比率）に従って重み付き抽選する
			 */
			void SpawnFromSky(float dropHeight);


		private:
			/**
			 * @brief 1体生成してタイプと座標をセットする
			 * @param type        生成するタイプ
			 * @param spawnRadius スポーン範囲の半径
			 * @param clusterCenter 群れのセンターのポジション
			 * @param clusterRadius 群れの半径
			 */
			void SpawnOne(EnChildPenguinType type, float spawnRadius, const Vector3& clusterCenter = Vector3::Zero, float clusterRadius = CLUSTER_RADIUS);

			/**
			 * @brief 群れのメンバー用の座標を計算する関数
			 * @param center センターのポジション
			 * @param clusterRadius 群れの半径
			 */
			Vector3 GenerateClusterMemberPosition(const Vector3& center, float clusterRadius);

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

			/**
			 * @brief 子ペンギンを1体生成し、タイプ・座標をセットして開始する（SpawnOne/SpawnFromSky共通処理）
			 * @param type     生成するタイプ
			 * @param spawnPos 配置する座標
			 * @return 生成した子ペンギンのポインタ
			 */
			ChildPenguin* PlaceChildPenguin(EnChildPenguinType type, const Vector3& spawnPos);


		private:
			/** 子ペンギンのリスト */
			std::vector<actor::ChildPenguin*> m_childPenguinList;
			/** 削除待ちのペンギンを入れるリスト */
			std::vector<ChildPenguin*> m_destroyList;

			/** ステージの生成半径（CreateChildPenguinsでキャッシュ、フィーバー時のランダム配置に再利用） */
			float m_spawnRadius = 3000.0f;
			/** 地面の高さを調べるレイの発射高度（CreateChildPenguinsでキャッシュ、ステージごとの地形最大高さに合わせる） */
			float m_groundRayStartY = 3000.0f;
			/** タイプ別の初期スポーン数（CreateChildPenguinsでキャッシュ、フィーバー時の比率抽選に再利用） */
			int m_seriousNum = 0, m_clingyNum = 0, m_naughtyNum = 0, m_clumsyNum = 0, m_caringNum = 0;
			/** タイプ別の初期スポーン比率で重み付けした抽選器（CreateChildPenguinsで一度だけ構築し、フィーバー時のタイプ抽選に再利用） */
			std::discrete_distribution<int> m_typeDist;




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
			 * @details 現在隊列（m_followers）に参加している子ペンギンの数を返す
			 * @return 救出済み子ペンギンの数
			 */
			int GetRescuedNum() const;

			/**
			 * @brief 指定した子ペンギンが現在隊列に参加しているか判定する
			 * @param penguin 判定する子ペンギンのポインタ
			 * @return 隊列中なら true
			 */
			bool IsFollower(const ChildPenguin* penguin) const;

			/**
			 * @brief 隊列に参加している甘えん坊が何匹いるか
			 */
			int GetClingyCount() const;


		private:
			/**
			 * @brief 陣形の座標を計算する（FormationController に委譲）
			 */
			void CalculateFormationPositions();

			/**
			 * @brief 隊列をソートし、各自に目標座標を割り当てる
			 */
			void SortAndAssignFollowers();

			/**
			 * @brief 次レベルの空きスロット座標を計算して m_nextLevelSlots に格納する
			 * @param center 親ペンギンの座標
			 */
			void CalculateNextLevelSlots(const Vector3& center);


		public:
			/**
			 * @brief 陣形を切り替える
			 * @param type 切り替え先の陣形
			 */
			void SwitchFormation(EnFormationType type) { m_formationController.SwitchFormation(type); }

			/**
			 * @brief 現在の陣形種別を取得する
			 */
			EnFormationType GetCurrentFormationType() const { return m_formationController.GetCurrentType(); }

			/**
			 * @brief 陣形切り替え演出（スライドUI）の最中か
			 * @details true の間はLB/RBによる再切り替え入力を無視する
			 */
			bool IsSwitchingFormation() const { return m_formationController.IsSwitchingFormation(); }

			/**
			 * @brief 陣形切り替え演出の進行度を 0.0(開始)〜1.0(完了) で返す（UI表示用）
			 */
			float GetFormationSwitchProgress() const { return m_formationController.GetSwitchProgress(); }

			/**
			 * @brief 陣形の移動速度倍率を取得する
			 * @details パッシブ倍率 × ウルト倍率の積を返す。
			 */
			float GetFormationSpeedMultiplier() const { return m_formationController.GetSpeedMultiplier(); }

			/**
			 * @brief 入隊判定半径を取得する（現在のフォロワー数に比例）
			 */
			float GetJoinRadius()  const { return m_formationController.GetJoinRadius(); }

			/**
			 * @brief 現在の陣形が渦潮耐性を持つか（パッシブ OR ウルト免疫）
			 */
			bool HasWhirlpoolResistance() const { return m_formationController.HasWhirlpoolResistance(); }


			//============================================//
			// ウルト操作
			//============================================//

			/**
			 * @brief ウルトを発動する（入力ハンドラから呼ぶ）
			 */
			void ActivateUlt();

			/**
			 * @brief ウルト発動中か
			 */
			bool IsUltActive() const { return m_formationController.IsUltActive(); }

			/**
			 * @brief ウルトが発動可能か
			 */
			bool CanActivateUlt() const { return m_formationController.CanActivateUlt(); }

			/**
			 * @brief ウルトのクールダウン残量を 0.0〜1.0 で返す（UI表示用）
			 */
			float GetUltCooldownRate() const { return m_formationController.GetUltCooldownRate(); }

			/**
			 * @brief ウルト発動中の残り時間割合を 0.0〜1.0 で返す（UI表示用）
			 * @return 発動直後は1.0、終了間際は0.0。発動中でなければ0.0
			 */
			float GetUltActiveRemainingRate() const { return m_formationController.GetUltActiveRemainingRate(); }

			/**
			 * @brief 指定フォロワー数に対応する入隊判定半径を返す
			 * @param count フォロワー数
			 */
			float GetJoinRadius(int count) const { return m_formationController.GetJoinRadius(count); }

			/**
			 * @brief 指定ペンギンが渦潮の捕獲を免れるか
			 * @details 密集陣かつフォロワーである場合に true を返す
			 * @param penguin 判定するペンギン
			 */
			bool IsWhirlpoolImmune(const ChildPenguin* penguin) const
			{
				return HasWhirlpoolResistance() && IsFollower(penguin);
			}

			/**
			 * @brief 陣形レベルを取得する
			 * @return 陣形レベル
			 */
			int GetFormationLevel() const { return m_formationController.GetFormationLevel(); }

			/**
			 * @brief 次のレベルアップに必要なフォロワー数を取得する
			 * @details レベルが上がるほど必要人数も増える（Lv1→2は9人、Lv2→3は18人...）
			 * @return 次のレベルアップに必要なフォロワー数
			 */
			int GetFormationNextLevelRequirement() const { return m_formationController.GetCurrentRingRequirement(); }

			/**
			 * @brief 現在のレベル内で集めたフォロワー数を取得する
			 * @return レベル内の進行フォロワー数（0 〜 GetFormationNextLevelRequirement()-1）
			 */
			int GetFormationLevelProgress() const { return m_formationController.GetCurrentRingProgress(); }

			/**
			 * @brief 陣形の最外半径を取得する（CalculatePositions後に有効）
			 * @return 最外半径
			 */
			float GetOuterRadius() const { return m_formationController.GetOuterRadius(); }

		private:
			/** 親ペンギンのポインタ（GameSceneなどで設定される） */
			DaddyPenguin* m_daddyPenguin = nullptr;

			/** 現在、親に追従している子ペンギンのリスト（隊列） */
			std::vector<ChildPenguin*> m_followers;

			/** 計算された陣形の目標座標 */
			std::vector<Vector3> m_formationPositions;

			/** 次レベルの空きスロット座標（ビジュアライザー向け） */
			std::vector<Vector3> m_nextLevelSlots;

			/** 陣形コントローラー */
			FormationController m_formationController;

			/** 陣形範囲ビジュアライザー */
			FormationRangeVisualizer m_rangeVisualizer;

#if defined(_DEBUG) || defined(K2_DEBUG)
			/** 陣形の状態をImGuiで監視するデバッグ用クラス */
			std::unique_ptr<FormationDebugMonitor> m_formationDebugMonitor;
#endif




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
			// サウンド：近傍ペンギンの可聴管理
			//============================================//

		public:
			/**
			 * @brief 指定した子ペンギンが可聴対象かどうかを返す
			 * @details DaddyPenguinに近い上位 AUDIBLE_PENGUIN_NUM 匹のみ true を返す
			 * @param penguin 確認する子ペンギン
			 * @return 可聴対象なら true
			 */
			bool IsAudible(const ChildPenguin* penguin) const;


		private:
			/**
			 * @brief 毎フレーム呼び出し、DaddyPenguinに近い順で
			 *        上位 AUDIBLE_PENGUIN_NUM 匹を m_audiblePenguins に格納する
			 */
			void UpdateAudiblePenguins();

			/** DaddyPenguinに近い順の上位 N 匹（可聴対象） */
			std::unordered_set<ChildPenguin*> m_audiblePenguins;

			/** 可聴対象とする子ペンギンの最大数 */
			static constexpr int AUDIBLE_PENGUIN_NUM = 5;

			/** UpdateAudiblePenguins 用の距離キャッシュ（毎フレームのヒープ確保を避けるため） */
			std::vector<std::pair<float, ChildPenguin*>> m_audibleDistCache;




			//============================================//
			// 親の察知（子ペンギンの視界・音による入隊判定に使う）
			//============================================//

		public:
			/**
			 * @brief 親ペンギンが出している音が届く距離を取得する
			 * @details 親が何をしているか（待機・歩き・走り・滑り・泳ぎ）で変わる。
			 *          子ペンギンはこの距離の内側にいれば、向きも遮蔽も関係なく親に気づく。
			 *          値は NoiseManager が子ペンギンの足音に使っている距離に合わせてある。
			 *
			 *          NoiseManager 経由にしていないのは、NoiseManager の音はシロクマも
			 *          聞いてしまうため。親の足音でシロクマが起きるようになると
			 *          エネミー側のバランスが変わり、ストリーム B の担当範囲を超える。
			 * @return 音が届く距離。親が止まっていれば 0
			 */
			float GetDaddyNoiseRadius() const
			{
				return m_daddyNoiseRadius;
			}

			/**
			 * @brief 遮蔽レイキャストを分散させるためのフレーム番号を取得する
			 * @details 子100体が同じフレームに一斉にレイキャストしないよう、
			 *          子ごとに割り当てたスロットとこの値で実行フレームをずらす。
			 * @return Update() のたびに1増えるカウンター
			 */
			unsigned int GetPerceptionFrame() const
			{
				return m_perceptionFrame;
			}

			/**
			 * @brief 遮蔽レイキャストのスロットを1つ払い出す
			 * @details 子ペンギンのAIコントローラーが生成時に1回だけ呼ぶ。
			 * @return 0 から始まる通し番号
			 */
			unsigned int IssuePerceptionSlot()
			{
				return m_nextPerceptionSlot++;
			}


		private:
			/**
			 * @brief 親ペンギンの出す音の届く距離を毎フレーム更新する
			 */
			void UpdateDaddyNoiseRadius();

			/** 親ペンギンの音が届く距離（毎フレーム更新） */
			float m_daddyNoiseRadius = 0.0f;
			/** 遮蔽レイキャストの分散に使うフレームカウンター */
			unsigned int m_perceptionFrame = 0;
			/** 次に払い出す遮蔽レイキャストのスロット番号 */
			unsigned int m_nextPerceptionSlot = 0;




			//============================================//
			// 逃走：シロクマの脅威と、親による再集合の呼びかけ
			//============================================//

		public:
			/**
			 * @brief 指定座標から一定距離内で、いま獲物を追っているシロクマの座標を返す
			 * @details 「自分を追っているクマ」ではなく「誰かを追っているクマ」を見る。
			 *          クマが群れに突っ込んだとき、狙われた1体だけでなく
			 *          周りの子もまとめて逃げるようにするため
			 *          （これが無いと「群れごと失う」谷が出ない）。
			 *
			 *          脅威リストは毎フレーム1回だけ作り直すので、
			 *          子ペンギン100体がこれを呼んでもエネミー側を走査し直すことはない。
			 * @param from   基準座標
			 * @param radius 探索半径
			 * @param outPos 最も近い脅威の座標（出力）
			 * @return 見つかればtrue
			 */
			bool FindNearestBearThreat(const Vector3& from, float radius, Vector3& outPos) const;

			/**
			 * @brief 親が再集合を呼びかける（Yボタン）
			 * @details パニックで散った子の逃走を打ち切り、親に気づかせる。
			 *          まだ近くにクマがいる子は戻らない（クマの目の前へ呼び戻さないため）。
			 */
			void CallRegroup();

			/**
			 * @brief 再集合の呼びかけが効いている最中かどうか
			 * @return 効いている間はtrue
			 */
			bool IsRegroupCallActive() const
			{
				return m_regroupCallTimer > 0.0f;
			}

			/**
			 * @brief 再集合を呼びかけられる状態かどうか（クールダウン中でないか）
			 * @return 呼びかけられるならtrue
			 */
			bool CanCallRegroup() const
			{
				return m_regroupCallCooldown <= 0.0f;
			}

			/**
			 * @brief 再集合の呼びかけが届く距離を取得する
			 * @return 届く距離
			 */
			float GetRegroupCallRadius() const;

			/**
			 * @brief 再集合の呼びかけのクールダウンの残り割合を取得する（UI表示用）
			 * @return 0.0(明けた)〜1.0(入ったばかり)の割合
			 */
			float GetRegroupCallCooldownRatio() const;


		private:
			/**
			 * @brief いま獲物を追っているシロクマの座標を毎フレーム集め直す
			 */
			void UpdateBearThreats();

			/**
			 * @brief 再集合の呼びかけのタイマーを毎フレーム進める
			 */
			void UpdateRegroupCall();

			/** いま獲物を追っているシロクマの座標（毎フレーム更新） */
			std::vector<Vector3> m_bearThreats;
			/** 再集合の呼びかけが効いている残り時間 */
			float m_regroupCallTimer = 0.0f;
			/** 再集合の呼びかけのクールダウン残り時間 */
			float m_regroupCallCooldown = 0.0f;




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
			 * @brief 転倒・スリップ中のペンギンを解除する
			 * @param penguin 解除するペンギン
			 */
			void UnregisterDowning(ChildPenguin* penguin);

			/**
			 * @brief 転倒・スリップ中かどうか
			 * @param penguin 確認するペンギン
			 * @return 転倒中なら true
			 */
			bool IsDowning(const ChildPenguin* penguin) const;

			/**
			 * @brief 待機命令中に追従しようとしている甘えん坊を登録する
			 */
			void RegisterAttempting(ChildPenguin* penguin);

			/**
			 * @brief 待機命令中に追従しようとしている甘えん坊を解除する
			 */
			void UnregisterAttempting(ChildPenguin* penguin);

			/**
			 * @brief 追従しようとしているかどうか
			 */
			bool IsAttempting(const ChildPenguin* penguin) const;

			/**
			 * @brief 徘徊中のやんちゃペンギンを登録する
			 */
			void RegisterRoaming(ChildPenguin* penguin);

			/**
			 * @brief 徘徊中のやんちゃペンギンを解除する
			 */
			void UnregisterRoaming(ChildPenguin* penguin);

			/**
			 * @brief 徘徊中かどうか
			 */
			bool IsRoaming(const ChildPenguin* penguin) const;

			/**
			 * @brief 世話焼きが担当しているペンギンを登録する
			 */
			void RegisterAssigned(ChildPenguin* penguin);

			/**
			 * @brief 世話焼きが担当しているペンギンを解除する
			 */
			void UnregisterAssigned(ChildPenguin* penguin);

			/**
			 * @brief 世話焼きが担当しているペンギンの集合を取得する
			 */
			const std::unordered_set<ChildPenguin*>& GetAssignedTargets() const
			{
				return m_assignedTargets;
			}

			/**
			 * @brief 最も近い転倒中のペンギンを取得する
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
			// ゴーストペンギン関連
			//============================================//

		private:
			/**
			 * @brief ゴーストペンギンの情報構造体
			 */
			struct GhostPenguinInfo
			{
				ModelRender modelRender;
				util::FloatCurve floatCurve;
				util::FloatCurve alphaCurve;

				Enemy* target;
				float timer;
				bool isHidden;
				bool isFadingOut;
				bool isDebuffActive;
				Vector3 position;

				EffectHandle handle;

				GhostPenguinInfo();
			};


		public:
			using GhostPenguins = std::vector<std::unique_ptr<GhostPenguinInfo>>;

		public:
			/**
			 * @brief 幽霊ペンギンが非表示状態かどうかを取得
			 * @return 非表示ならtrue
			 */
			bool IsGhostHidden() const { return m_isGhostHidden; }

			/**
			 * @brief ゴーストペンギンを登録する
			 * @param penguin 登録するペンギンのポインタ
			 * @param target ゴーストペンギンのデバフ対象となるシロクマ
			 */
			void RegisterGhostPenguin(ChildPenguin* penguin, Enemy* target);

			/**
			 * @brief ゴーストペンギンを更新する
			 */
			void UpdateGhostPenguins();

			/**
			 * @brief ゴーストペンギンを描画する
			 */
			void RenderGhostPenguins(RenderContext& rc);

			/**
			 * @brief ゴーストペンギンの数を取得する
			 * @return ゴーストペンギンの数
			 */
			uint8_t GetGhostPenguinNum() const { return m_ghostPenguinNum; }

			/**
			 * @brief ゴーストペンギンのリストを取得する
			 * @return ゴーストペンギンのリスト
			 */
			const GhostPenguins& GetGhostPenguins() const { return m_ghostPenguins; }


		private:
			/** ゴーストペンギンの数 */
			uint8_t m_ghostPenguinNum;
			/** ゴーストペンギンのリスト */
			GhostPenguins m_ghostPenguins;
			/** ゴーストペンギンが非表示かどうか */
			bool m_isGhostHidden;
			/** ゴーストペンギンのタイマー */
			float m_ghostTimer;
			/** シロクマ */
			Enemy* m_target;


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


		public:
			/** かまくらイベントを開始する */
			void StartIglooEvent(const Vector3& interactPos);
			// ★ 追加：かまくらイベントを終了して外に出る
			void EndIglooEvent(const Vector3& exitPos);
			/** 子ペンギンが1匹かまくらに入り終わった時の報告を受け取る */
			void FinishEnterIglooOne();
			/** 全員がかまくらに入り終わったか確認する */
			bool IsIglooEventFinished() const;


		private:
			ChildPenguinManager();
			~ChildPenguinManager();


		private:
			/** シングルトンインスタンス */
			static ChildPenguinManager* m_instance;


			/** かまくらに入ろうとしている子ペンギンの残り数 */
			int m_iglooEnteringCount = 0;
			/** ログ用IDの連番カウンタ */
			int m_nextLogId = 0;

			/** 群れの半径 */
			static constexpr float CLUSTER_RADIUS = 150.0f;
		};
	}
}