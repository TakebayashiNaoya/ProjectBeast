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
			using WpWarning = MenuBase;

		public:
			/**
			 * @brief ステータスを設定
			 * @param status ステータス
			 */
			void SetStatus(WpWarningStatus* status)
			{
				m_status = status;
			}
			/**
			 * @brief 描画フラグを設定
			 * @param isDraw 描画フラグ
			 */
			void SetIsDraw(bool isDraw)
			{
				m_isDraw = isDraw;
			}
			/**
			 * @brief 渦潮の位置を設定
			 * @param position 渦潮の位置
			 */
			void SetWhirlpool(nature::Whirlpool* whirlpool)
			{
				m_whirlpool = whirlpool;
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
			void SetAnimation();
			/** @brief アニメーションをリセットする関数 */
			void ResetAnimation();


		private:
			/** 吹き出しアイコンのポインタ */
			UIIcon* m_speechBubble;
			/** 警告アイコンのポインタ */
			UIIcon* m_warning;

			/** ステータス */
			WpWarningStatus* m_status;
			/** アニメーションステータス */
			std::unique_ptr<WpWarningAnimStatus> m_animStatus;
			/** 渦潮 */
			nature::Whirlpool* m_whirlpool;
			/** 描画フラグ */
			bool m_isDraw;
		};
	}
}