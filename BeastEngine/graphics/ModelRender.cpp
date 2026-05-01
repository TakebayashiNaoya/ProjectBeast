/**
 * @file ModelRender.cpp
 * @brief モデルレンダーの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "ModelRender.h"


namespace nsBeastEngine
{
	void ModelRender::Init(
		const char* filePath,
		AnimationClip* animationClips,
		int numAnimationClips,
		bool islighting,
		EnModelUpAxis enModelUpAxiz)
	{
		// スケルトン初期化
		InitSkeleton(filePath);
		m_skeletonRef = &m_skeleton;
		// アニメーション初期化
		InitAnimation(animationClips, numAnimationClips, enModelUpAxiz);

		ModelInitData modelInitData;
		modelInitData.m_tkmFilePath = filePath;
		modelInitData.m_modelUpAxis = enModelUpAxiz;

		// シェーダー設定
		if (islighting) {
			modelInitData.m_fxFilePath = "Assets/shader/model.fx";
		}
		else {
			modelInitData.m_fxFilePath = "Assets/shader/lightOffModel.fx";
		}

		// アニメーションがある場合はスケルトンを指定
		if (animationClips != nullptr) {
			modelInitData.m_skeleton = &m_skeleton;
		}

		// シェーダーのエントリーポイント設定
		SetupShaderEntryPointFunc(modelInitData);

		// シーンライト
		modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
		modelInitData.m_expandConstantBufferSize = sizeof(Light);

		// モデル初期化
		m_model.Init(modelInitData);

		if (m_isForwardRender) {
			m_forwardRenderModel.Init(modelInitData);
		}
		else {
			InitRenderToGBufferModel(modelInitData);
		}
	}


	void ModelRender::InitFromLoaded(
		const ModelInitData& initData,
		Skeleton* skeleton,
		AnimationClip* animationeClips,
		int numAnimationClips)
	{
		// 外部ロード済みデータをローカルに反映
		m_animationClips = animationeClips;
		m_numAnimationClips = numAnimationClips;

		ModelInitData modelInitData = initData; // コピーしてローカル調整

		// シェーダーのエントリーポイント設定（アニメ有無でスキン用を切替）
		SetupShaderEntryPointFunc(modelInitData);

		// スケルトンが渡された場合は参照する（コピーしない）
		m_skeletonRef = skeleton;
		if (m_skeletonRef != nullptr) {
			modelInitData.m_skeleton = m_skeletonRef;
		}

		// シーンライトが未設定ならデフォルトを補完
		if (modelInitData.m_expandConstantBuffer == nullptr) {
			modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
			modelInitData.m_expandConstantBufferSize = sizeof(Light);
		}

		// モデル初期化
		m_model.Init(modelInitData);

		if (m_isForwardRender)
		{
			// フォワードレンダリング用のモデルを初期化する
			m_forwardRenderModel.Init(modelInitData);
		}
		else
		{
			// GBuffer描画用のモデルを初期化する
			InitRenderToGBufferModel(modelInitData);
		}

		// アニメーション初期化
		if (m_animationClips != nullptr && numAnimationClips > 0 && m_skeletonRef != nullptr) {
			m_animation.Init(*m_skeletonRef, m_animationClips, numAnimationClips);
		}
	}


	void ModelRender::InitRenderToGBufferModel(const ModelInitData& baseInitData)
	{
		ModelInitData gBufferInitData = baseInitData;

		// GBuffer書き込み用シェーダーに差し替える
		gBufferInitData.m_fxFilePath = "Assets/shader/RenderToGBuffer.fx";

		// エントリーポイントを明示的に設定する
		gBufferInitData.m_vsEntryPointFunc = "VSMain";
		gBufferInitData.m_vsSkinEntryPointFunc = "VSMainSkin";

		// シャドウは現在未実装のためPSMainを使用
		// シャドウ実装時はPSMainShadowRecieverに切り替える
		gBufferInitData.m_psEntryPointFunc = "PSMain";

		// GBufferのカラーバッファフォーマットを設定する
		gBufferInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// アルベド
		gBufferInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_UNORM;		// 法線
		gBufferInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R8G8B8A8_UNORM;		// メタリックスムース

		// PBR補正パラメータをb2に設定する
		gBufferInitData.m_expandConstantBuffer2 = &m_pbrParam;
		gBufferInitData.m_expandConstantBufferSize2 = sizeof(PBRParam);

		m_renderToGBufferModel.Init(gBufferInitData);
	}


	void ModelRender::OnRenderShadowMap(RenderContext& rc)
	{
		m_shadowModels.Draw(rc);
	}


	void ModelRender::InitSkeleton(const char* filePath)
	{
		/** 一旦tkmのファイルパスを受け取る */
		std::string skeletonFilePath = filePath;
		/** パスの中に.tkmが何文字目にあるか探す */
		int pos = static_cast<int>(skeletonFilePath.find(".tkm"));
		/** .tkmを.tksに置き換える */
		skeletonFilePath.replace(pos, 4, ".tks");
		/** char型に変換してInit */
		m_skeleton.Init(skeletonFilePath.c_str());
	}


	void ModelRender::InitAnimation(AnimationClip* animtionClips, int numAnimationClips, EnModelUpAxis enModelUpAxis)
	{
		m_animationClips = animtionClips;
		m_numAnimationClips = numAnimationClips;
		if (m_animationClips != nullptr && m_skeletonRef != nullptr) {
			m_animation.Init(*m_skeletonRef, m_animationClips, numAnimationClips);
		}
	}


	void ModelRender::SetupShaderEntryPointFunc(ModelInitData& modelInitData)
	{
		modelInitData.m_vsEntryPointFunc = "VSMain";
		modelInitData.m_vsSkinEntryPointFunc = "VSMain";
		/** アニメーションがある場合 */
		if (m_animationClips != nullptr) {
			modelInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		}
	}


	void ModelRender::UpdateWorldMatrixInModes()
	{
		m_model.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_renderToGBufferModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_forwardRenderModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_shadowModels.UpdateWorldMatrix(m_position, m_rotation, m_scale);
	}


	void ModelRender::Update()
	{
		/** モデルのワールド行列を更新する */
		UpdateWorldMatrixInModes();

		/** スケルトンのボーン行列を更新する */
		if (m_skeletonRef != nullptr && m_skeletonRef->IsInited()) {
			m_skeletonRef->Update(m_model.GetWorldMatrix());
		}

		/** アニメーションを進める */
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		if (!m_visible) return;

		if (!m_isForwardRender) {
			// ディファードレンダリングで描画するなら
			g_renderingEngine->AddDeferredModelList(this);
		}
		else {
			// フォワードレンダリングで描画するなら
			g_renderingEngine->AddForwardModelList(this);
		}
	}


	void ModelRender::OnDraw(RenderContext& rc)
	{
		/** 描画が有効でない場合は処理しない */
		if (!m_visible) return;

		/** フォワードレンダリング用のモデルが有効な場合はそちらを描画し、そうでない場合は通常のモデルを描画する */
		if (m_isForwardRender) {
			m_forwardRenderModel.Draw(rc, m_maxInstance);
		}
		else {
			m_renderToGBufferModel.Draw(rc, m_maxInstance);
		}
	}
}