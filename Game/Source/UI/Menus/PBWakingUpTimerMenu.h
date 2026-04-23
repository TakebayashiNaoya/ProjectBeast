/**
 * @file PBWakingUpTimerMenu.h
 * @brief PB起床タイマーの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/UI/Layout.h"


namespace app
{
	namespace ui
	{
		class PBWakingUpTimerMenu : public MenuBase
		{
			using PBWakingUpTimerClass = MenuBase;

		public:
			PBWakingUpTimerMenu();
			virtual ~PBWakingUpTimerMenu();

			void Update()override;
			void InitializeLogic()override;


		public:
			/**
			 * @brief タイマーの円の回転処理
			 */
			void CircleTimer();

			/**
			 * @brief 表示するタイマー値を設定する（EnemyStateMachineのSleepTimerを渡す）
			 * @param time 現在の睡眠タイマー値（0.0f〜30.0f）
			 */
			inline void SetCurrentPBTime(float time) { m_currentPBTime = time; }

			/**
			 * @brief エネミーのワールド座標を設定する（表示位置の計算に使用）
			 * @param position エネミーのワールド座標
			 */
			inline void SetTargetPosition(const Vector3& position) { m_targetPosition = position; }

			/**
			 * @brief 描画するかどうかを設定する
			 * @param isDraw 描画するか
			 */
			inline void SetDraw(bool isDraw) { m_isDraw = isDraw; }


		private:
			/** シロクマの起床タイマーレイアウト */
			std::unique_ptr<Layout>m_pbTimerLayout; 

			/** 現在のタイマー値（外部から毎フレーム設定される） */
			float m_currentPBTime = 0.0f;

			/** 黄色になる瞬間のフラグ */
			bool m_isYellowPlayed;

			/** 赤色になる瞬間のフラグ */
			bool m_isRedPlayed;

			/** 表示対象エネミーのワールド座標 */
			Vector3 m_targetPosition = Vector3::Zero;

			/** 描画するかどうか */
			bool m_isDraw = false;
		};
	}
}