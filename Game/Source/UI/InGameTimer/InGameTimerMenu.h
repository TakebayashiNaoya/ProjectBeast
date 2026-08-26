/**
 * @file InGameTimerMenu.h
 * @brief インゲームタイマーの動的処理クラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/InGameStartingAnimLogic/InGameStartingAnimLogic.h"


namespace app
{
	namespace ui
	{


		class InGameTimerMenu : public MenuBase
		{
			using InGameTimerClass = MenuBase;

		public:
			InGameTimerMenu();
			~InGameTimerMenu() = default;

			void Update() override;
			void InitializeLogic() override;

			/**
			 * @brief 現在のタイムを設定する
			 * @param time 現在のタイム（秒）
			 */
			inline void SetTime(const float time) { m_currentTime = time; }


		private:
			/**
			 * @brief タイムの数字を更新するため
			 * @brief Update内を肥大化させない為に分割
			 */
			void UpdateTimerDigits();

			/**
			 * @brief 時計の回転を更新するため
			 * @brief Update内を肥大化させない為に分割
			 */
			void UpdateClockRotation();


		private:
			/** 現在のタイム（BattleManagerから通知されてセットされる） */
			float m_currentTime;

			/** 点滅させる秒数 */
			float m_blinkTime;
			/** 点滅が可能かのフラグ */
			bool m_isBlink;
			/** カラーアニメーションするかのフラグ */
			bool m_isBlinkAnimationPlaying;
			/** 回転アニメーションするかのフラグ */
			bool m_isRotAnimationPlaying;
			/** 拡縮アニメーションするかのフラグ */
			bool m_isScaleAnimPlaying;
			/** 回転 */
			float m_clockAngle;
			/** 回転のタイマー */
			float m_slopeTimer;


			InGameStartingAnimLogic m_gameStartingAnimLogic;
		};
	}
}