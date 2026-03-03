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
	class RenderingEngine
	{
	public:
		RenderingEngine();
		~RenderingEngine();

		/**
		 * @brief レンダリングエンジンの初期化
		 */
		void Init();

		/**
		 * @brief 更新
		 */
		void Update();

		/**
		 * @brief メインレンダリングターゲットの初期化
		 */
		void InitMainRenderTarget();

		/**
		 * @brief 2D描画用のレンダーターゲットを初期化
		 */
		void Init2DRenderTarget();

		/**
		 * @brief シャドウマップへの描画処理を初期化
		 */
		void InitShadowMapRender();

		/**
		 * @brief メインレンダリングターゲットのカラーバッファの内容をフレームバッファにコピーするためのスプライトを初期化する
		 */
		void InitCopyMainRenderTargetToFrameBufferSprite();

		/**
		 * @brief メインレンダリングターゲットの内容をフレームバッファにコピーする
		 * @param rc レンダリングコンテキスト
		 */
		void CopyMainRenderTargetToFrameBufferSprite(RenderContext& rc);

		/**
		 * @brief 溜まったリストを一気に描画する
		 * @param rc レンダリングコンテキスト
		 */
		void Execute(RenderContext& rc);

		/**
		 * @brief モデルをリストに登録する
		 * @param model 登録するモデル
		 */
		void RegisterModel(Model* model)
		{
			m_registerModels.push_back(model);
		}

		/**
		 * @brief 描画オブジェクトをリストに登録する（予約）
		 * @param renderObject 登録する描画オブジェクト
		 */
		void AddRenderObject(IRenderer* renderObject)
		{
			m_renderObjects.push_back(renderObject);
		}

		/**
		 * @brief 描画オブジェクトをリストから削除する
		 * @param renderObject 削除する描画オブジェクト
		 */
		void RemoveRenderObject(IRenderer* renderObject)
		{

		}

		/**
		 * @brief シャドウマップへの描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void RenderToShadowMap(RenderContext& rc);

		/**
		 * @brief 2D描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void Render2D(RenderContext& rc);

		/**
		 * @brief シャドウマップを取得
		 * @return シャドウマップのテクスチャ
		 */
		 //Texture& GetShadowMap()
		 //{
		 //	return m_shadowMapRender.GetTexture();
		 //}


	private:
		/** メインレンダリングターゲットをフレームバッファにコピーするためのスプライト */
		Sprite			m_copyMainRtToFrameBufferSprite;
		/** シーンライト */
		SceneLight		m_sceneLight;
		/** メインレンダリングターゲット */
		RenderTarget	m_mainRenderTarget;
		/** 2D描画用のレンダーターゲット */
		RenderTarget	m_2DRenderTarget;
		/** 2D合成用のスプライト */
		Sprite			m_2DSprite;

		Sprite			m_mainSprite;

		///** ポストエフェクト */
		//PostEffect		m_postEffect;
		///** シャドウマップ */
		//ShadowMapRender m_shadowMapRender;

		/** 登録されたモデルのリスト */
		std::vector<Model*>		m_registerModels;
		/** 描画するオブジェクトの予約リスト */
		std::vector<IRenderer*> m_renderObjects;
	};
}