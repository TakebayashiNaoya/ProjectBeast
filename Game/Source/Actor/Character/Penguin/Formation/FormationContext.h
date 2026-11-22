/**
 * @file FormationContext.h
 * @brief 陣形の効果と演出に渡すコンテキスト
 */
#pragma once


namespace app
{
	namespace actor
	{
		class ChildPenguinManager;
		class DaddyPenguin;


		/** @brief 陣形の効果と演出が参照するゲームコンテキスト */
		struct FormationContext
		{
			ChildPenguinManager* penguinManager = nullptr;
			DaddyPenguin*        daddyPenguin   = nullptr;

			/**
			 * @brief 陣形の最外半径（FormationController::GetOuterRadius()の値）
			 * @details 演出（IFormationVisual）が陣形の大きさに合わせてエフェクトの
			 *          スケールを算出するために使う。コンテキスト構築側が毎回詰めること。
			 *          フォロワー0体のときは0になるため、演出側で最小値にクランプする。
			 */
			float formationRadius = 0.0f;
		};
	}
}
