/**
 * @file SeriousPenguin.h
 * @brief 真面目ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class SeriousPenguinStateMachine;


		/**
		 * @brief 甘えん坊ペンギンクラス
		 */
		class SeriousPenguin : public ChildPenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline SeriousPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }


		public:
			SeriousPenguin();
			virtual ~SeriousPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<SeriousPenguinStateMachine> m_stateMachine;
		};
	}
}

