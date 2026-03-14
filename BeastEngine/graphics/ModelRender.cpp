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

		/** シャドウマップテクスチャ */
		//modelInitData.m_expandShaderResoruceView[0] = &g_renderingEngine->GetShadowMap();

		/** シーンライト */
		modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
		modelInitData.m_expandConstantBufferSize = sizeof(Light);

		m_modelResource = ResourceManager::GetInstance().Load<ModelResource>(filePath);
		m_modelResource->SetInitData(modelInitData);

		/** シャドウマップテクスチャ */
		//modelInitData.m_expandShaderResoruceView[0] = nullptr;

		//modelInitData.m_fxFilePath = "Assets/shader/DrawShadowMap.fx";
		//modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
		//modelInitData.m_expandConstantBuffer = &g_sceneLight->GetLVP();
		//m_shadowModels.Init(modelInitData);
	}


	void ModelRender::InitOcean(ModelInitData& initData, const char* tkmFilePath)
	{
		m_isFowardRender = true;
		m_enableReflection[ReflectLayer::enOcean] = false;

		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		m_frowardRenderModel.Init(initData);
		m_frowardRenderModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		//InitModelOnZprepass(tkmFilePath, enModelUpAxisZ);
		//InitInstancingDraw(1);
	}


	void ModelRender::InitModelOnZprepass(const char* tkmFilePath, EnModelUpAxis modelUpAxis, bool isSkyCube)
	{
		ModelInitData modelInitData;
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_fxFilePath = "Assets/shader/ZPrepass.fx";
		modelInitData.m_modelUpAxis = modelUpAxis;

		//ノンスキンメッシュ用の頂点シェーダーのエントリーポイントを指定する。
		modelInitData.m_vsEntryPointFunc = "VSMain";

		//アニメーションがあるならVSSkinMainを指定。
		if (m_animationClips != nullptr)
		{
			//スケルトンを指定する。
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
			// インスタンシング描画を行う場合は、拡張SRVにインスタンシング描画用のデータを設定する。
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
			m_animationResource = ResourceManager::GetInstance().Load<AnimationResource>("animationResource");
			m_animationResource->SetSkeleton(&m_skeleton);
			m_animationResource->SetAnimationClips(m_animationClips, m_numAnimationClips);
			//m_animationResource->GetAnimation()->Init(m_skeleton, m_animationClips, numAnimationClips);
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
		m_modelResource->GetModel()->UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_shadowModels.UpdateWorldMatrix(m_position, m_rotation, m_scale);
	}


	void ModelRender::Update()
	{
		/** モデルの読み込みが完了していたらアニメーションを進める */
		if (m_modelResource->IsCompleted() == false) return;

		/** モデルのワールド行列を更新する */
		UpdateWorldMatrixInModes();


		if (m_animationResource->IsCompleted() == false) return;

		/** スケルトンのボーン行列を更新する */
		if (m_skeleton.IsInited()) {
			m_skeleton.Update(m_modelResource->GetModel()->GetWorldMatrix());
		}

		/** アニメーションを進める */
		m_animationResource->GetAnimation()->Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		/** モデルの読み込みが完了していたらアニメーションを進める */
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