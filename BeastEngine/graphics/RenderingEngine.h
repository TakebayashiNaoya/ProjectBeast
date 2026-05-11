/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林
 */
#pragma once
#include "MyRenderer.h"
#include "Graphics/Light/SceneLight.h"
#include "Nature/INatureObject.h"
#include "geometry/Frustum.h"


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
		 * @brief フラスタムを取得する
		 * @details WhirlpoolManagerなど、RenderingEngine外部から
		 *          カリング判定を行う場合に使用する。
		 * @return フラスタムの参照
		 */
		const Frustum& GetFrustum() const { return m_frustum; }


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
		 * @brief メインレンダリングターゲットの初期化
		 */
		void InitMainRenderTarget();

		/**
		 * @brief GBufferの初期化
		 */
		void InitGBuffer();

		/**
		 * @brief ディファードシェーディングを行うためのスプライトの初期化
		 */
		void InitDeferredLightingSprite();

		/**
		 * @brief メインレンダリングターゲットのカラーバッファの内容を
		 *		  フレームバッファにコピーするためのスプライトを初期化する
		 */
		void InitCopyMainRenderTargetToFrameBufferSprite();

		/**
		 * @brief 2D描画用のレンダーターゲットを初期化
		 */
		void Init2DRenderTarget();


		//============================================//
		// Execute内で呼ばれる描画処理
		//============================================//

	private:
		/**
		 * @brief GBufferへの描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void RenderToGBuffer(RenderContext& rc);

		/**
		 * @brief ディファードライティングの描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void DeferredLighting(RenderContext& rc);

		/**
		 * @brief フォワードレンダリングの描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void ForwardRendering(RenderContext& rc);

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
		/** GBufferに入れるレンダリングターゲットの役割 */
		enum EnGBuffer
		{
			enGBuffer_Albedo = 0,  /** アルベド		*/
			enGBuffer_Normal,      /** 法線			*/
			enGBuffer_Specular,    /** スペキュラ   */
			enGBuffer_Num,         /** G-Bufferの数 */
		};
		/** GBuffer用のレンダリングターゲット */
		std::array<RenderTarget, enGBuffer_Num> m_gBuffer;

		/** シーンライト */
		SceneLight		m_sceneLight;
		/** ディファードライティング用のスプライト */
		Sprite			m_deferredLightingSprite;
		/** メインレンダリングターゲットをフレームバッファにコピーするためのスプライト */
		Sprite			m_copyMainRtToFrameBufferSprite;
		/** メインレンダリングターゲット */
		RenderTarget	m_mainRenderTarget;
		/** 2D描画用のレンダーターゲット */
		RenderTarget	m_2DRenderTarget;
		/** 2D合成用のスプライト */
		Sprite			m_2DSprite;
		/** 3D描画結果のスプライト */
		Sprite			m_mainSprite;

		/** ディファードモデルリスト */
		std::vector<ModelRender*> m_deferredModelList;
		/** フォワードモデルリスト */
		std::vector<ModelRender*> m_forwardModelList;
		/** 描画するオブジェクトの予約リスト */
		std::vector<IRenderer*> m_renderObjects;
		/** 自然オブジェクトのリスト（Ocean・WhirlpoolManagerなど） */
		std::vector<INatureObject*> m_natureObjects;

		/** フラスタム（視錐台）カリング用 */
		Frustum m_frustum;
	};
}