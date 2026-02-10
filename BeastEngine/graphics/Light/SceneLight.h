/**
 * @file SceneLight.h
 * @brief シーンライトに関する構造体をまとめたヘッダファイル
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief ディレクションライト
	 */
	struct DirectionalLight
	{
		Vector3 direction;  /** ライトの方向 */
		int castShadow;     /** 影をキャストするかどうか */
		Vector4 color;      /** ライトのカラー */
	};
}

