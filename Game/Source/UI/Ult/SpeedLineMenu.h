/**
 * @file SpeedLineMenu.h
 * @brief 加速時に集中線を表示するメニュー
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 加速時の集中線演出を行うメニュークラス
		 */
		class SpeedLineMenu : public MenuBase
		{
		public:
			SpeedLineMenu();
			~SpeedLineMenu();


			void InitializeLogic() override;

			void Update() override;


		public:
			/**
			 * @brief 加速度合いを設定する(毎フレーム、ゲーム側から呼ぶ)
			 * @param accel01 加速度合い 0.0f(非加速)～1.0f(最大加速)
			 * @details
			 *   例: プレイヤーの現在速度から
			 *       SetAcceleration((speed - normalSpeed) / (maxSpeed - normalSpeed));
			 *   のように正規化して渡す。ブースト中だけ 1.0f を渡す形でもよい。
			 */
			void SetAcceleration(float accel01);

			/**
			 * @brief 集中線が現在見えているかどうか
			 */
			bool IsVisible() const { return m_currentAlpha > 0.01f; }

			/**
			 * @brief 集中線の表示/非表示を設定する
			 * @param active true: 表示する, false: 非表示にする
			 */
			void SetActive(const bool active)
			{
				m_isActive = active;
			}


		private:
			/** 集中線本体(必須) */
			UIIcon* m_mainLine;
			/** 重ね用の2枚目(任意。無ければ nullptr のまま) */
			UIIcon* m_subLine;

			/** JSONで指定された初期スケール(脈動の基準にする) */
			Vector3 m_mainBaseScale;
			Vector3 m_subBaseScale;
			/** JSONで指定された初期カラー(RGBは維持し、Aのみ制御する) */
			Vector4 m_mainBaseColor;
			Vector4 m_subBaseColor;

			/** 目標アルファ(=加速度合い) */
			float m_targetAlpha;
			/** 現在アルファ(目標へ滑らかに追従) */
			float m_currentAlpha;

			/** 脈動用の経過時間 */
			float m_time;
			/** 描き替えちらつき用のフレームカウンタ */
			int m_flickerCounter;
			/** 現在のちらつき回転角(度) */
			float m_flickerDeg;
			/** 集中線の表示/非表示フラグ */
			bool m_isActive = false;


		private:
			/**
			 * @brief 集中線1枚分のパラメーター反映
			 * @param line        対象のUIIcon
			 * @param baseScale   JSON初期スケール
			 * @param baseColor   JSON初期カラー
			 * @param alpha       このフレームのアルファ
			 * @param flickerDeg  ちらつき回転角(度)
			 * @param pulseScale  脈動スケール倍率
			 */
			void ApplyToLine(
				UIIcon* line
				, const Vector3& baseScale
				, const Vector4& baseColor
				, const float alpha
				, const float flickerDeg
				, const float pulseScale
			);
		};
	}
}
