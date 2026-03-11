/**
 * @file SpoiledPenguin.h
 * @brief 甘えん坊ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class SpoiledPenguinStateMachine;


		/**
		 * @brief 甘えん坊ペンギンクラス
		 */
		class SpoiledPenguin : public ChildPenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline SpoiledPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }


		public:
			SpoiledPenguin();
			virtual ~SpoiledPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<SpoiledPenguinStateMachine> m_stateMachine;
		};
	}
}

