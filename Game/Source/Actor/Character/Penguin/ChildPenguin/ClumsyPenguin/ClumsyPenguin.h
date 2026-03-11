/**
 * @file ClumsyPenguin.h
 * @brief おっちょこちょいペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ClumsyPenguinStateMachine;


		/**
		 * @brief おっちょこちょいペンギンクラス
		 */
		class ClumsyPenguin : public ChildPenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline ClumsyPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }


		public:
			ClumsyPenguin();
			virtual ~ClumsyPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<ClumsyPenguinStateMachine> m_stateMachine;
		};
	}
}

