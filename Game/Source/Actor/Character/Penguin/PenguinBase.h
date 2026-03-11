/**
 * @file PenguinBase.h
 * @brief ペンギンの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{


		/**
		 * @brief ペンギンの基底クラス
		 */
		class PenguinBase : public CharacterBase
		{
		public:
			PenguinBase() = default;
			virtual ~PenguinBase() override = default;


		protected:
			virtual void Start() override;
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;
		};
	}
}

