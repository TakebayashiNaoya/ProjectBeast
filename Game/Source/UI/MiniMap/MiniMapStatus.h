/**
 * @file MiniMapStatus.h
 * @biref MiniMapStatusクラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ミニマップのステータス
		 */
		class MiniMapStatus : public UIStatus
		{
		public:
			MiniMapStatus();
			~MiniMapStatus() override;

			void SetUpUI() override;

			void Update() override;

			/** ゲッター群 */
			float GetRadius() const { return m_radius; }
			float GetLimitDistance() const { return m_limitDistance; }
			const Vector3& GetMapCenterPos() const { return m_mapCenterPos; }


		private:
			float m_radius;
			float m_limitDistance;
			Vector3 m_mapCenterPos;
		};
	}
}


