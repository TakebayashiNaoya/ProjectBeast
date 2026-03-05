/**
 * @file DaddyPenguin.h
 * @brief 親ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class DaddyPenguinStateMachine;


		/**
		 * @brief 親ペンギンクラス
		 */
		class DaddyPenguin : public CharacterBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline DaddyPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }


		public:
			DaddyPenguin();
			virtual ~DaddyPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<DaddyPenguinStateMachine> m_stateMachine;
		};
	}
}

