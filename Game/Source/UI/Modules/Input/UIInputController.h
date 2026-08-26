/**
 * @file UIInputController.h
 * @brief UI入力制御を行うクラス
 */
#pragma once
#include "Types.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief スティック/十字キーの方向入力を判定するクラス
		 * @details
		 * - スティックの倒しっぱなし状態に応じて、方向入力を判定する
		 * - トリガー入力は倒しっぱなし状態に関係なく毎回一発判定する
		 * - 倒しっぱなし状態でrepeatIntervalが指定されていればオートリピートする
		 */
		class AxisInputDetector
		{
		public:
			/**
			 * @brief スティック/十字キーの方向入力を判定する
			 * @param stickValue     スティックの入力値
			 * @param triggerNegative 負方向に対応するボタンのトリガー入力
			 * @param triggerPositive 正方向に対応するボタンのトリガー入力
			 * @param threshold      スティックの反応閾値
			 * @param repeatInterval 倒しっぱなし時のリピート間隔（秒）。0以下ならリピートせず、
			 *                       ニュートラルに戻るまで再入力を無効にする（従来通りの挙動）
			 * @return 入力された方向（Noneの場合は入力なし）
			 */
			Direction Update(
				float stickValue,
				bool triggerNegative,
				bool triggerPositive,
				float threshold,
				float repeatInterval = 0.0f
			);


			/** @brief 内部状態をリセットする（Reload後などに呼ぶ） */
			inline void Reset()
			{
				m_isNeutral = true;
				m_repeatTimer = 0.0f;
			}


			AxisInputDetector();
			~AxisInputDetector() = default;


		private:
			/** ニュートラルに戻っているかどうか */
			bool m_isNeutral;
			/** リピートまでの残り時間 */
			float m_repeatTimer;
		};
	}
}


