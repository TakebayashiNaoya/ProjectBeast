/**
 * @file Whirlpool.h
 * @brief 渦潮のクラス
 * @author 藤谷
 */
#pragma once
#include "IStage.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 渦潮のクラス
		 */
		class Whirlpool : public IStageObject
		{
		public:
			void Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			Whirlpool() = default;
			virtual ~Whirlpool() override = default;


		private:

		};
	}
}

