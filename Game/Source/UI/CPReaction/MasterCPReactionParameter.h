/**
 * @file MasterCPReactionParameter.h
 * @brief CPReactionのパラメーター管理クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief CPReactionのパラメーター管理クラス
		 */
		struct MasterCPReactionParameter : public core::IMasterParameter
		{
			appParameter(MasterCPReactionParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */

			/** リアクションの描画時間 */
			float swayTime;
			/** アイコンのYオフセット */
			float iconOffsetY;
			/** プレイヤーからこの距離より離れた子ペンギンの吹き出しは表示しない */
			float drawableDistance;
			/** 吹き出しのオフセット */
			Vector3 speechBubbleOffset;
			/** 困りリアクションのオフセット */
			Vector3 troubleReactionOffset;
			/** 喜びリアクションのオフセット */
			Vector3 happyReactionOffset;
			/** 真面目ペンギンのリアクションカラー */
			Vector4 seriousReactionColor;
			/** 甘えん坊ペンギンのリアクションカラー */
			Vector4 clingyReactionColor;
			/** やんちゃペンギンのリアクションカラー */
			Vector4 naughtyReactionColor;
			/** おっちょこちょいペンギンのリアクションカラー */
			Vector4 clumsyReactionColor;
			/** 世話焼きペンギンのリアクションカラー */
			Vector4 caringReactionColor;
		};
	}
}


