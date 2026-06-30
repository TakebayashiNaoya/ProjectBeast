/**
 * @file MiniMapParameter.h
 * @brief ミニマップ専用パラメータークラス
 * @author 忽那
 */
#pragma once
#include "Source/Core/IMasterParameter.h"

#include "MiniMapTypes.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ミニマップ専用パラメーター
		 */
		struct MiniMapParameter : public core::IMasterParameter
		{
			appParameter(MiniMapParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}
#endif
			/** JSONから受け取る変数群 */
			/** マップの半径 */
			float mapRadius;
			/** マップの距離制限 */
			float mapLimitDistance;
			/** マップの中心座標 */
			Vector3 mapCenterPos;
			/** アイコンの初期化情報 */
			MiniMapInitializeInfo iconInitializeInfos;
			/** 画像の初期座標 */
			Vector3 initPosition;
			/** 画像の初期拡大率 */
			Vector3 initScale;
			/** 画像の初期回転 */
			Quaternion initRotation;
			/** 画像の初期カラー */
			Vector4 initColor;
		};
	}
}


