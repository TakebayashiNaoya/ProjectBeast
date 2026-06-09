/**
 * @file MasterStageSelectParameter.h
 * @brief StageSelectのパラメーター管理クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief StageSelectのパラメーター管理クラス
		 */
		struct MasterStageSelectParameter : public core::IMasterParameter
		{
			appParameter(MasterStageSelectParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */
			/** 入力間隔 */
			float inputInterval;
			/** 入力閾値 */
			float inputThreshold;


			/** ステージ選択肢のテキストの位置 */
			Vector3 stageSelectPosition;


			/** 選択肢のYオフセット */
			float choicesYOffset;
			/** チュートリアル選択肢のX位置 */
			float tutorialPositionX;
			/** イージー選択肢のX位置 */
			float easyPositionX;
			/** ノーマル選択肢のX位置 */
			float normalPositionX;
			/** ハード選択肢のX位置 */
			float hardPositionX;
			/** テキストの色 */
			Vector4 choicesTextColor;

			/** ボタンの背景の位置 */
			Vector3 buttonBGPosition;
			/** ボタンのXオフセット */
			float buttonXOffset;
			/** ボタンのYオフセット */
			float buttonYOffset;
			/** もどるボタンのX位置 */
			float backButtonPositionX;
			/** 決定ボタンのX位置 */
			float decideButtonPositionX;
			/** 選択ボタンのX位置 */
			float selectButtonPositionX;

			/** テキストの背景色 */
			Vector4 textBGColor;
		};
	}
}


