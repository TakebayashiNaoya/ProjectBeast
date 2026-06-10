/**
 * @file DangerArrowCalc.h
 * @brief edge/overhead 矢印共通の定数・計算関数
 * @details DangerArrowSystem と TutorialController の両方が使用する共通ヘッダ。
 *          定数や計算ロジックをここで一元管理することで、
 *          両システムの矢印表示が常に一致することを保証する。
 */
#pragma once

namespace app
{
	namespace ui
	{
		/** edge矢印の配置円半径（スクリーン座標px） */
		constexpr float ARROW_CIRCLE_RADIUS = 300.0f;
		/** edge矢印の円中心Yオフセット（スクリーン座標px） */
		constexpr float ARROW_CIRCLE_CENTER_Y = -80.0f;
		/** overhead矢印の上方向オフセット（スクリーン座標px） */
		constexpr float ARROW_OVERHEAD_OFFSET_Y = 50.0f;
		/** 上向きDDSをターゲット方向へ向けるZ回転オフセット（= -π/2） */
		constexpr float ARROW_ROT_OFFSET = -1.5707963f;
		/** overhead矢印の回転角（下向き = π） */
		constexpr float ARROW_OVERHEAD_ANGLE = 3.1415927f;


		/**
		 * @brief edge矢印（フラスタム外）の配置座標を計算する
		 * @param worldScreenPos ワールド→スクリーン変換後の座標
		 * @return 画面縁の円上の配置座標
		 */
		inline Vector2 CalcEdgeArrowScreenPos(const Vector2& worldScreenPos)
		{
			const float angle = atan2f(worldScreenPos.y, worldScreenPos.x);
			return Vector2(
				ARROW_CIRCLE_RADIUS * cosf(angle),
				ARROW_CIRCLE_CENTER_Y + ARROW_CIRCLE_RADIUS * sinf(angle)
			);
		}

		/**
		 * @brief edge矢印の回転角を計算する
		 * @param worldScreenPos ワールド→スクリーン変換後の座標
		 * @return Z回転角（ラジアン）
		 */
		inline float CalcEdgeArrowAngle(const Vector2& worldScreenPos)
		{
			return atan2f(worldScreenPos.y, worldScreenPos.x) + ARROW_ROT_OFFSET;
		}

		/**
		 * @brief overhead矢印（フラスタム内）の配置座標を計算する
		 * @param worldScreenPos ワールド→スクリーン変換後の座標
		 * @return ターゲット真上の配置座標
		 */
		inline Vector2 CalcOverheadArrowScreenPos(const Vector2& worldScreenPos)
		{
			return Vector2(worldScreenPos.x, worldScreenPos.y + ARROW_OVERHEAD_OFFSET_Y);
		}
	}
}
