/**
 * @file WpWarningMenu.h
 * @brief WpWarningのメニュークラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Menu.h"

#include "WpWarningAnimStatus.h"
#include "WpWarningStatus.h"


namespace app
{
	namespace nature
	{
		/** 前方宣言 */
		class Whirlpool;
	}


	namespace ui
	{
		/**
		 * @brief WpWarningのメニュークラス
		 */
		class WpWarningMenu : public MenuBase
		{
		public:
			/**
			 * @brief ターゲットの座標を設定
			 * @param position 座標
			 */
			void SetTargetPosition(const Vector3& position)
			{
				m_speechBubble->m_transform.m_localTransform.m_position = position;
				m_warning->m_transform.m_localTransform.m_position = position;
			}
			/**
			 * @brief 描画するかどうかの設定
			 * @param isDraw 描画するかどうか
			 */
			void SetIsDraw(const bool isDraw)
			{
				m_speechBubble->m_isDraw = isDraw;
				m_warning->m_isDraw = isDraw;
				m_isDraw = isDraw;
			}
			/**
			 * @brief ステータスを設定
			 * @param status ステータス
			 */
			void SetStatus(WpWarningStatus* status)
			{
				m_status = status;
			}


		public:
			WpWarningMenu();
			~WpWarningMenu() override = default;

			/** @brief UIのロジック初期化処理 */
			void InitializeLogic()override final;
			/** @brief UIの更新処理 */
			void Update() override final;


		private:
			/** @brief アイコンの位置を更新する関数 */
			void UpdateIconPosition();
			/** @brief アニメーションを設定する関数 */
			void SetAnimation(UIIcon* icon);
			/** @brief アニメーションをリセットする関数 */
			void ResetAnimation(UIIcon* icon);


		private:
			/** 吹き出しアイコンのポインタ */
			UIIcon* m_speechBubble;
			/** 警告アイコンのポインタ */
			UIIcon* m_warning;

			/** ステータス */
			WpWarningStatus* m_status;
			/** アニメーションステータス */
			std::unique_ptr<WpWarningAnimStatus> m_animStatus;
			/** 描画フラグ */
			bool m_isDraw;
		};
	}
}