/**
 * @file TitleEventMenu.h
 * @brief タイトルの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Input/UICursorSelector.h"
#include "Source/UI/Input/UIInputController.h"


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

			/**
			 * @brief UIパーツの取得
			 */
			void GetUIParts();

			/**
			 * @brief 描画フラグの更新
			 */
			void UpdateDrawFlag();


		public:
			/**
			 * @brief イベントの種類のenum
			 * @details Start:スタート、Sound:サウンドオプション、Rule:ルール説明、End:おわり
			 */
			enum class EnEventType : uint8_t
			{
				Start,
				Sound,
				Rule,
				End,
				Num
			};


		private:
			GamePad* m_gamePad;
			EnEventType m_selectIndex;
			bool m_isStickNeutral;
			bool m_isSelect;
			bool m_isDraw;


			/** 背景アイコン */
			UIIcon* m_bgIcon;
			/** ロゴアイコン */
			UIIcon* m_rogoIcon;
			/** フレームアイコン */
			UIIcon* m_frameIcon;
			/** フレームの背景アイコン */
			UIIcon* m_frameBackIcon;
			/** イベントテキスト */
			std::array<UIText*, static_cast<uint8_t>(EnEventType::Num)> m_eventIcon;

			/** コントローラの入力制御 */
			AxisInputDetector m_axisInputDetector;
			/** カーソル移動時の制御 */
			CursorIndexSelector m_cursorSelector;
		};
	}
}
