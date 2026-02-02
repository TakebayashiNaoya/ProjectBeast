#pragma once




namespace app
{
	namespace core
	{
		/*
		 *@brief トランスフォームクラス
		 */
		class Transform
		{
		public:
			/** 座標 */
			Vector3 m_position;

			/** 回転 */
			Quaternion m_rotation;

			/** スケール */
			Vector3 m_scale;


		public:
			Transform();
			~Transform() = default;
		};
	}
}

