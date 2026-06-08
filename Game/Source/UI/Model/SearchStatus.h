/**
 * @file SearchStatus.h
 * @brief SearchStatusクラス
 * @author 忽那
 */
#pragma once
#include "UIStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief PB追跡・索敵のUI用ステータスを管理するクラス
		 */
		class SearchStatus : public UIStatus
		{
		public:
			SearchStatus();
			~SearchStatus() override;

			void SetUp() override;
			void Update() override;

			/** ゲッター群 */
			float GetOffsetValueY()const { return m_offsetValueY; }
			float GetDotValue()const { return m_dotValue; }
			float GetIconPosX()const { return m_iconPosX; }
			float GetIconPosZ()const { return m_iconPosZ; }
			const Vector3& GetOffsetA()const { return m_offsetA; }
			const Vector3& GetOffsetB()const { return m_offsetB; }


		private:
			float m_offsetValueY;
			float m_dotValue;
			float m_iconPosX;
			float m_iconPosZ;
			Vector3 m_offsetA;
			Vector3 m_offsetB;
		};
	}
}


