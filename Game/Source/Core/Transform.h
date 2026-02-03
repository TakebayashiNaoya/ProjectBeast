/**
 * Transform.h
 * トランスフォーム定義
 */
#pragma once


namespace app
{
	namespace core
	{
		class Transform : public Noncopyable
		{
		public:
			/** 座標 */
			Vector3 m_position = Vector3::Zero;
			/** 回転 */
			Quaternion m_rotation = Quaternion::Identity;
			/** 拡大縮小 */
			Vector3 m_scale = Vector3::One;
		};
	}
}