/**
 * @file CollisionAttr.h
 * @brief コリジョン属性
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	namespace nsCollision
	{
		/**
		 * @brief コリジョン属性の大分類
		 */
		enum EnCollisionAttr
		{
			enCollisionAttr_Ground,
			enCollisionAttr_Character,
			enCollisionAttr_User,		/** 以下にユーザー定義のコリジョン属性を設定する */
			enCollisionAttr_Wall
		};
	}
}