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
			/** @brief 選択肢の位置を取得する */
			inline const Vector3& GetChoicesBasePosition(const EnStageChoices& choice) const { return m_choicesPositions.at(static_cast<size_t>(choice)); }
			/** @brief 選択肢の位置を全て取得する */
			inline const std::array<Vector3, static_cast<size_t>(EnStageChoices::Max)>& GetChoicesPositions() const { return m_choicesPositions; }
			/** @brief テキストの色を取得する */
			inline const Vector4& GetTextColor() const { return m_textColor; }


		public:
			void SetUp() override final;
			void Update() override final;



		private:
			/** 入力間隔 */
			float m_inputInterval;
			/** 入力閾値 */
			float m_inputThreshold;
			/** 選択肢の位置 */
			std::array<Vector3, static_cast<size_t>(EnStageChoices::Max)> m_choicesPositions;
			/** テキストの色 */
			Vector4 m_textColor;
		};
	}
}


