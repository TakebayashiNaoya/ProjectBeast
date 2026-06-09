/**
 * @file RenderViewContext.h
 * @brief 描画に使用するビューのリソースをまとめた構造体
 * @author 竹林
 */
#pragma once
#include "BeastEnginePreCompile.h"
#include "Geometry/Frustum.h"


namespace nsBeastEngine
{
	/** GBufferに入れるレンダリングターゲットの役割 */
	enum EnGBuffer
	{
		enGBuffer_Albedo = 0,  /** アルベド		*/
		enGBuffer_Normal,      /** 法線			*/
		enGBuffer_Specular,    /** スペキュラ   */
		enGBuffer_Num,         /** G-Bufferの数 */
	};


	/**
	 * @brief 描画に使用するリソースをまとめた構造体
	 * @details メインカメラ・サブカメラそれぞれのGBuffer・レンダーターゲット・
	 *          フラスタム・カメラを保持する
	 */
	struct RenderViewContext
	{
		UINT width = 0;		/** 幅 */
		UINT height = 0;	/** 高さ */

		std::array<RenderTarget, enGBuffer_Num> gBuffer;	/** GBufferのレンダリングターゲット */

		RenderTarget	renderTarget;			/** レンダリングターゲット */
		Sprite			deferredLightingSprite;	/** ディファードシェーディング用のスプライト */
		Frustum			frustum;				/** フラスタム */
		nsK2EngineLow::Camera* camera = nullptr;	/** このビューで使用するカメラ */
	};
}