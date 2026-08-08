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
	 * @brief ディファードライティング用スプライトのSRVスロット
	 * @details SpriteInitData::m_textures の添字が、そのままシェーダーの
	 *          レジスタ番号（t0〜）になる。
	 *          DeferredLighting.fx のレジスタ宣言と必ず一致させること。
	 * @details GBufferの数（enGBuffer_Num）を添字に流用しないこと。
	 *          GBufferを増やしたときにシャドウマップのレジスタが黙ってずれる。
	 */
	enum EnDeferredLightingSrv
	{
		enDeferredLightingSrv_Albedo = 0,  /** アルベド		 (t0) */
		enDeferredLightingSrv_Normal,      /** 法線			 (t1) */
		enDeferredLightingSrv_Specular,    /** PBRパラメータ (t2) */
		enDeferredLightingSrv_ShadowMap,   /** シャドウマップ(t3) */
		enDeferredLightingSrv_Num,         /** スロットの数	     */
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