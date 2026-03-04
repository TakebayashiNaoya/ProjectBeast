/**
 * @brief コライダーのインターフェースクラス
 */
#pragma once


 /** 前方宣言 */
class btCollisionShape;


namespace nsK2EngineLow
{
	/**
	 * @brief コライダーのインターフェースクラス
	 */
	class ICollider : public Noncopyable
	{
	public:
		/**
		 * @brief コライダーを取得
		 */
		virtual btCollisionShape* GetBody() const = 0;
	};
}
