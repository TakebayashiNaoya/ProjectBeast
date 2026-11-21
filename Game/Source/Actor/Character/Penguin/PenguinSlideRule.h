/**
 * @file PenguinSlideRule.h
 * @brief ペンギンの自動スライドの判定規則
 */
#pragma once


namespace app
{
	namespace actor
	{
		/**
		 * @brief 自動スライドの判定（親ペンギンの自動スライドと子ペンギンAIで共用する）
		 * @details しきい値の定義はこのファイルだけに置く。
		 *          判定は副作用のないconstexpr関数にして、末尾のstatic_assertで真理値表を固定する。
		 */
		namespace slideRule
		{
			/**
			 * @brief スライドを始める下り傾斜（符号つき傾斜sinθ。正が下り。約3.4度）
			 */
			constexpr float SLOPE_ENTER = 0.06f;
			/**
			 * @brief 坂に運ばれる滑走を続ける下り傾斜（符号つき傾斜sinθ。約1.1度）
			 * @details 開始より緩くして、凸凹での開始・終了のばたつきを防ぐ。
			 */
			constexpr float SLOPE_KEEP = 0.02f;
			/**
			 * @brief 惰性で滑り続ける速度の余裕（立ち速度に対する倍率）
			 * @details 現在速度が「立ち速度×この値」以下になったら立って走る。
			 *          ちょうど立ち速度で切り替えると、速度の揺れで滑走と走りを往復するため少し上に置く。
			 */
			constexpr float COAST_SPEED_MARGIN = 1.05f;
			/**
			 * @brief 惰性の減速（ゆっくり減速）を使う傾斜の下限（符号つき傾斜sinθ）
			 * @details これより急な上りでは従来どおり速く減速する（上りの罰を残す）。
			 */
			constexpr float COAST_SLOPE_MIN = -0.06f;


			/**
			 * @brief 判定結果
			 */
			enum class EnDecision
			{
				/** 立つ（滑らない。無入力ならSlideEndで止まり、入力があればRun/Sneakへ） */
				Stand,
				/** 坂に運ばれて滑る（目標速度はslideSpeed×傾斜倍率） */
				Drive,
				/** 勢いだけで滑る（目標速度は立ち速度。減速は惰性） */
				Coast,
			};


			/**
			 * @brief 滑るかどうかを決める
			 * @details 速度はすべて倍率（スティックの倒し量・陣形倍率）込みの実速度で渡す。
			 *          無入力→Stand
			 *          未滑走:canStartかつ 傾斜>SLOPE_ENTERかつ 坂の速度>立ち速度→Drive（開始）
			 *          滑走中:傾斜>SLOPE_KEEPかつ 坂の速度>立ち速度→Drive
			 *          滑走中:それ以外で 現在速度>立ち速度×COAST_SPEED_MARGIN→Coast
			 *          それ以外→Stand
			 * @param isSliding     現在スライド中（SlideStart/Sliding）か
			 * @param hasMoveInput  移動入力があるか
			 * @param canStart      新たに滑り始めてよいか（接地しているかなど、呼び出し側の条件）
			 * @param slope         進行方向に対する符号つき傾斜（下り正）
			 * @param slopeSpeed    傾斜モデルで滑った場合の速度
			 * @param standingSpeed 立って動いた場合の目標速度
			 * @param currentSpeed  現在の水平速度
			 * @return 判定結果
			 */
			constexpr EnDecision Decide(
				const bool isSliding, const bool hasMoveInput, const bool canStart,
				const float slope, const float slopeSpeed, const float standingSpeed, const float currentSpeed)
			{
				if (!hasMoveInput) return EnDecision::Stand;

				// 立ち速度に届かない坂では滑らない（ダッシュの方が速いならダッシュする）
				const bool isSlopeFaster = slopeSpeed > standingSpeed;

				if (!isSliding)
				{
					return (canStart && slope > SLOPE_ENTER && isSlopeFaster)
						? EnDecision::Drive
						: EnDecision::Stand;
				}

				if (slope > SLOPE_KEEP && isSlopeFaster) return EnDecision::Drive;

				// 坂に運ばれなくなっても、勢いが立ち速度を上回るうちは惰性で滑り続ける
				if (currentSpeed > standingSpeed * COAST_SPEED_MARGIN) return EnDecision::Coast;

				return EnDecision::Stand;
			}

			/**
			 * @brief 惰性の減速（ゆっくり減速）を使う傾斜かどうか
			 * @param slope 進行方向に対する符号つき傾斜（下り正）
			 * @return 下り・平地・緩い上りならtrue、急な上りならfalse
			 */
			constexpr bool UsesCoast(const float slope)
			{
				return slope >= COAST_SLOPE_MIN;
			}


			// 真理値表（数値は親のslideSpeed 200/runSpeed 130を想定）
			/** 開始：下り坂で坂の速度がダッシュを上回る */
			static_assert(Decide(false, true, true, 0.10f, 260.0f, 130.0f, 130.0f) == EnDecision::Drive, "slide: start on downhill");
			/** 緩斜面では開始しない */
			static_assert(Decide(false, true, true, 0.04f, 224.0f, 130.0f, 130.0f) == EnDecision::Stand, "slide: no start on gentle slope");
			/** 坂の速度がダッシュ以下なら開始しない */
			static_assert(Decide(false, true, true, 0.10f, 120.0f, 130.0f, 130.0f) == EnDecision::Stand, "slide: no start when slower than dash");
			/** 坂で継続 */
			static_assert(Decide(true, true, true, 0.03f, 218.0f, 130.0f, 250.0f) == EnDecision::Drive, "slide: keep on slope");
			/** 平地では勢いで滑る */
			static_assert(Decide(true, true, true, 0.00f, 200.0f, 130.0f, 250.0f) == EnDecision::Coast, "slide: coast on flat");
			/** 勢いが切れたら立つ */
			static_assert(Decide(true, true, true, 0.00f, 200.0f, 130.0f, 136.0f) == EnDecision::Stand, "slide: stand when momentum is gone");
			/** 上りでも勢いがあれば滑る */
			static_assert(Decide(true, true, true, -0.20f, 60.0f, 130.0f, 250.0f) == EnDecision::Coast, "slide: coast uphill with momentum");
			/** 無入力なら立つ（SlideEndで止まる） */
			static_assert(Decide(true, false, true, 0.20f, 300.0f, 130.0f, 300.0f) == EnDecision::Stand, "slide: stand without input");
		}
	}
}
