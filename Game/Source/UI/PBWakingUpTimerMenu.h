/**
 * @file PBWakingUpTimerMenu.h
 * @brief PB起床タイマーの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/Enemy.h"


namespace app
{
	namespace ui
	{
		class PBWakingUpTimerDigit
		{
		public:
			PBWakingUpTimerDigit();
			~PBWakingUpTimerDigit();
			void Update();
			void SetUIDigit(UIDigit* digit);
			void SetValue(float value);
			inline void SetIsDraw(bool isDraw)
			{
				if (m_digit)m_digit->m_isDraw = isDraw;
			}


		private:
			UIDigit* m_digit;
		};




		class PBWakingUpTimerMenu : public MenuBase
		{
			using PBWakingUpTimerClass = MenuBase;


		public:
			PBWakingUpTimerMenu();
			~PBWakingUpTimerMenu();
			void Update()override;
			void InitializeLogic()override;
			void ResetTimer();


			/**
			 * @brief PB起床タイマーの描画距離の取得
			 * @return m_isLengthDraw 描画距離の取得
			 */
			//inline bool IsLengthDraw()const { return m_isLengthDraw; }
			/**
			 * @brief PB起床タイマーの描画距離の設定
			 * @param isLengthDraw 描画距離に応じての設定
			 */
			//inline void SetIsLengthDraw(bool isLengthDraw) { m_isLengthDraw = isLengthDraw; }

			/**
			 * @brief PB起床タイマーのタイマーアクティブの取得
			 * @return m_isTimerActive タイマーアクティブの取得
			 */
			inline bool IsPBTimerActive()const { return m_isPBTimerActive; }
			/**
			 * @brief PB起床タイマーのタイマーアクティブかの設定
			 * @param isTimerActive タイマーアクティブの設定
			 */
			inline void SetIsPBTimerActive(bool isPBTimerActive) { m_isPBTimerActive = isPBTimerActive; }
			/**
			 * @brief 外部からPBの起床タイマーを設定する
			 * @param time PBの起床タイマー
			 */
			inline void SetCurrentPBTime(float time){ m_currentPBTime = time; }
			
			/**
			 * @brief Enemyの設定
			 * @param enemy しろくま
			 */
			void SetEnemy(actor::Enemy* enemy) { m_enemy = enemy; }

			void SetIsDraw(bool isDraw);


		private:
			float m_currentPBTime;
			float m_maxTime;
			float m_minTime;
			bool m_isLengthDraw;
			bool m_isPBTimerActive;

			actor::EnemyController* m_eneCon;
			actor::Enemy* m_enemy;

			using Digit = std::unique_ptr<PBWakingUpTimerDigit>;
			using Key = uint32_t;

			std::unordered_map<Key, Digit>m_wakingUpTimeMap;
		};
	}
}
