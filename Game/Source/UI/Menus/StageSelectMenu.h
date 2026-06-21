/**
 * @file StageSelectMenu.h
 * @brief ステージ選択画面のメニュークラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/UI/Parts/UIParts.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ステージ選択肢の列挙体
		 */
		enum class EnStageChoices : uint8_t
		{
			Easy,
			Normal,
			Hard,
			Tutorial,
			Max,
		};


		/**
		 * @brief ボタンの種類
		 * @detail もどる、決定、選択
		 */
		enum class EnStageButtonTypes : uint8_t
		{
			Back,
			Decide,
			Select,
			Max,
		};


		/**
		 * @brief ステージ選択画面のメニュークラス
		 */
		class StageSelectMenu : public MenuBase
		{
		public:
			StageSelectMenu();
			~StageSelectMenu() override;


		private:
			void InitializeLogic() override final;

			void Update() override final;


		public:
			/** @brief ステージが選択されたかどうかを取得する */
			inline void SetIsSelected(const bool isSelected) { m_isSelected = isSelected; }


		public:
			/** @brief 選択中のステージを取得する */
			inline EnStageChoices GetSelectingStage() const { return m_selectingStage; }
			/** @brief ステージが選択されたかどうかを取得する */
			inline bool IsSelected() const { return m_isSelected; }
			/** @brief 選択後のアニメーションが終了したかどうかを取得する */
			inline bool IsFinishedSelectAnimation() const
			{
				return !m_cursorFrameBG->IsPlayAnimation() && m_isSelected;
			}
			/** @brief ステージ選択状態をリセットする */
			void Reset();



		private:
			/** @brief ステージ選択状態を更新する */
			void UpdateSelecting();
			/** @brief ステージ選択肢の選択状態を更新する */
			void UpdateSelected();



		private:
			/**
			 * @brief 描画フラグを更新する
			 */
			void UpdateDrawFlag();
			/**
			 * @brief 位置を更新する
			 */
			void UpdateIcons();
			/**
			 * @brief UIパーツを取得する
			 */
			void GetUIParts();
			/**
			 * @brief アニメーションを設定する
			 */
			void SetAnimations(const uint32_t animationKey);
			/**
			 * @brief JSONからメニューパラメーターを読み込む
			 */
			void LoadMenuParam();


		private:
			/** ステージ選択状態 */
			enum class EnStageSelectState : uint8_t
			{
				Selecting,
				Selected,
			};


			/** JSONから読み込むメニューパラメーター */
			struct StageSelectParam
			{
				float   inputInterval        = 0.2f;
				float   inputThreshold       = 0.5f;
				float   tutorialCursorScaleX = 400.0f / 280.0f;
				float   cursorBlinkDuration  = 0.5f;
				Vector4 cursorBlinkStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				Vector4 cursorBlinkEndColor   = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
				/** ステージごとの背景映像パス（Easy/Normal/Hard/Tutorial 順）*/
				std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> stageVideoPaths = {};
			};


			/** ステージ選択肢のデータ構造体 */
			struct StageChoicesData
			{
				/** テキスト */
				UIText* m_text;
				/** バブルアイコン */
				UIIcon* m_bubbleIcon;


				StageChoicesData();
				~StageChoicesData() = default;
			};




			/*****************************************************/


			/** ステージ選択のデータ構造体 */
			struct StageButtonData
			{
				/** ボタンアイコン */
				UIIcon* m_button;
				/** テキスト */
				UIText* m_text;


				StageButtonData();
				~StageButtonData() = default;
			};


		private:
			/** ステージ選択状態 */
			EnStageSelectState m_state;
			/** 選択中のステージ選択肢 */
			EnStageChoices m_selectingStage;


			/** 背景アイコン */
			UIIcon* m_bgIcon;
			/** "ステージセレクト"のテキスト */
			UIText* m_stageSelectText;
			/** "ステージセレクト"の背景アイコン */
			UIIcon* m_stageSelectTextBGIcon;

			/** ステージ選択肢 */
			std::array<StageChoicesData, static_cast<uint8_t>(EnStageChoices::Max)> m_choices;

			/** ステージ選択画面のボタン */
			std::array<StageButtonData, static_cast<uint8_t>(EnStageButtonTypes::Max)> m_buttons;
			/** ボタンの背景アイコン */
			UIIcon* m_buttonBGIcon;

			/** 選択カーソルのフレーム */
			UIIcon* m_cursorFrame;
			/** 選択カーソルのフレームの背景 */
			UIIcon* m_cursorFrameBG;

			/** ステージ背景映像 */
			UIVideo* m_stagePreviewVideo;
			/** 直前のステージ選択（映像切り替え検出用）*/
			EnStageChoices m_prevSelectingStage;

			/** 選択入力のインターバル */
			float m_selectInputInterval;
			/** 選択されたかどうか */
			bool m_isSelected;
			/** JSONから読み込んだメニューパラメーター */
			StageSelectParam m_param;
		};
	}
}


