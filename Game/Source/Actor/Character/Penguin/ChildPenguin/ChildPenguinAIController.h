/**
 * @file ChildPenguinAIController.h
 * @brief 子ペンギンのAIコントローラー
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStateMachine;
		class DaddyPenguin;


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
			ChildPenguinAIController(ChildPenguin* owner, DaddyPenguin* daddyPenguin);
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


		protected:
			/** 子ペンギンのポインタ */
			ChildPenguin* m_owner;
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_daddyPenguin;
			/** ステートマシンへの参照 */
			ChildPenguinStateMachine* m_stateMachine;
		};


		/**
		 * @brief まじめタイプの子ペンギンAI
		 * 追従命令→ついてくる、待機命令→その場待機
		 */
		class SeriousChildPenguinAI : public ChildPenguinAIController
		{
		public:
			/**
			 * @brief 更新処理
			 */
			void Update() override;


		public:
			SeriousChildPenguinAI(ChildPenguin* owner, DaddyPenguin* daddyPenguin);
			~SeriousChildPenguinAI() override = default;


		private:
			/** 追従時の目標距離 */
			static constexpr float FOLLOW_DISTANCE = 150.0f;
			/** ダッシュ開始距離 */
			static constexpr float DASH_DISTANCE = 300.0f;
		};


		/**
		 * @brief 甘えん坊タイプの子ペンギンAI
		 * 待機命令を無視して常にDaddyに追従
		 */
		class ClingyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			/**
			 * @brief 更新処理
			 */
			void Update() override;


		public:
			ClingyChildPenguinAI(ChildPenguin* owner, DaddyPenguin* daddyPenguin);
			~ClingyChildPenguinAI() override = default;


		private:
			/** 追従時の目標距離（より近い） */
			static constexpr float FOLLOW_DISTANCE = 100.0f;
			/** ダッシュ開始距離 */
			static constexpr float DASH_DISTANCE = 250.0f;
			/** 待機命令中に強制追従が始まる距離 */
			static constexpr float BREAK_AWAY_DISTANCE = 400.0f;
		};
	}
}
