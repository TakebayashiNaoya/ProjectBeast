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
		/** 前方宣言 */
		class StageSelectStatus;


		/**
		 * @brief ステージ選択肢の列挙体
		 * @detail もどる、イージー、ノーマル、ハード
		 */
		enum class EnStageChoices : uint8_t
		{
			Back,
			Easy,
			Normal,
			Hard,
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


		private:
			/** ステージ選択肢のデータ構造体 */
			struct StageChoicesData
			{
				/** 位置 */
				Vector3 m_position;
				/** テキストアイコン */
				UIIcon* m_textIcon;
				/** バブルアイコン */
				UIIcon* m_bubbleIcon;


				StageChoicesData();
				~StageChoicesData() = default;
			};

			/** ステージ選択のステータス */
			std::unique_ptr<StageSelectStatus> m_status;
			/** ステージ選択状態 */
			EnStageSelectState m_state;
			/** 選択中のステージ選択肢 */
			EnStageChoices m_selectingStage;


			/** 背景アイコン */
			UIIcon* m_bgIcon;
			/** "ステージセレクト"のアイコン */
			UIIcon* m_stageSelectTextIcon;
			/** ステージ選択肢 */
			std::array<StageChoicesData, static_cast<uint8_t>(EnStageChoices::Max)> m_choices;


			/** 選択カーソルのフレーム */
			UIIcon* m_cursorFrame;
			/** 選択カーソルのフレームの背景 */
			UIIcon* m_cursorFrameBG;

			/** ステージ選択肢のバブルのフレーム */
			UIIcon* m_backBubbleFrame;
			/** ステージ選択肢のバブルのフレームの背景 */
			UIIcon* m_backBubbleFrameBG;


			/** 選択入力のインターバル */
			float m_selectInputInterval;
			/** 選択されたかどうか */
			bool m_isSelected;
		};
	}
}


