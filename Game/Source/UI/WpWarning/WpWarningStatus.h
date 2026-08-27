/**
 * @file WpWarningStatus.h
 * @brief WpWarningのステータス
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"


namespace app
{
	namespace ui
	{
		class WpWarningStatus : public UIStatus
		{
		public:
			WpWarningStatus();
			~WpWarningStatus() override;


		public:
			/** @brief UIのロジック初期化処理 */
			void SetUp() override final;
			/** @brief UIの更新処理 */
			void Update() override final;


		public:
			/** @brief アイコンのY方向オフセットを取得 */
			float GetIconOffsetY() const { return m_offsetY; }


		private:
			/** オフセットY */
			float m_offsetY;
		};
	}
}


