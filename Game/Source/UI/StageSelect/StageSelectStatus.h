/**
 * @file StageSelectStatus.h
 * @brief StageSelectのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"

#include "StageSelectMenu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief StageSelectのステータスクラス
		 */
		class StageSelectStatus : public UIStatus
		{
		public:
			StageSelectStatus();
			~StageSelectStatus() override;


		public:
			/** @brief 入力間隔を取得する */
			inline float GetInputInterval() const { return m_inputInterval; }
			/** @brief 入力閾値を取得する */
			inline float GetInputThreshold() const { return m_inputThreshold; }

			/** @brief ステージ選択肢のテキストの位置を取得する */
			inline Vector3 GetStageSelectPosition() const { return m_stageSelectPosition; }


			/** @brief 選択肢のYオフセットを取得する */
			inline float GetChoicesYOffset() const { return m_choicesYOffset; }
			/** @brief 選択肢のX位置を取得する */
			inline float GetChoicePositionX(const EnStageChoices choice) const
			{
				return m_choicesPositionX.at(static_cast<size_t>(choice));
			}
			/** @brief テキストの色を取得する */
			inline Vector4 GetChoicesTextColor() const { return m_choicesTextColor; }


			/** @brief ボタンの背景の位置を取得する */
			inline Vector3 GetButtonBGPosition() const { return m_buttonBGPosition; }
			/** @brief ボタンのXオフセットを取得する */
			inline float GetButtonXOffset() const { return m_buttonXOffset; }
			/** @brief ボタンのYオフセットを取得する */
			inline float GetButtonYOffset() const { return m_buttonYOffset; }
			/** @brief ボタンのX位置を取得する */
			inline float GetButtonPositionX(const EnStageButtonTypes button) const
			{
				return m_buttonPositionX.at(static_cast<size_t>(button));
			}


			/** @brief テキストの背景色を取得する */
			inline Vector4 GetTextBGColor() const { return m_textBGColor; }


		public:
			void SetUp() override final;
			void Update() override final;



		private:
			/** 入力間隔 */
			float m_inputInterval;
			/** 入力閾値 */
			float m_inputThreshold;

			/** ステージ選択肢のテキストの位置 */
			Vector3 m_stageSelectPosition;

			/** 選択肢のYオフセット */
			float m_choicesYOffset;
			/** 選択肢のX位置 */
			std::array<float, static_cast<size_t>(EnStageChoices::Max)> m_choicesPositionX;
			/** テキストの色 */
			Vector4 m_choicesTextColor;

			/** ボタンの背景の位置 */
			Vector3 m_buttonBGPosition;
			/** ボタンのXオフセット */
			float m_buttonXOffset;
			/** ボタンのYオフセット */
			float m_buttonYOffset;
			/** ボタンのX位置 */
			std::array<float, static_cast<size_t>(EnStageButtonTypes::Max)> m_buttonPositionX;

			/** テキストの背景色 */
			Vector4 m_textBGColor;
		};
	}
}


