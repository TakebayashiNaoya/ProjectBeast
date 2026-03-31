/**
 * @file FinishMenu.h
 * @brief FINISH演出の動的処理クラス
 * @author 立山
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief FINISHアイコンクラス
		 */
		class FinishIcon
		{
		public:
			FinishIcon();
			~FinishIcon();
			void Update();
			void SetUIIcon(UIIcon* icon);

			inline void SetIsDraw(bool isDraw)
			{
				if (m_icon) m_icon->m_isDraw = isDraw;
			}

		private:
			UIIcon* m_icon;
		};


		/**
		 * @brief FINISH演出メニュークラス
		 * @detail ゲーム終了時に"FINISH"を表示し、演出終了を通知する
		 */
		class FinishMenu : public MenuBase
		{
			using FinishClass = MenuBase;

		public:
			FinishMenu();
			~FinishMenu() = default;

			void Update() override;
			void InitializeLogic() override;

			/**
			 * @brief FINISH演出を開始する
			 */
			void StartFinish();

			/**
			 * @brief 演出が終了したかどうかを返す
			 * @return 演出終了フラグ
			 */
			bool IsFinished() const { return m_isFinished; }

			/**
			 * @brief 演出中かどうかを返す
			 * @return 演出開始フラグ
			 */
			bool IsStarted() const { return m_isStarted; }

		private:
			/** 演出中フラグ */
			bool m_isStarted = false;
			/** 演出終了フラグ */
			bool m_isFinished = false;
			/** 経過時間 */
			float m_timer = 0.0f;

			/** 演出の表示時間（秒） */
			static constexpr float FINISH_DURATION = 3.0f;

			using Icon = std::unique_ptr<FinishIcon>;
			using Key = uint32_t;
			std::unordered_map<Key, Icon> m_finishIconMap;
		};
	}
}