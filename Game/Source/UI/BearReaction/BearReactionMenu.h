/**
 * @file BearReactionMenu.h
 * @brief クマのリアクションメニュークラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "BearReactionTypes.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief クマのリアクションメニュークラス
		 */
		class BearReactionMenu : public MenuBase
		{
		public:
			/**
			 * @brief クマのリアクションのターゲット座標を設定
			 * @param position 座標
			 */
			void SetTargetPosition(const Vector3& position);


			void SetReactionType(const EnBearReactionType reactionType);


		public:
			void InitializeLogic() override final;

			void Update() override final;


		public:
			BearReactionMenu();
			~BearReactionMenu() override;


		private:
			/** 吹き出し */
			UIIcon* m_speechBubble;
			/** 舌 */
			UIIcon* m_tongue;
			/** ベッド */
			UIIcon* m_bed;
			/** 落ち込み */
			UIIcon* m_feelingDown;
		};
	}
}


