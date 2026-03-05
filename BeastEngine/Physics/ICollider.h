/**
 * @file ICollider.h
 * @brief コライダーのインターフェースクラス
 * @author 竹林尚哉
 */
#pragma once


 /** 前方宣言 */
class btCollisionShape;


namespace nsBeastEngine
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
