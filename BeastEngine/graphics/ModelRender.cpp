#include "BeastEnginePreCompile.h"
#include "ModelRender.h"


namespace nsBeastEngine
{
	ModelRender::ModelRender()
		: m_animationClips(nullptr)
		, m_numAnimationClips(0)
		, m_isEnableInstancingDraw(false)
		, m_maxInstance(1)
	{
	}


	ModelRender::~ModelRender()
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
		//InitModelOnRenderGBuffer(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);

		// ZPrepass描画用のモデルを初期化。
		//InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);

		// シャドウマップ描画用のモデルを初期化。
		//InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);

		//// 幾何学データを初期化。
		//InitGeometryDatas(maxInstance);

		//// 各種ワールド行列を更新する。
		//UpdateWorldMatrixInModes();

		//if (m_isRaytracingWorld) {
		//	// レイトレワールドに追加。
		//	g_renderingEngine->AddModelToRaytracingWorld(m_renderToGBufferModel);
		//	m_addRaytracingWorldModel = &m_renderToGBufferModel;
		//}
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
		/**
		 * TODO: アニメーションクリップを配列で渡す
		 * アロケーションアレイ
		 */

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


	void ModelRender::InitComputeAnimatoinVertexBuffer(
		const char* tkmFilePath,
		EnModelUpAxis enModelUpAxis)
	{
		StructuredBuffer* worldMatrxiArraySB = nullptr;
		if (m_isEnableInstancingDraw) {
			worldMatrxiArraySB = &m_worldMatrixArraySB;
		}

		//m_computeAnimationVertexBuffer.Init(
		//	tkmFilePath,
		//	m_skeleton.GetNumBones(),
		//	m_skeleton.GetBoneMatricesTopAddress(),
		//	enModelUpAxis,
		//	m_maxInstance,
		//	worldMatrxiArraySB
		//);
	}


	//void ModelRender::InitModelOnRenderGBuffer(RenderingEngine& renderingEngine, const char* tkmFilePath, EnModelUpAxis enModelUpAxis, bool isShadowReciever)
	//{
	//	ModelInitData modelInitData;
	//	modelInitData.m_fxFilePath = "Assets/shader/preProcess/RenderToGBufferFor3DModel.fx";

	//	/** 頂点シェーダーのエントリーポイントをセットアップ */
	//	SetupVertexShaderEntryPointFunc(modelInitData);
	//	/** 頂点の事前計算処理を使う */
	//	modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;
	//	if (m_animationClips != nullptr) {
	//		/** スケルトンを指定する */
	//		modelInitData.m_skeleton = &m_skeleton;
	//	}

	//	if (isShadowReciever) {
	//		modelInitData.m_psEntryPointFunc = "PSMainShadowReciever";
	//	}
	//	/** モデルの上方向を指定する */
	//	modelInitData.m_modelUpAxis = enModelUpAxis;

	//	modelInitData.m_tkmFilePath = tkmFilePath;
	//	modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_SNORM;
	//	modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

	//	if (m_isEnableInstancingDraw) {
	//		/** インスタンシング描画を行う場合は、拡張SRVにインスタンシング描画用のデータを設定する */
	//		modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
	//	}
	//	m_renderToGBufferModel.Init(modelInitData);
	//}


	//void ModelRender::SetupVertexShaderEntryPointFunc(ModelInitData& modelInitData)
	//{
	//	modelInitData.m_vsSkinEntryPointFunc = "VSMainUsePreComputedVertexBuffer";
	//	modelInitData.m_vsEntryPointFunc = "VSMainUsePreComputedVertexBuffer";

	//	if (m_animationClips != nullptr) {
	//		// アニメーションあり。
	//		modelInitData.m_vsSkinEntryPointFunc = "VSMainSkinUsePreComputedVertexBuffer";
	//	}
	//}


	//void ModelRender::InitModelOnZprepass(
	//	RenderingEngine& renderingEngine,
	//	const char* tkmFilePath,
	//	EnModelUpAxis modelUpAxis
	//)
	//{
	//	ModelInitData modelInitData;
	//	modelInitData.m_tkmFilePath = tkmFilePath;
	//	modelInitData.m_fxFilePath = "Assets/shader/preProcess/ZPrepass.fx";
	//	modelInitData.m_modelUpAxis = modelUpAxis;

	//	// 頂点シェーダーのエントリーポイントをセットアップ。
	//	SetupVertexShaderEntryPointFunc(modelInitData);
	//	// 頂点の事前計算処理を使う。
	//	modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

	//	if (m_animationClips != nullptr) {
	//		//スケルトンを指定する。
	//		modelInitData.m_skeleton = &m_skeleton;
	//	}

	//	modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	if (m_isEnableInstancingDraw) {
	//		// インスタンシング描画を行う場合は、拡張SRVにインスタンシング描画用のデータを設定する。
	//		modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
	//	}

	//	m_zprepassModel.Init(modelInitData);
	//}


	//void ModelRender::InitModelOnShadowMap(
	//	RenderingEngine& renderingEngine,
	//	const char* tkmFilePath,
	//	EnModelUpAxis modelUpAxis,
	//	bool isFrontCullingOnDrawShadowMap
	//)
	//{
	//	ModelInitData modelInitData;
	//	modelInitData.m_tkmFilePath = tkmFilePath;
	//	modelInitData.m_modelUpAxis = modelUpAxis;
	//	modelInitData.m_cullMode = isFrontCullingOnDrawShadowMap ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK;
	//	// 頂点シェーダーのエントリーポイントをセットアップ。
	//	SetupVertexShaderEntryPointFunc(modelInitData);

	//	// 頂点の事前計算処理を使う。
	//	modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

	//	if (m_animationClips != nullptr) {
	//		//スケルトンを指定する。
	//		modelInitData.m_skeleton = &m_skeleton;
	//	}

	//	modelInitData.m_fxFilePath = "Assets/shader/preProcess/DrawShadowMap.fx";
	//	if (g_renderingEngine->IsSoftShadow()) {
	//		modelInitData.m_colorBufferFormat[0] = g_softShadowMapFormat.colorBufferFormat;
	//	}
	//	else {
	//		modelInitData.m_colorBufferFormat[0] = g_hardShadowMapFormat.colorBufferFormat;
	//	}

	//	if (m_isEnableInstancingDraw) {
	//		// インスタンシング描画を行う場合は、拡張SRVにインスタンシング描画用のデータを設定する。
	//		modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
	//	}

	//	for (int ligNo = 0; ligNo < /*MAX_DIRECTIONAL_LIGHT*/NUM_SHADOW_LIGHT; ligNo++)
	//	{
	//		ConstantBuffer* cameraParamCBArray = m_drawShadowMapCameraParamCB[ligNo];
	//		Model* shadowModelArray = m_shadowModels[ligNo];
	//		for (int shadowMapNo = 0; shadowMapNo < NUM_SHADOW_MAP; shadowMapNo++) {
	//			cameraParamCBArray[shadowMapNo].Init(sizeof(Vector4), nullptr);
	//			modelInitData.m_expandConstantBuffer = &cameraParamCBArray[shadowMapNo];
	//			modelInitData.m_expandConstantBufferSize = sizeof(Vector4);
	//			shadowModelArray[shadowMapNo].Init(modelInitData);
	//		}
	//	}
	//}


	void ModelRender::Update()
	{
		// アニメーションの更新などをここで行う
		// 例: m_animation.Progress(g_gameTime->GetFrameDeltaTime());

		// とりあえず今は空でもOK
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		// ここで描画コマンドを発行する

		// まだ実装していないなら空でもOK
	}
}