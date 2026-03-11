/**
 * @file ChildPenguin.h
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguinStateMachine;


		/**
		 * @brief 親ペンギンクラス
		 */
		class ChildPenguin : public PenguinBase
		{
		public:
			/**
			 * @brief 子ペンギンのタイプ
			 */
			enum class EnChildPenguinType : uint8_t
			{
				// 世話焼き
				Caring = 0,
				// おっちょこちょい
				Clumsy,
				// 甘えん坊
				Spoiled,
				// しっかり者
				Serious,
			};


		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline ChildPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }


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
		};
	}
}

