/**
 * @file Event.h
 * @brief イベントの動的処理
 * @author 忽那
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		enum class EnEventType : uint8_t
		{
			Victory,
			Defeat,
			Max
		};


		class EventIcon
		{
		public:
			EventIcon(EnEventType type);
			~EventIcon();
			void Update();
			void SetUIIcon(UIIcon* icon);
			/**
			 * @brief イベントアイコンの描画状態を設定
			 * @param isDraw 描画状態
			 */
			inline void SetIsDraw(bool isDraw)
			{
				m_icon->m_isDraw = isDraw;
			}
			/**
			 * @brief イベントアイコンのタイプを取得
			 * @return m_type イベントアイコンのタイプ
			 */
			inline EnEventType GetType()const
			{
				return m_type;
			}


		private:
			UIIcon* m_icon;
			EnEventType m_type;
		};


		class EventMenu : public MenuBase
		{
			using EventClass = MenuBase;
		public:
			EventMenu();
			~EventMenu();
			void Update()override;
			void InitializeLogic()override;
			/**
			 * @brief イベントのリセット
			 */
			void ResetEvent();

			/**
			 * @brief イベントを開始
			 * @param eventType 開始するイベントのタイプ
			 */
			void StartEvent(EnEventType eventType);


			/**
			 * @brief 現在のイベントタイプを取得
			 * @return m_currentEventType 現在のイベントタイプ
			 */
			EnEventType GetCurrentEventType()const { return m_currentEventType; }

			/**
			 * @brief イベントが開始しているかどうかを取得
			 * @return m_isEventStart イベントが開始しているかどうか
			 */
			bool IsEventStart()const { return m_isEventStart; }
			/**
			 * @brief イベントの開始状態を設定
			 * @param isEventStart イベントの開始状態
			 */
			void SetIsEventStart(bool isEventStart) { m_isEventStart = isEventStart; }
			/**
			 * @brief イベントが終了しているかどうかを取得
			 * @return m_isEventFinished イベントが終了しているかどうか
			 */
			bool IsEventFinished()const { return m_isEventFinished; }
			/**
			 * @brief イベントの終了状態を設定
			 * @param isEventFinished イベントの終了状態
			 */
			void SetIsEventFinished(bool isEventFinished) { m_isEventFinished = isEventFinished; }

			/**
			 * @brief 現在のイベントタイマーを取得
			 * @return m_evnetTimer 現在のイベントタイマー
			 */
			float GetCurrentEventTimer()const { return m_eventTimer; }
			/**
			 * @brief 現在のイベントタイマーを設定
			 * @param eventTimer 現在のイベントタイマー
			 */
			void SetCurrentEventTimer(float eventTimer) { m_eventTimer = eventTimer; }
		

		private:
			bool m_isEventStart;
			bool m_isEventFinished;
			GamePad* m_gamePad;
			/** 経過時間の計測 */
			float m_eventTimer;
			/** イベントを表示しておく時間 */
			static constexpr float EVENT_DURATION = 3.0f;
			EnEventType m_currentEventType;
			using Icon = std::unique_ptr<EventIcon>;
			using Key = uint32_t;

			std::unordered_map<Key, Icon>m_eventIconMap;
		};
	}
}