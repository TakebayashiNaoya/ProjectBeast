/**
 * @file UltContext.h
 * @brief ウルト発動・更新時に渡すコンテキスト
 * @author 竹林
 */
#pragma once


namespace app
{
	namespace actor
	{
		class ChildPenguinManager;
		class DaddyPenguin;


		/** @brief ウルトデコレーターが参照するゲームコンテキスト */
		struct UltContext
		{
			ChildPenguinManager* penguinManager = nullptr;
			DaddyPenguin*        daddyPenguin   = nullptr;
		};
	}
}
