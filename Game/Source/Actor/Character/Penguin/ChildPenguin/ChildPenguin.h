/**
 * @file ChildPenguinBase.h
 * @brief 子ペンギンの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinBase.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンの基底クラス
		 */
		class ChildPenguinBase : public PenguinBase
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


		protected:
			virtual void Start() override;
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		protected:
			/**
			 * @brief 初期化
			 * @param childPenguinType 子ペンギンの種類
			 */
			void Init(const EnChildPenguinType childPenguinType);


		public:
			ChildPenguinBase();
			virtual ~ChildPenguinBase() override = default;


		protected:
			/** 子ペンギンの種類 */
			EnChildPenguinType m_childPenguinType;
		};
	}
}

