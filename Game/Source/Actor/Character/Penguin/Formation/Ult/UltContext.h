/**
 * @file UltContext.h
 * @brief ウルト発動・更新時に渡すコンテキスト
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

			/**
			 * @brief 陣形の最外半径（FormationController::GetOuterRadius() の値）
			 * @details ウルト演出（IUltEffect）が陣形の大きさに合わせてエフェクトの
			 *          スケールを算出するために使う。コンテキスト構築側が毎回詰めること。
			 *          フォロワー0体のときは 0 になるため、演出側で最小値にクランプする。
			 */
			float formationRadius = 0.0f;
		};
	}
}
