/**
 * @file SoundOptionMenu.h
 * @brief サウンドのオプションの動的処理群
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/Input/UICursorSelector.h"
#include "Source/UI/Modules/Input/UIInputController.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief サウンドの種類
		 */
		enum class SoundType : uint8_t
		{
			Master,
			Voice,
			SE,
			BGM
		};


		/**
		 * @brief サウンドオプションの動的クラス
		 */
		class SoundOptionMenu : public MenuBase
		{
			using SoundClass = MenuBase;


		public:
			SoundOptionMenu();

			void Update() override;
			void InitializeLogic() override;


		private:
			/**
			 * @brief 全てのDigitの表示を更新する
			 */
			void UpdateDigits();

			/**
			 * @brief 現在選択中のノブアイコンの入力処理と音量反映
			 */
			void UpdateKnob();

			/**
			 * @brief 選択中のアイコンのカラーを線形的に変化させる。
			 */
			void UpdateColorAnim();

			/**
			 * @brief 値と座標を初期値に戻す
			 */
			void Reset();

			/**
			 * @brief サウンドメニューで扱うアイコンの情報を初期化する
			 */
			void InitializeIcon();

			/**
			 * @brief サウンドメニューで扱うフレームの情報を初期化する
			 */
			void InitializeFrame();

			/**
			 * @brief サウンドメニューで扱うアイコンのアニメーションを登録する用
			 */
			void InitializeIconAnim();

			/**
			 * @brief サウンドメニューで扱う数値の情報を初期化する
			 */
			void InitializeDigit();

			/**
			 * @brief 戻るフラグ
			 * @return 戻るフラグを取得
			 */
			bool IsBack() const { return m_isBack; }
			/**
			 * @brief 戻るフラグを設定
			 * @param isReturn 戻るフラグ
			 */
			void SetBack(bool isBack) { m_isBack = isBack; }


		private:
			/** 現在値 */
			float m_currentValue;
			/** 戻るボタンのフラグ */
			bool m_isBack;
			SoundType m_currentSoundType;
			/** スティック上下ニュートラル判定 */
			bool m_isStickNeutralY;
			/** スティック左右ニュートラル判定 */
			bool m_isStickNeutralX;

			/** スティックのY入力検出器 */
			AxisInputDetector m_stickYDetector;
			/** スティックのX入力検出器 */
			AxisInputDetector m_stickXDetector;
			/** カーソルのインデックス選択器 */
			CursorIndexSelector m_cursorSelector;
		};
	}
}
