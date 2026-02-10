/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林尚哉
 */
#pragma once
#include "MyRenderer.h"
#include "Graphics/Light/SceneLight.h"


namespace nsBeastEngine
{
	class RenderingEngine : public Noncopyable
	{
	public:
		RenderingEngine();
		~RenderingEngine();


	public:
		/**
		 * @brief レンダリングパイプラインの初期化
		 * @param isSoftShadow	ソフトシャドウを使用するかどうか
		 */
		void Init(bool isSoftShadow);

		/**
		 * @brief レイトレ用のライトデータ
		 */
		struct RaytracingLightData
		{
			DirectionalLight m_directionalLight;  /** ディレクショナルライト */
			Vector3 m_ambientLight;               /** 環境光（IBLテクスチャが指定されていない場合に利用される） */
			float m_iblIntencity;                 /** IBL強度 */
			int m_enableIBLTexture;               /** IBLテクスチャが指定されている */
		};
		/**
		 * @brief レイトレ用のライトデータを取得
		 * @return レイトレ用のライトデータ
		 */
		RaytracingLightData& GetRaytracingLightData()
		{
			return m_raytracingLightData;
		}


	private:
		/**
		 * @brief メインレンダリングターゲットの初期化
		 */
		void InitMainRenderTarget();


	private:
		/**	メインレンダリングターゲット */
		RenderTarget m_mainRenderTarget;
		/** レイトレ用のライトデータ */
		RaytracingLightData m_raytracingLightData;
	};
}