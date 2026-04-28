/**
 * @file PBWakingUpTimerMenu.h
 * @brief PB起床タイマーの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/UI/Model/PBWakingUpTimerStatus.h"
#include "Source/UI/Model/PBWakingUpTimerAnimStatus.h"


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

			/**
			 * @brief リセット漏れを防ぐ
			 */
			void ResetTimer();


		private:
			/** シロクマの起床タイマー専用のアニメーションステータス */
			std::unique_ptr<PBWakingUpTimerAnimStatus>m_pbAnimStatus;
			/** シロクマの起床タイマー専用のステータス */
			std::unique_ptr<PBWakingUpTimerStatus>m_pbTimerStatus;

			/** 現在のタイマー値（外部から毎フレーム設定される） */
			float m_currentPBTime = 0.0f;

			/** 最初から緑から黄色になるフラグ */
			bool m_isGreenPlayed;

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