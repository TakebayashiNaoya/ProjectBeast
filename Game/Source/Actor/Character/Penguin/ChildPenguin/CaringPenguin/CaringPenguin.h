/**
 * @file CaringPenguin.h
 * @brief 世話焼きペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class CaringPenguinStateMachine;


		/**
		 * @brief 世話焼きペンギンクラス
		 */
		class CaringPenguin : public ChildPenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline CaringPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }


		public:
			CaringPenguin();
			virtual ~CaringPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<CaringPenguinStateMachine> m_stateMachine;
		};
	}
}

