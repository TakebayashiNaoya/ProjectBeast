/**
 * @file ChildPenguinAIController.h
 * @brief 子ペンギンのAIコントローラー
 * @author 藤谷、竹林
 */
#pragma once
#include "ChildPenguinTypes.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStateMachine;
		class ClumsyChildPenguinStateMachine;
		class NaughtyChildPenguinStateMachine;


		/**
		 * @brief 子ペンギンのAIコントローラー基底クラス
		 */
		class ChildPenguinAIController
		{
		public:
			/**
			 * @brief 更新処理
			 */
			virtual void Update() = 0;


		public:
			ChildPenguinAIController(ChildPenguin* owner, EnChildPenguinType type);
			virtual ~ChildPenguinAIController() = default;


		public:
			/**
			 * @brief 隊列に加わる距離を取得
			 * @return 隊列に加わる距離
			 */
			float GetJoinDistance() const { return m_joinDistance; }


			/**
			 * @brief かまくらに入るイベントを開始する
			 * @param targetPos 向かうべき入り口の座標（青い円）
			 */
			void StartEnterIglooEvent(const Vector3& targetPos)
			{
				m_iglooTargetPos = targetPos;
				m_isEnterIglooMode = true;
				m_isInsideIgloo = false;
			}


			// ★ 追加：イベント中かどうかを取得する関数
			bool IsEnterIglooMode() const { return m_isEnterIglooMode; }


			/**
			 * @brief かまくらから出てくるイベントを開始する
			 * @param exitPos 出てくる座標（青い円）
			 */
			void EndEnterIglooEvent(const Vector3& exitPos);


			/**
			 * @brief かまくらが壊された時の強制排出処理（★ 追加）
			 * @param iglooPos 壊されたかまくらの中心座標
			 */
			void ForceEjectFromIgloo(const Vector3& iglooPos);


		protected:
			/**
			 * @brief DaddyPenguinへの方向ベクトルを計算
			 * @return DaddyPenguinへの正規化された方向ベクトル
			 */
			Vector3 CalculateDirectionToDaddy() const;

			/**
			 * @brief DaddyPenguinまでの距離を取得
			 * @return DaddyPenguinまでの距離
			 */
			float GetDistanceToDaddy() const;

			/**
			 * @brief 任意の目標座標への正規化された方向ベクトルを計算
			 * @param targetPos 目標座標
			 * @return 正規化された方向ベクトル
			 */
			Vector3 CalculateDirectionToTarget(const Vector3& targetPos) const;

			/**
			 * @brief 任意の目標座標までの距離を取得
			 * @param targetPos 目標座標
			 * @return 距離
			 */
			float GetDistanceToTarget(const Vector3& targetPos) const;

			/**
			 * @brief 陣形ポジションまでの距離に応じてAIControllerInputを組み立てる
			 *
			 * @details
			 * 【移動手段の切り替え（3段階）】
			 *   距離が大きいほど速い移動手段を使う。親の移動状態には依存しない。
			 *   distance <= stopDistance : 停止
			 *   distance <= walkDistance : 歩き
			 *   distance <= runDistance  : 走り
			 *   distance >  runDistance  : 滑り
			 *
			 * 【ヒステリシスによるチラつき防止】
			 *   フェーズを「上げる閾値」と「下げる閾値」を分けている。
			 *   下げる閾値 = 上げる閾値 - HYSTERESIS
			 *   これにより、閾値付近での高速な状態切り替えを防ぐ。
			 *   ※ m_stopDistance > HYSTERESIS となる値を設定すること。
			 *
			 * 【目標手前での減速（アプローチ減速）】
			 *   stopDistance の2倍以内に入ると moveDirection をスケールダウンする。
			 *   目標に近づくほど入力が弱まり行き過ぎを抑制する。
			 */
			void BuildInput();

			/**
			 * @brief 任意の目標座標へ向かう入力を組み立てる（BuildInputの座標指定版）
			 * @param targetPos 移動先の座標
			 * @note 世話焼きペンギンの介入移動などに使用する
			 */
			void BuildInputToTarget(const Vector3& targetPos);


			/** かまくらイベントの更新処理 */
			void UpdateIglooEvent();

			/**
			 * @brief シロクマ逃走チェックと移動入力設定
			 * @details 自分を追跡中のエネミーが FLEE_DETECTION_DISTANCE 以内にいれば
			 *          エネミーと反対方向へダッシュ入力を設定してtrueを返す。
			 * @return 逃走行動中ならtrue（このフレームの通常AIをスキップする）
			 */
			bool CheckAndFlee();


		protected:
			/** 子ペンギンのポインタ */
			ChildPenguin* m_owner;
			/** ステートマシンへの参照 */
			ChildPenguinStateMachine* m_stateMachine;
			/** 隊列（陣形）に参加しているかどうか */
			bool m_isFollowing = false;
			/** 停止とみなす距離（HYSTERESIS より十分大きい値にすること） */
			float m_stopDistance;
			/** 歩きの上限距離 */
			float m_walkDistance;
			/** 走りの上限距離（これを超えると滑りで追う） */
			float m_runDistance;
			/** 隊列に加わる距離（未参加→参加） */
			float m_joinDistance;
			/**
			 * @brief 追跡をあきらめてその場で待機する距離（参加中→離脱）
			 * @details m_joinDistance より大きい値にすること。
			 */
			float m_giveUpDistance;


			/** かまくらイベント中かどうか */
			bool m_isEnterIglooMode = false;
			/** すでにかまくらの中に入ったか */
			bool m_isInsideIgloo = false;
			/** かまくらの入り口（青い円）の目標座標 */
			Vector3 m_iglooTargetPos = Vector3::Zero;
			/** エフェクトハンドル */
			EffectHandle m_hartEffectHandle;


		private:
			/**
			 * @brief 移動手段の内部フェーズ
			 * @details ヒステリシス制御のために保持する
			 */
			enum class MovePhase
			{
				Stop,    ///< 停止
				Walk,    ///< 歩き（Sneak）
				Run,     ///< 走り
				Slide,   ///< 滑り
			};

			/** 現在の移動フェーズ */
			MovePhase m_movePhase = MovePhase::Stop;

			/** 逃走検知距離（この距離以内の追跡エネミーがいると逃走行動に入る） */
			static constexpr float FLEE_DETECTION_DISTANCE = 300.0f;

			/**
			 * @brief 逃走方向を切り替えるまでの残り時間（秒）
			 * @details 0以下になると次の方向が抽選される
			 */
			float m_fleeDirChangeTimer = 0.0f;

			/**
			 * @brief 逃走方向に加えるY軸回転オフセット（ラジアン）
			 * @details 0のとき直進、±値のとき左右に振れる
			 */
			float m_fleeAngleOffset = 0.0f;
		};




		/***********************************************
		 * 派生クラス
		 ***********************************************/


		 /**
		  * @brief まじめタイプの子ペンギンAI
		  * @details 追従命令→ついてくる、待機命令→その場待機
		  */
		class SeriousChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

		public:
			SeriousChildPenguinAI(ChildPenguin* owner);
			~SeriousChildPenguinAI() override = default;
		};




		/**************************************************************/


		/**
		 * @brief 甘えん坊タイプの子ペンギンAI
		 * @details 命令に関わらず常にDaddyに追従しようとする。制止されている間は待機する
		 */
		class ClingyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

			/**
			 * @brief 世話焼きペンギンによる制止フラグを設定する
			 * @param isRestrained 制止フラグ
			 */
			inline void SetRestrained(const bool isRestrained)
			{
				m_isRestrained = isRestrained;
			}

			/**
			 * @brief 制止中かどうかを取得する
			 * @return 制止中ならtrue
			 */
			inline bool IsRestrained() const
			{
				return m_isRestrained;
			}

		public:
			ClingyChildPenguinAI(ChildPenguin* owner);
			~ClingyChildPenguinAI() override = default;

		private:
			/** 世話焼きペンギンに制止されているかどうか */
			bool m_isRestrained = false;
			/** 甘えん坊専用エフェクトハンドル */
			EffectHandle m_clingyEffectHandle;
			/** エフェクトの再生時間 */
			float m_effectInterval = 0.0f;
		};




		/**************************************************************/


		/**
		 * @brief やんちゃタイプの子ペンギンAI
		 * @details 追従命令→ついてくる、待機命令かつ親が一定距離以上離れたら→徘徊する
		 */
		class NaughtyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

			/**
			 * @brief 世話焼きペンギンによる制止フラグを設定する
			 * @param isRestrained 制止フラグ
			 */
			inline void SetRestrained(const bool isRestrained)
			{
				m_isRestrained = isRestrained;
			}

			/**
			 * @brief 制止中かどうかを取得する
			 * @return 制止中ならtrue
			 */
			inline bool IsRestrained() const
			{
				return m_isRestrained;
			}

		public:
			NaughtyChildPenguinAI(ChildPenguin* owner);
			~NaughtyChildPenguinAI() override = default;

		private:
			/**
			 * @brief 次の徘徊目標座標をランダムに選ぶ
			 */
			void PickNewRoamTarget();

		private:
			/** やんちゃ固有ステートマシンへのポインタ（キャスト済みのキャッシュ） */
			NaughtyChildPenguinStateMachine* m_naughtyStateMachine = nullptr;
			/** 世話焼きペンギンに制止されているかどうか */
			bool m_isRestrained = false;
			/** 徘徊先の目標座標 */
			Vector3 m_roamTarget = Vector3::Zero;
			/** 待機命令中に徘徊を開始する親との距離 */
			float m_roamTriggerDistance = 0.0f;
			/** 徘徊先を選ぶ現在地からの半径 */
			float m_roamRadius = 0.0f;
			/** 反省時間 */
			float m_scoldCooldown = 0.0f;
			/** 渦潮に飲み込まれたかどうかのフラグ */
			bool m_wasSwallowedByWhirlpool = false;
		};




		/**************************************************************/


		/**
		 * @brief おっちょこちょいタイプの子ペンギンAI
		 * @details 歩き・走り中に確率で転倒し、スライド解除時に確率でスリップする
		 */
		class ClumsyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

			/**
			 * @brief 世話焼きペンギンに助けてもらう
			 * @note 転倒・スリップ中に呼ばれるとアニメ終了を待たず即座に起き上がる
			 */
			void HelpedByCaringPenguin();

		public:
			ClumsyChildPenguinAI(ChildPenguin* owner);
			~ClumsyChildPenguinAI() override = default;

		private:
			/** おっちょこちょい固有ステートマシンへのポインタ（キャスト済みのキャッシュ） */
			ClumsyChildPenguinStateMachine* m_clumsyStateMachine = nullptr;
			/** 歩き・走り中の転倒確率（秒あたり） */
			float m_tripChancePerSec = 0.0f;
			/** スライド解除時のスリップ確率 */
			float m_slipChance = 0.0f;
			/** 前フレームにスライド中だったかどうか（スライド解除の検出に使う） */
			bool m_wasSliding = false;
		};




		/**************************************************************/


		/**
		 * @brief 世話焼きタイプの子ペンギンAI
		 * @details 待機命令中に問題を起こしている子ペンギンに近づいて制止・助けを行う
		 */
		class CaringChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

		public:
			CaringChildPenguinAI(ChildPenguin* owner);
			~CaringChildPenguinAI() override = default;

		private:
			/**
			 * @brief 介入対象への到達判定
			 * @param target 介入対象
			 * @return 十分近ければtrue
			 */
			bool IsCloseEnoughTo(const ChildPenguin* target) const;

			/**
			 * @brief 介入対象に制止・助けの処理を適用する
			 * @param target 介入対象
			 */
			void ApplyIntervention(ChildPenguin* target) const;

			/**
			 * @brief 制止していた対象への制止を解除する
			 * @param target 制止を解除する対象
			 */
			void ReleaseSuppression(ChildPenguin* target) const;


		private:
			/**
			 * @brief 介入時にエフェクトを再生する
			 * @details 対象ペンギンの頭上に汗エフェクト
			 */
			void PlayCaringEffect() const;

			/** 現在介入中の対象ペンギン */
			ChildPenguin* m_interventionTarget = nullptr;
			/**
			 * @brief 介入対象を探す最大距離（JSONパラメーターから読み込む）
			 * @details この距離より遠いペンギンには介入しない
			 */
			float m_interventionRange = 0.0f;
			/** 介入到達とみなす距離 */
			static constexpr float INTERVENTION_REACH_DISTANCE = 25.0f;
			/** 複数回連続で流すための制御値 */
			mutable float m_sweatEffectCoolTime = 0.0f;
			/** 汗エフェクトハンドル */
			mutable EffectHandle m_caringEffectHandle;
			/** 汗エフェクトの回数制御値 */
			mutable int m_sweatEffectCount = 0;
		};
	}
}