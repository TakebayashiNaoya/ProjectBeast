/**
 * @file MyRenderer.h
 * @brief レンダラー関連のヘッダー
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief レンダリングターゲットのフォーマット
	 * @details カラーバッファと深度バッファのフォーマットをまとめた構造体
	 */
	struct RenderTargetFormat {
		DXGI_FORMAT colorBufferFormat;	/** カラーバッファのフォーマット */
		DXGI_FORMAT depthBufferFormat;	/** 深度バッファのフォーマット */
	};


	/** メインレンダリングターゲットのフォーマット */
	const RenderTargetFormat g_mainRenderTargetFormat = {
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_UNKNOWN
	};
}