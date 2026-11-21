/**
 * @file PenguinSlideRule.h
 * @brief ペンギンの自動スライドの判定規則
 */
#pragma once


namespace app
{
	namespace actor
	{
		/** 自動スライドの判定（親と子で共用する） */
		namespace slideRule
		{
			/** スライドを始める下り傾斜（sinθ。約3.4度） */
			constexpr float SLOPE_ENTER = 0.06f;
			/** スライドを続ける下り傾斜（sinθ。約1.1度） */
			constexpr float SLOPE_KEEP = 0.02f;
			/** 速さが通常の移動速度のこの倍率まで落ちたら立つ */
			constexpr float COAST_SPEED_MARGIN = 1.05f;
			/** これより急な上りではゆっくり減速しない（sinθ） */
			constexpr float COAST_SLOPE_MIN = -0.06f;


			/** 判定結果 */
			enum class EnDecision
			{
				/** 立つ（滑らない） */
				Stand,
				/** 坂で加速しながら滑る */
				Drive,
				/** 勢いだけで滑る */
				Coast,
			};


			/** 滑るかどうかを決める（速度はスティックの倒し量と陣形倍率を掛けた値で渡す） */
			constexpr EnDecision Decide(
				const bool isSliding, const bool hasMoveInput, const bool canStart,
				const float slope, const float slopeSpeed, const float standingSpeed, const float currentSpeed)
			{
				if (!hasMoveInput) return EnDecision::Stand;

				// ダッシュの方が速い坂では滑らない
				const bool isSlopeFaster = slopeSpeed > standingSpeed;

				if (!isSliding)
				{
					return (canStart && slope > SLOPE_ENTER && isSlopeFaster)
						? EnDecision::Drive
						: EnDecision::Stand;
				}

				if (slope > SLOPE_KEEP && isSlopeFaster) return EnDecision::Drive;

				// 坂が終わっても、速さが残っているうちは滑り続ける
				if (currentSpeed > standingSpeed * COAST_SPEED_MARGIN) return EnDecision::Coast;

				return EnDecision::Stand;
			}

			/** ゆっくり減速させる傾斜か（急な上り以外） */
			constexpr bool UsesCoast(const float slope)
			{
				return slope >= COAST_SLOPE_MIN;
			}


			// 判定のテスト（親のslideSpeed200・runSpeed130を想定）
			static_assert(Decide(false, true, true, 0.10f, 260.0f, 130.0f, 130.0f) == EnDecision::Drive, "slide: start on downhill");
			static_assert(Decide(false, true, true, 0.04f, 224.0f, 130.0f, 130.0f) == EnDecision::Stand, "slide: no start on gentle slope");
			static_assert(Decide(false, true, true, 0.10f, 120.0f, 130.0f, 130.0f) == EnDecision::Stand, "slide: no start when slower than dash");
			static_assert(Decide(true, true, true, 0.03f, 218.0f, 130.0f, 250.0f) == EnDecision::Drive, "slide: keep on slope");
			static_assert(Decide(true, true, true, 0.00f, 200.0f, 130.0f, 250.0f) == EnDecision::Coast, "slide: coast on flat");
			static_assert(Decide(true, true, true, 0.00f, 200.0f, 130.0f, 136.0f) == EnDecision::Stand, "slide: stand when momentum is gone");
			static_assert(Decide(true, true, true, -0.20f, 60.0f, 130.0f, 250.0f) == EnDecision::Coast, "slide: coast uphill with momentum");
			static_assert(Decide(true, false, true, 0.20f, 300.0f, 130.0f, 300.0f) == EnDecision::Stand, "slide: stand without input");
		}
	}
}
