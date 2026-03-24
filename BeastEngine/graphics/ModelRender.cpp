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
		/** スケルトンの初期化 */
		InitSkeleton(filePath);
		/** アニメーションの初期化 */
		InitAnimation(animationClips, numAnimationClips, enModelUpAxiz);

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
		if (animationClips != nullptr) {
			modelInitData.m_skeleton = &m_skeleton;
		}

		/** シーンライト */
		modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
		modelInitData.m_expandConstantBufferSize = sizeof(Light);

		m_modelResource = ResourceManager::GetInstance().Load<ModelResource>(filePath);
		m_modelResource->SetInitData(modelInitData);
	}


	void ModelRender::InitOcean(ModelInitData& initData, const char* tkmFilePath)
	{
		m_isFowardRender = true;
		m_enableReflection[ReflectLayer::enOcean] = false;

		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		m_frowardRenderModel.Init(initData);
		m_frowardRenderModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
	}


	void ModelRender::InitModelOnZprepass(const char* tkmFilePath, EnModelUpAxis modelUpAxis, bool isSkyCube)
	{
		ModelInitData modelInitData;
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_fxFilePath = "Assets/shader/ZPrepass.fx";
		modelInitData.m_modelUpAxis = modelUpAxis;

		modelInitData.m_vsEntryPointFunc = "VSMain";

		if (m_animationClips != nullptr)
		{
			modelInitData.m_skeleton = &m_skeleton;

			if (m_isEnableInstancingDraw) {
				modelInitData.m_vsSkinEntryPointFunc = "VSSkinInstancingMain";
			}
			else {
				modelInitData.m_vsSkinEntryPointFunc = "VSSkinMain";
			}
		}
		else
		{
			if (m_isEnableInstancingDraw) {
				modelInitData.m_vsEntryPointFunc = "VSInstancingMain";
			}
			else {
				modelInitData.m_vsEntryPointFunc = "VSMain";
			}
		}

		if (isSkyCube) {
			modelInitData.m_psEntryPointFunc = "PSSkyCubeMain";
		}

		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (m_isEnableInstancingDraw) {
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}

		m_zprepassModel.Init(modelInitData);
	}


	void ModelRender::OnRenderShadowMap(RenderContext& rc)
	{
		m_shadowModels.Draw(rc);
	}


	void ModelRender::InitSkeleton(const char* filePath)
	{
		std::string skeletonFilePath = filePath;
		int pos = (int)skeletonFilePath.find(".tkm");
		skeletonFilePath.replace(pos, 4, ".tks");
		m_skeleton.Init(skeletonFilePath.c_str());
	}


	void ModelRender::InitAnimation(AnimationClip* animationClips, int numAnimationClips, EnModelUpAxis enModelUpAxis)
	{
		m_animationClips = animationClips;
		m_numAnimationClips = numAnimationClips;
		if (m_animationClips != nullptr) {
			m_animation.Init(m_skeleton, m_animationClips, m_numAnimationClips);
		}
	}


	void ModelRender::SetupShaderEntryPointFunc(ModelInitData& modelInitData)
	{
		modelInitData.m_vsEntryPointFunc = "VSMain";
		modelInitData.m_vsSkinEntryPointFunc = "VSMain";
		if (m_animationClips != nullptr) {
			modelInitData.m_vsSkinEntryPointFunc = "VSSkinMain";
		}
	}


	void ModelRender::UpdateWorldMatrixInModes()
	{
		m_modelResource->GetModel()->UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_shadowModels.UpdateWorldMatrix(m_position, m_rotation, m_scale);
	}


	void ModelRender::Update()
	{
		if (m_modelResource->IsCompleted() == false) return;

		UpdateWorldMatrixInModes();

		if (m_skeleton.IsInited()) {
			m_skeleton.Update(m_modelResource->GetModel()->GetWorldMatrix());
		}

		if (m_animation.IsInited()) {
			m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
		}
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		if (m_modelResource->IsCompleted() == false) return;

		if (m_isFowardRender) {
			g_renderingEngine->RegisterModel(&m_frowardRenderModel);
		}
		else {
			g_renderingEngine->RegisterModel(m_modelResource->GetModel());
		}
		g_renderingEngine->AddRenderObject(this);
	}
}