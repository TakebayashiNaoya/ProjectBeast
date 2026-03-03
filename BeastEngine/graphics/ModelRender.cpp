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
		AnimationClip* animationeClips,
		int numAnimationClips,
		bool islighting,
		EnModelUpAxis enModelUpAxiz)
	{
		/** スケルトンの初期化 */
		InitSkeleton(filePath);
		/** アニメーションの初期化 */
		InitAnimation(animationeClips, numAnimationClips, enModelUpAxiz);

		ModelInitData modelInitData;
		/** tkmファイルのファイルパスの指定 */
		modelInitData.m_tkmFilePath = filePath;
		/** シェーダーのファイルパスの指定 */
		if (islighting) {
			modelInitData.m_fxFilePath = "Assets/shader/model.fx";
		}
		else {
			modelInitData.m_fxFilePath = "Assets/shader/lightOffModel.fx";
		}
		/** シェーダーのエントリーポイントの設定 */
		SetupShaderEntryPointFunc(modelInitData);
		/** アニメーションがある場合はスケルトンを渡す */
		if (animationeClips != nullptr) {
			modelInitData.m_skeleton = &m_skeleton;
		}

		/** シャドウマップテクスチャ */
		//modelInitData.m_expandShaderResoruceView[0] = &g_renderingEngine->GetShadowMap();

		/** シーンライト */
		modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
		modelInitData.m_expandConstantBufferSize = sizeof(Light);
		m_model.Init(modelInitData);

		/** シャドウマップテクスチャ */
		//modelInitData.m_expandShaderResoruceView[0] = nullptr;

		//modelInitData.m_fxFilePath = "Assets/shader/DrawShadowMap.fx";
		//modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
		//modelInitData.m_expandConstantBuffer = &g_sceneLight->GetLVP();
		//m_shadowModels.Init(modelInitData);
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
		int pos = (int)skeletonFilePath.find(".tkm");
		/** .tkmを.tksに置き換える */
		skeletonFilePath.replace(pos, 4, ".tks");
		/** char型に変換してInit */
		m_skeleton.Init(skeletonFilePath.c_str());
	}


	void ModelRender::InitAnimation(AnimationClip* animtionClips, int numAnimationClips, EnModelUpAxis enModelUpAxis)
	{
		m_animationClips = animtionClips;
		m_numAnimationClips = numAnimationClips;
		if (m_animationClips != nullptr) {
			m_animation.Init(m_skeleton, m_animationClips, numAnimationClips);
		}
	}


	void ModelRender::SetupShaderEntryPointFunc(ModelInitData& modelInitData)
	{
		modelInitData.m_vsEntryPointFunc = "VSMain";
		modelInitData.m_vsSkinEntryPointFunc = "VSMain";
		/** アニメーションがある場合 */
		if (m_animationClips != nullptr) {
			modelInitData.m_vsSkinEntryPointFunc = "VSSkinMain";
		}
	}


	void ModelRender::UpdateWorldMatrixInModes()
	{
		m_model.UpdateWorldMatrix(m_pos, m_rot, m_sca);
		m_shadowModels.UpdateWorldMatrix(m_pos, m_rot, m_sca);
	}


	void ModelRender::Update()
	{
		/** モデルのワールド行列を更新する */
		UpdateWorldMatrixInModes();

		/** スケルトンのボーン行列を更新する */
		if (m_skeleton.IsInited()) {
			m_skeleton.Update(m_model.GetWorldMatrix());
		}

		/** アニメーションを進める */
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		g_renderingEngine->RegisterModel(&m_model);
		g_renderingEngine->AddRenderObject(this);
	}
}