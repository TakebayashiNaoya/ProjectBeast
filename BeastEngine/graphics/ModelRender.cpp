#include "BeastEnginePreCompile.h"
#include "ModelRender.h"


namespace nsBeastEngine
{
	ModelRender::ModelRender()
		: m_animationClips(nullptr)
		, m_numAnimationClips(0)
	{
	}


	void ModelRender::Init(
		const char* filePath,
		AnimationClip* animationClips,
		int numAnimationClips,
		EnModelUpAxis enModelUpAxis,
		bool isShadowReciever,
		int maxInstance,
		bool isFrontCullingOnDrawShadowMap)
	{
		// インスタンシング描画用のデータを初期化。
		// NOTE: 今は必要ないのでコメントアウト。
		//InitInstancingDraw(maxInstance);

		// スケルトンを初期化。
		InitSkeleton(filePath);

		// アニメーションを初期化。
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);

		// アニメーション済み頂点バッファの計算処理を初期化。
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);

		// GBuffer描画用のモデルを初期化。
		InitModelOnRenderGBuffer(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);

		// ZPrepass描画用のモデルを初期化。
		InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);

		// シャドウマップ描画用のモデルを初期化。
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);

		// 幾何学データを初期化。
		InitGeometryDatas(maxInstance);

		// 各種ワールド行列を更新する。
		UpdateWorldMatrixInModes();

		if (m_isRaytracingWorld) {
			// レイトレワールドに追加。
			g_renderingEngine->AddModelToRaytracingWorld(m_renderToGBufferModel);
			m_addRaytracingWorldModel = &m_renderToGBufferModel;
		}
	}


	void ModelRender::InitSkeleton(const char* filePath)
	{
		/** パスを保存 */
		std::string skeletonFilePath = filePath;
		/** ファイルパスの中から ".tkm" という文字を探し、何文字目かを "pos" に保存する */
		int pos = (int)skeletonFilePath.find(".tkm");
		/** 見つかった位置から4文字分を ".tks" に置き換える */
		skeletonFilePath.replace(pos, 4, ".tks");
		/** 書き換えたパスを使って、スケルトンの初期化 */
		m_skeleton.Init(skeletonFilePath.c_str());
	}


	void ModelRender::InitAnimation(AnimationClip* animationClips, int numAnimationClips, EnModelUpAxis enModelUpAxis)
	{
		/** アニメーションクリップを保存 */
		m_animationClips = animationClips;
		/** アニメーションクリップの数を保存 */
		m_numAnimationClips = numAnimationClips;
		/** アニメーションクリップが存在している場合 */
		if (m_animationClips) {
			/** アニメーションを初期化 */
			m_animation.Init(m_skeleton, m_animationClips, numAnimationClips);
		}
	}
}