/**
 * @file MiniMapStatus.h
 * @biref MiniMapStatusクラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"

#include "MiniMapTypes.h"


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

			void SetUp() override;

			void Update() override;

			/** ゲッター群 */
			float GetRadius() const { return m_radius; }
			float GetLimitDistance() const { return m_limitDistance; }
			const Vector3& GetMapCenterPos() const { return m_mapCenterPos; }
			const MiniMapInitializeInfo& GetIconInitializeInfos() const { return m_iconInitializeInfos; }
			const Vector3& GetInitPosition() const { return m_initPosition; }
			const Vector3& GetInitScale() const { return m_initScale; }
			const Quaternion& GetInitRotation() const { return m_initRotation; }
			const Vector4& GetInitColor() const { return m_initColor; }


		private:
			float m_radius;
			float m_limitDistance;
			Vector3 m_mapCenterPos;
			MiniMapInitializeInfo m_iconInitializeInfos;
			Vector3 m_initPosition;
			Vector3 m_initScale;
			Quaternion m_initRotation;
			Vector4 m_initColor;
		};
	}
}


