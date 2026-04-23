/**
 * @file TitleEventMenu.h
 * @brief タイトルの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		class TitleEventMenu : public MenuBase
		{
		public:
			TitleEventMenu();
			~TitleEventMenu();
			void Update()override;
			void InitializeLogic()override;

			/**
			 * @brief 選択されているアイコンのビジュアルを変更させる
			 */
			void SelectVisual();
			
			/**
			 * @brief 現在選択されているキーを取得する
			 * @return 現在選択されているキーの取得
			 */
			uint32_t GetSelectKey()const;

			/**
			 * @brief 描画の設定。
			 * @param isDraw 描画するかどうかのフラグ。
			 */
			void SetDraw(bool isDraw) { m_isDraw = isDraw; }


		private:
			GamePad* m_gamePad;
			int m_selectIndex;
			bool m_isStickNeutral;
			bool m_isSelect;
			bool m_isDraw;
		};
	}
}
