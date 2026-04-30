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
		class SearchStatus : public UIStatus
		{
		public:
			SearchStatus();
			~SearchStatus()override;

			void SetUpUI()override;
			void Update()override;

			/** ゲッター群 */
			float GetOffsetValueY()const { return m_offsetValueY; }
			float GetDotValue()const { return m_dotValue; }
			float GetIconPosY()const { return m_iconPosY; }
			float GetIconPosX()const { return m_iconPosX; }
			float GetIconPosZ()const { return m_iconPosZ; }
			Vector3 GetOffsetA()const { return m_offsetA; }
			Vector3 GetOffsetB()const { return m_offsetB; }


		private:
			float m_offsetValueY;
			float m_dotValue;
			float m_iconPosY;
			float m_iconPosX;
			float m_iconPosZ;
			Vector3 m_offsetA;
			Vector3 m_offsetB;
		};
	}
}


