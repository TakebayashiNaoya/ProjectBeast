/**
 * @file TitleEventMenu.h
 * @brief タイトルの動的処理クラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/Input/UICursorSelector.h"
#include "Source/UI/Modules/Input/UIInputController.h"


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

			/**
			 * @brief タイトルの環境演出（音符・雪・ロゴの弾み）の更新
			 * @details 静止画のタイトルに命を入れるレイヤー。
			 *          音符が湧き上がり、雪が降り、ロゴが拍で弾む。
			 */
			void UpdateAmbient();


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

			/** 環境演出（音符・雪・ロゴ弾み）用の経過時間（秒） */
			float m_ambientTimer = 0.0f;
			/** カーソル移動ポップの残り時間（秒） */
			float m_cursorPopTimer = 0.0f;
			/** ロゴのJSON上の基準スケール（弾み演出の基準値） */
			Vector3 m_rogoBaseScale = Vector3::One;
			/** 基準スケールを取得済みか */
			bool m_isRogoBaseScaleCaptured = false;
		};
	}
}
