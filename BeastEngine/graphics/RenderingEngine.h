/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林
 */
#pragma once
#include "MyRenderer.h"
#include "Graphics/Light/SceneLight.h"
#include "Nature/INatureObject.h"
#include "Geometry/Frustum.h"
#include "Graphics/PostEffect/PostEffectManager.h"
#include "Graphics/PostEffect/PostEffectTypes.h"


namespace nsBeastEngine
{
	/** 前方宣言 */
	class ModelRender;


	/**
	 * @brief レンダリングエンジン
	 * @details 描画オブジェクトを登録して、まとめて描画するクラス
	 */
	class RenderingEngine
	{
	private:
		/** GBufferに入れるレンダリングターゲットの役割 */
		enum EnGBuffer
		{
			enGBuffer_Albedo = 0,  /** アルベド		*/
			enGBuffer_Normal,      /** 法線			*/
			enGBuffer_Specular,    /** スペキュラ   */
			enGBuffer_Num,         /** G-Bufferの数 */
		};

		/** 描画に使用するリソース */
		struct RenderViewContext
		{
			UINT width = 0;		/** 幅 */
			UINT height = 0;	/** 高さ */

			std::array<RenderTarget, enGBuffer_Num> gBuffer;	/** GBufferのレンダリングターゲット */

			RenderTarget	renderTarget;			/** レンダリングターゲット */
			Sprite			deferredLightingSprite;	/** ディファードシェーディング用のスプライト */
			Frustum			frustum;				/** フラスタム */
		};

		/** メインカメラで映すビュー */
		RenderViewContext m_mainView;
		/** サブカメラで映すビュー */
		RenderViewContext m_subView;


	public:
		RenderingEngine();
		~RenderingEngine();

		/**
		 * @brief レンダリングエンジンの初期化
		 * @details BeastEngine::Init()から呼ばれる
		 */
		void Init();

		/**
		 * @brief 更新
		 * @details mainから呼ばれる
		 */
		void Update();

		/**
		 * @brief 溜まったリストを一気に描画する
		 * @details BeastEngine::Execute()から呼ばれる
		 * @param rc レンダリングコンテキスト
		 */
		void Execute(RenderContext& rc);

		/**
		 * @brief シーンライトの取得
		 * @return シーンライト
		 */
		SceneLight& GetSceneLight() { return m_sceneLight; }

		/**
		 * @brief メインカメラのフラスタムを取得する
		 * @details WhirlpoolManagerなど、RenderingEngine外部から
		 *          カリング判定を行う場合に使用する。
		 * @return フラスタムの参照
		 */
		const Frustum& GetFrustum() const { return m_mainView.frustum; }

		/**
		 * @brief サブカメラ用レンダリングターゲットを取得する
		 * @details SubCameraManager::RenderToScreen()でスプライトに貼り付けるために使用する
		 * @return サブカメラ用レンダリングターゲットの参照
		 */
		RenderTarget& GetSubCameraRenderTarget() { return m_subView.renderTarget; }


		//============================================//
		// 登録・解除用の関数
		//============================================//

	public:
		/**
		 * @brief ディファードモデルリストに描画オブジェクトを追加する
		 * @param modelRender 追加する描画オブジェクト
		 */
		void AddDeferredModelList(ModelRender* modelRender)
		{
			m_deferredModelList.push_back(modelRender);
		}

