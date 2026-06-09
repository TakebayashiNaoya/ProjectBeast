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
			/** 選択肢のピボット */
			Vector2 choicesPivot;
			/* イージー選択肢の位置 */
			Vector3 easyPosition;
			/* ノーマル選択肢の位置 */
			Vector3 normalPosition;
			/* ハード選択肢の位置 */
			Vector3 hardPosition;
			/* 戻る選択肢の位置 */
			Vector3 backPosition;
			/** オフセット位置 */
			Vector3 offSetPosition;
			/** テキストの色 */
			Vector4 textColor;
		};
	}
}


