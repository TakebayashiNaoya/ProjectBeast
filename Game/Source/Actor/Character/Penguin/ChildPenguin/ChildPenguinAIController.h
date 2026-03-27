/**
 * @file ChildPenguinAIController.h
 * @brief 子ペンギンのAIコントローラー
 * @author 藤谷
 */
#pragma once
#include "ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStateMachine;


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
			 *          m_joinDistance と差を設けることで、離脱後に少し戻るだけで
			 *          すぐ追従を再開するような挙動を防ぐ。
			 */
			float m_giveUpDistance;


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

			/**
			 * @brief ヒステリシス幅
			 * @details フェーズを下げるとき、閾値からさらにこの距離だけ内側に入って初めて下げる。
			 *          m_stopDistance より小さい値にすること。
			 */
			static constexpr float HYSTERESIS = 5.0f;
		};


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


		/**
		 * @brief 甘えん坊タイプの子ペンギンAI
		 * @details 待機命令を無視して常にDaddyに追従する
		 */
		class ClingyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

		public:
			ClingyChildPenguinAI(ChildPenguin* owner);
			~ClingyChildPenguinAI() override = default;

		private:
			/** 待機命令中に強制追従が始まる親との距離 */
			float m_breakAwayDistance;
		};


		/**
		 * @brief やんちゃタイプの子ペンギンAI
		 * @details 追従命令→ついてくる、待機命令→その場待機
		 */
		class NaughtyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

		public:
			NaughtyChildPenguinAI(ChildPenguin* owner);
			~NaughtyChildPenguinAI() override = default;
		};


		/**
		 * @brief おっちょこちょいタイプの子ペンギンAI
		 * @details 追従命令→ついてくる、待機命令→その場待機
		 */
		class ClumsyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

		public:
			ClumsyChildPenguinAI(ChildPenguin* owner);
			~ClumsyChildPenguinAI() override = default;
		};


		/**
		 * @brief 世話焼きタイプの子ペンギンAI
		 * @details 追従命令→ついてくる、待機命令→その場待機
		 */
		class CaringChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void Update() override;

		public:
			CaringChildPenguinAI(ChildPenguin* owner);
			~CaringChildPenguinAI() override = default;
		};
	}
}