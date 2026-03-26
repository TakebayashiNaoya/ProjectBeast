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
			 * @brief 子ペンギンのタイプを設定
			 * @param type 子ペンギンのタイプ
			 */
			void SetChildPenguinType(EnChildPenguinType type);


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
		};
	}
}

