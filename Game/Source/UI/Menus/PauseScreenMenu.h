/**
 * @file PauseScreenMenu.h
 * @brief ポーズ画面の動的クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"
#include <unordered_map>
#include <memory>


namespace app
{
	namespace ui
	{
		/** ポーズ画面のタイプの種類 */
		enum class PauseScreenType : uint8_t
		{
			TitleType,
			ReturnPlayType,
			SoundOptionType,
			RuleType,
			GoBackTitleType,
			Max
		};


		class PauseScreenIcon
		{
		public:
			PauseScreenIcon(PauseScreenType type);
			~PauseScreenIcon();
			void SetUIIcon(UIIcon* icon);
			void UpdateSelect(PauseScreenType currentType);


		private:
			PauseScreenType m_type;
			UIIcon* m_icon;
			GamePad* m_gamePad;
		};


		class PauseScreenButton
		{
		public:
			PauseScreenButton(PauseScreenType type);
			~PauseScreenButton();
			void SetUIButton(UIButton* button);
			void SetPauseIcon(PauseScreenIcon* icon);
			void UpdateSelect(PauseScreenType currentType);


		private:
			PauseScreenType m_type;
			UIButton* m_button;
			PauseScreenIcon* m_pauseIcon;
			GamePad* m_gamePad;
		};


		class PauseScreenMenu : public MenuBase
		{
			using PauseClass = MenuBase;

		public:
			PauseScreenMenu();

			void Update()override;
			void InitializeLogic()override;


		public:
			/**
			 * @brief アイコンの初期化用
			 */
			void InitializeIcon();
			/**
			 * @brief ボタンの初期化用
			 */
			void InitializeButton();

			/**
			 * @brief アイコン、ボタンの更新用
			 */
			void UpdateSelect();

			/**
			 * @brief アイコンとボタンを共通して動かす用
			 */
			void MoveCursor();

			/**
			 * @brief 現在のポーズ画面のタイプを変更
			 */
			void EnterType();

			/**
			 * @brief 現在のポーズ画面のタイプを取得
			 * @return 現在のポーズ画面のタイプ
			 */
			PauseScreenType GetCurrentType()const { return m_currentType; }


		public:
			/**
			 * @brief ポーズ画面の結果を取得
			 * @return ポーズ画面の結果
			 */
			bool IsRetry()const { return m_isRetry; }
			/**
			 * @brief ポーズ画面の結果を取得
			 * @return ポーズ画面の結果
			 */
			bool IsSound()const { return m_isSound; }
			/**
			 * @brief ポーズ画面の結果を取得
			 * @return ポーズ画面の結果
			 */
			bool IsRule()const { return m_isRule; }
			/**
			 * @brief ポーズ画面の結果を取得
			 * @return ポーズ画面の結果
			 */
			bool IsGoTitle()const { return m_isGoTitle; }

			/**
			 * @brief ポーズ画面の結果を設定
			 * @param isRetry リトライするかどうか
			 */
			void IsRetry(bool isRetry) { m_isRetry = isRetry; }
			/**
			 * @brief ポーズ画面の結果を設定
			 * @param isGoTitle タイトルに戻るかどうか
			 */
			void IsGoTitle(bool isGoTitle) { m_isGoTitle = isGoTitle; }
			/**
			 * @brief ポーズ画面の結果を設定
			 * @param isSound サウンドオプションに行くかどうか
			 */
			void IsSound(bool isSound) { m_isSound = isSound; }
			/**
			 * @brief ポーズ画面の結果を設定
			 * @param isRule ルール画面に行くかどうか
			 */
			void IsRule(bool isRule) { m_isRule = isRule; }


		private:
			/** 現在のポーズアイコン/ボタンタイプ */
			PauseScreenType m_currentType;
			/** 表示されているかどうか */
			bool m_isVisible;
			/** 戻る */
			bool m_isRetry;
			/** サウンド */
			bool m_isSound;
			/** ルール */
			bool m_isRule;
			/** タイトル */
			bool m_isGoTitle;


			using Icon = std::unique_ptr<PauseScreenIcon>;
			using Button = std::unique_ptr<PauseScreenButton>;
			using Key = uint32_t;
			std::unordered_map<Key, Icon>m_pauseIconMap;
			std::unordered_map<Key, Button>m_pauseButtonMap;
		};
	}
}