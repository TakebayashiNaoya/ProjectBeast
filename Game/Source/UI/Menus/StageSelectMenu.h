/**
 * @file StageSelectMenu.h
 * @brief ステージ選択画面のメニュークラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ステージ選択肢の列挙体
		 * @detail もどる、イージー、ノーマル、ハード
		 */
		enum class EnStageChoices : uint8_t
		{
			Tutorial,
			Easy,
			Normal,
			Hard,
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


		private:
			/** ステージ選択状態 */
			enum class EnStageSelectState : uint8_t
			{
				Selecting,
				Selected,
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


			/** 選択入力のインターバル */
			float m_selectInputInterval;
			/** 選択されたかどうか */
			bool m_isSelected;
		};
	}
}


