/**
 * @file ChildPenguin.h
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinBase.h"
#include "ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguinStateMachine;
		class ChildPenguinAIController;
		class DaddyPenguin;


		/**
		 * @brief 子ペンギンクラス
		 */
		class ChildPenguin : public PenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline ChildPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }
			/**
			 * @brief 親ペンギンを設定
			 * @param daddyPenguin 親ペンギンのポインタ
			 */
			void SetDaddyPenguin(DaddyPenguin* daddyPenguin);
			/**
			 * @brief 子ペンギンのタイプを取得
			 * @return 子ペンギンのタイプ
			 */
			inline EnChildPenguinType GetChildPenguinType() const { return m_type; }
			/**
			 * @brief 子ペンギンのタイプを設定
			 * @param type 子ペンギンのタイプ
			 */
			void SetChildPenguinType(EnChildPenguinType type);
			/**
			 * @brief 目標座標を取得する（AIコントローラーが移動処理に使う）
			 * @return 目標座標
			 */
			inline const Vector3& GetFormationTargetPosition() const { return m_formationTarget; }
			/**
			 * @brief 陣形における自身の目標座標を設定する
			 * @param targetPos 目標座標
			 */
			inline void SetFormationTargetPosition(const Vector3& targetPos) { m_formationTarget = targetPos; }


		public:
			ChildPenguin();
			virtual ~ChildPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<ChildPenguinStateMachine> m_stateMachine;
			/** AIコントローラー */
			std::unique_ptr<ChildPenguinAIController> m_aiController;
			/** 親ペンギンのポインタ */
			DaddyPenguin* m_daddyPenguin = nullptr;
			/** 子ペンギンのタイプ */
			EnChildPenguinType m_type = EnChildPenguinType::Serious;
			/** 陣形における自身の目標座標 */
			Vector3 m_formationTarget;
		};
	}
}

