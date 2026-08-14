/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林
 */
#pragma once
#include "MyRenderer.h"
#include "Graphics/Light/SceneLight.h"
#include "Graphics/RenderViewContext.h"
#include "Nature/INatureObject.h"
#include "Graphics/ICustomRenderer.h"
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
		 * @brief メインビューを取得する
		 * @return メインビューの参照
		 */
		const RenderViewContext& GetMainView() const { return m_mainView; }

		/**
		 * @brief サブビューを取得する
		 * @return サブビューの参照
		 */
		const RenderViewContext& GetSubView() const { return m_subView; }

		/**
		 * @brief メインカメラのフラスタムを取得する
		 * @details WhirlpoolManagerなど、RenderingEngine外部から
		 *          カリング判定を行う場合に使用する。
		 * @return フラスタムの参照
		 */
		const Frustum& GetFrustum() const { return m_mainView.frustum; }

		/**
		 * @brief 現在描画中のビューのフラスタムを取得する
		 * @details ModelRender::OnDrawから参照される。
		 *          ExecuteViewPass前にm_activeFrustumがセットされる。
		 * @return アクティブフラスタムの参照
		 */
		const Frustum& GetActiveFrustum() const { return *m_activeFrustum; }

		/**
		 * @brief サブカメラ用レンダリングターゲットを取得する
		 * @return サブカメラ用レンダリングターゲットの参照
		 */
		RenderTarget& GetSubCameraRenderTarget() { return m_subView.renderTarget; }

		/**
		 * @brief ポストエフェクトマネージャーを取得する
		 * @details デバッグUIから実行中にパラメーターを調整するために使う
		 * @return ポストエフェクトマネージャーの参照
		 */
		PostEffectManager& GetPostEffectManager() { return m_postEffectManager; }


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

		/**
		 * @brief カスタムメッシュ描画オブジェクトを登録する
		 * @param renderer 登録する描画オブジェクト
		 */
		void RegisterCustomRenderer(ICustomRenderer* renderer)
		{
			m_customRenderers.push_back(renderer);
		}

		/**
		 * @brief カスタムメッシュ描画オブジェクトの登録を解除する
		 * @param renderer 登録解除する描画オブジェクト
		 */
		void UnregisterCustomRenderer(ICustomRenderer* renderer)
		{
			auto it = std::find(m_customRenderers.begin(), m_customRenderers.end(), renderer);
			if (it != m_customRenderers.end())
			{
				m_customRenderers.erase(it);
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

		/**
		 * @brief サブビュー用トーンマップの初期化
		 * @details メインビューはPostEffectManager内でブルームと合わせて処理するが、
		 *          小窓はトーンマップのみを掛けて色味をメインビューに合わせる。
		 */
		void InitSubViewToneMap();


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
		 * @brief ポストエフェクトの描画処理（ブルーム → トーンマップ）
		 * @details エフェクトを含む3D描画完了後・UI描画前に実行することで、
		 *          エフェクトもポストエフェクトの対象にしつつUIへの影響を防ぐ。
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
		/** メインカメラで映すビュー */
		RenderViewContext m_mainView;
		/** サブカメラで映すビュー */
		RenderViewContext m_subView;

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
		/**
		 * @brief サブビュー（小窓）専用のトーンマップ
		 * @details サブビューはPostEffectManagerとは別の解像度のRenderTargetを使うため、
		 *          メインビューと共有せず専用のインスタンスを持つ。
		 *          方式・露出はメインビューと同じ既定値で初期化するが、
		 *          デバッグUIからの調整はメインビュー側のみに反映される点に注意。
		 */
		ToneMap m_subViewToneMap;

		/** ディファードモデルリスト */
		std::vector<ModelRender*> m_deferredModelList;
		/** フォワードモデルリスト */
		std::vector<ModelRender*> m_forwardModelList;
		/** 描画するオブジェクトの予約リスト */
		std::vector<IRenderer*> m_renderObjects;
		/** 自然オブジェクトのリスト（Ocean・WhirlpoolManagerなど） */
		std::vector<INatureObject*> m_natureObjects;
		/** カスタムメッシュ描画オブジェクトのリスト */
		std::vector<ICustomRenderer*> m_customRenderers;

		/** フラスタムカリングの有効/無効 */
		bool m_frustumCullingEnabled = true;
		/** 現在描画中のビューのフラスタム（ExecuteViewPass前にセットされる） */
		const Frustum* m_activeFrustum = nullptr;
	};
}