		/**
		 * @brief フォワードモデルリストに描画オブジェクトを追加する
		 * @param modelRender 追加する描画オブジェクト
		 */
		void AddForwardModelList(ModelRender* modelRender)
		{
			m_forwardModelList.push_back(modelRender);
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
		 * @brief 自然オブジェクトを登録する
		 * @param obj 登録する自然オブジェクト
		 */
		void RegisterNatureObject(INatureObject* obj)
		{
			m_natureObjects.push_back(obj);
		}

		/**
		 * @brief 自然オブジェクトの登録を解除する
		 * @param obj 登録解除する自然オブジェクト
		 */
		void UnregisterNatureObject(INatureObject* obj)
		{
			auto it = std::find(m_natureObjects.begin(), m_natureObjects.end(), obj);
			if (it != m_natureObjects.end())
			{
				m_natureObjects.erase(it);
			}
		}


		//============================================//
		// Init内で呼ばれる初期化処理
		//============================================//

	private:
		/**
		 * @brief GBufferの初期化
		 * @param view 初期化対象のビュー
		 */
		void InitGBuffer(RenderViewContext& view);

		/**
		 * @brief ディファードシェーディングを行うためのスプライトの初期化
		 * @param view 初期化対象のビュー
		 */
		void InitDeferredLightingSprite(RenderViewContext& view);

		/**
		 * @brief メインレンダリングターゲットのカラーバッファの内容を
		 *		  フレームバッファにコピーするためのスプライトを初期化する
		 */
		void InitCopyMainRenderTargetToFrameBufferSprite();

		/**
		 * @brief 2D描画用のレンダーターゲットを初期化
		 */
		void Init2DRenderTarget();

		/**
		 * @brief ポストエフェクトマネージャーの初期化
		 */
		void InitPostEffectManager();


		//============================================//
		// Execute内で呼ばれる描画処理
		//============================================//

	private:
		/**
		 * @brief 指定したビューの描画パスを実行する
		 * @param rc レンダリングコンテキスト
		 * @param view 描画に使用するビュー
		 */
		void ExecuteViewPass(RenderContext& rc, RenderViewContext& view);

		/**
		 * @brief GBufferへの描画処理
		 * @param rc レンダリングコンテキスト
		 * @param view 使用するビュー
		 */
		void RenderToGBuffer(RenderContext& rc, RenderViewContext& view);

		/**
		 * @brief ディファードライティングの描画処理
		 * @param rc レンダリングコンテキスト
		 * @param view 使用するビュー
		 */
		void DeferredLighting(RenderContext& rc, RenderViewContext& view);

		/**
		 * @brief フォワードレンダリングの描画処理
		 * @param rc レンダリングコンテキスト
		 * @param view 使用するビュー
		 */
		void ForwardRendering(RenderContext& rc, RenderViewContext& view);

		/**
		 * @brief 自然オブジェクトの描画処理
		 * @param rc レンダリングコンテキスト
		 * @param view 使用するビュー
		 */
		void RenderNatureObjects(RenderContext& rc, RenderViewContext& view);

		/**
		 * @brief ポストエフェクトの描画処理
		 * @details 3D描画完了後・UI描画前に実行することでUIへの影響を防ぐ。
		 * @param rc レンダリングコンテキスト
		 */
		void PostEffect(RenderContext& rc);

		/**
		 * @brief 2D描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void Render2D(RenderContext& rc);

		/**
		 * @brief メインレンダリングターゲットの内容をフレームバッファにコピーする
		 * @param rc レンダリングコンテキスト
		 */
		void CopyMainRenderTargetToFrameBufferSprite(RenderContext& rc);


	private:
		/** シーンライト */
		SceneLight		m_sceneLight;
		/** メインレンダリングターゲットをフレームバッファにコピーするためのスプライト */
		Sprite			m_copyMainRtToFrameBufferSprite;
		/** 2D描画用のレンダーターゲット */
		RenderTarget	m_2DRenderTarget;
		/** 2D合成用のスプライト */
		Sprite			m_2DSprite;
		/** 3D描画結果のスプライト */
		Sprite			m_mainSprite;

		/** ポストエフェクトマネージャー */
		PostEffectManager m_postEffectManager;

		/** ディファードモデルリスト */
		std::vector<ModelRender*> m_deferredModelList;
		/** フォワードモデルリスト */
		std::vector<ModelRender*> m_forwardModelList;
		/** 描画するオブジェクトの予約リスト */
		std::vector<IRenderer*> m_renderObjects;
		/** 自然オブジェクトのリスト（Ocean・WhirlpoolManagerなど） */
		std::vector<INatureObject*> m_natureObjects;

		/** フラスタムカリングの有効/無効 */
		bool m_frustumCullingEnabled = true;
	};
}