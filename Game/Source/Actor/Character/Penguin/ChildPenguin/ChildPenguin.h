/**
 * @file ChildPenguin.h
 * @brief 子ペンギンの基底クラス
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
		 * @brief 子ペンギンの基底クラス
		 */
		class ChildPenguin : public PenguinBase
		{
		public:
			/**
			 * @brief 子ペンギンの種類
			 */
			enum class EnChildPenguinType : uint8_t
			{
				// 世話焼き
				Caring,
				// おちょこちょい
				Clumsy,
				// 真面目
				Serious,
				// 甘えん坊
				Spoiled,
				Max
			};


		public:
			/**
			 * @brief 子ペンギンの種類を取得
			 * @return 子ペンギンの種類
			 */
			inline EnChildPenguinType GetChildPenguinType() const
			{
				return m_childPenguinType;
			}
			/**
			 * @brief 子ペンギンの種類を設定
			 * @param childPenguinType 子ペンギンの種類
			 */
			inline void SetChildPenguinType(const EnChildPenguinType childPenguinType)
			{
				m_childPenguinType = childPenguinType;
			}


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		public:
			ChildPenguin();
			virtual ~ChildPenguin() override = default;


		private:
			/** 子ペンギンの種類 */
			EnChildPenguinType m_childPenguinType;
			/** ステートマシン */
			std::unique_ptr<ChildPenguinStateMachine> m_stateMachine;
		};
	}
}

