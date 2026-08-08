/**
 * @file ModelRender.cpp
 * @brief モデルレンダーの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "ModelRender.h"
#include "Graphics/BeastModel.h"


namespace nsBeastEngine
{
	namespace
	{
		/** シャドウマップ書き込み用シェーダーのファイルパス */
		constexpr const char* SHADOW_MAP_FX_PATH = "Assets/shader/shadowMap.fx";
	}


	bool ModelRender::s_isToonGlobalEnabled = false;


	ModelRender::ModelRender()
		: m_position(Vector3::Zero)
		, m_scale(Vector3::One)
		, m_rotation(Quaternion::Identity)
		, m_animationClips(nullptr)
		, m_maxInstance(1)
		, m_numAnimationClips(0)
		, m_animationSpeed(1.0f)
		, m_renderToGBufferModel(std::make_unique<BeastModel>())
		, m_forwardRenderModel(std::make_unique<BeastModel>())
	{}

	ModelRender::~ModelRender() = default;


	void ModelRender::SetMulColor(const Vector4& mulColor)
	{
		m_model.SetMulColor(mulColor);
		m_renderToGBufferModel->SetMulColor(mulColor);
		m_forwardRenderModel->SetMulColor(mulColor);

		if (m_toonModel != nullptr)
		{
			m_toonModel->SetMulColor(mulColor);
		}
		if (m_outlineModel != nullptr)
		{
			m_outlineModel->SetMulColor(mulColor);
		}
	}


	void ModelRender::SetAlpha(const float alpha)
	{
		m_model.SetAlpha(alpha);
		m_renderToGBufferModel->SetAlpha(alpha);
		m_forwardRenderModel->SetAlpha(alpha);

		if (m_toonModel != nullptr)
		{
			m_toonModel->SetAlpha(alpha);
		}
		if (m_outlineModel != nullptr)
		{
			m_outlineModel->SetAlpha(alpha);
		}

		// GBufferパスは本来アルファブレンドができないため、
		// モデル単位ディザリング（b4）の透過率として代わりに反映する
		m_modelDitherCb.modelDitherAlpha = 1.0f - alpha;
	}


	void ModelRender::SetExpandConstantBuffer2(void* data)
	{
		// GBufferモデルはb2をPBRParamで使用しているため設定しない
		m_model.SetExpandData2(data);
		m_forwardRenderModel->SetExpandData2(data);
	}


	void ModelRender::SetExpandConstantBuffer3(void* data)
	{
		// GBufferパスのディザリングはb3を使用する
		m_renderToGBufferModel->SetExpandData3(data);
	}


	void ModelRender::Init(
		const char* filePath,
		AnimationClip* animationClips,
		int numAnimationClips,
		bool islighting,
		EnModelUpAxis enModelUpAxiz)
	{
		// グローバルフラグが有効、かつフォワードレンダリング指定でない場合のみ
		// 個別フラグに反映する。
		// SetForwardRendering(true)を呼んでいるモデル（SkyCubeなど）は
		// 独自の描画パスを持つためトゥーンの対象外とする。
		if (s_isToonGlobalEnabled && !m_isForwardRender)
		{
			m_isToonEnabled = true;
		}

		// スケルトン初期化
		InitSkeleton(filePath);
		m_skeletonRef = &m_skeleton;
		// アニメーション初期化
		InitAnimation(animationClips, numAnimationClips, enModelUpAxiz);

		ModelInitData modelInitData;
		modelInitData.m_tkmFilePath = filePath;
		modelInitData.m_modelUpAxis = enModelUpAxiz;

		// シェーダー設定
		if (islighting)
		{
			modelInitData.m_fxFilePath = "Assets/shader/model.fx";
			// model.fxはSetAlpha()による透明化に対応するため常に半透明ブレンドを有効にする
			modelInitData.m_alphaBlendMode = AlphaBlendMode_Trans;
		}
		else
		{
			modelInitData.m_fxFilePath = "Assets/shader/lightOffModel.fx";
		}

		// アニメーションがある場合はスケルトンを指定
		if (animationClips != nullptr)
		{
			modelInitData.m_skeleton = &m_skeleton;
		}

		// シェーダーのエントリーポイント設定
		SetupShaderEntryPointFunc(modelInitData);

		// シーンライト
		modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
		modelInitData.m_expandConstantBufferSize = sizeof(Light);

		// 影モデル・GetModel()用の k2EngineLow::Model を初期化する
		m_model.Init(modelInitData);

		if (m_isForwardRender)
		{
			m_forwardRenderModel->Init(modelInitData);
		}

		// GBuffer・トゥーンの各モデルはブレンド対象外のため元のブレンドモードに戻す
		modelInitData.m_alphaBlendMode = AlphaBlendMode_None;

		if (!m_isForwardRender)
		{
			InitRenderToGBufferModel(modelInitData);
		}

		// トゥーン有効なら追加モデルを初期化する
		if (IsToonEnabled())
		{
			InitToonModels(modelInitData);
		}

		// シャドウマップ描画用モデルを初期化する
		// トゥーン・フォワードのモデルも影は落とすため、分岐せず常に作る
		InitShadowModel(modelInitData);

		// スケルトンを持つか記録する
		m_hasSkeleton = m_skeletonRef->IsInited();

		// ローカルAABBをtkmから計算する（スケルトンなしの場合に使用）
		CalcLocalAABBFromTkm(filePath);
	}


	void ModelRender::InitFromLoaded(
		const ModelInitData& initData,
		Skeleton* skeleton,
		AnimationClip* animationeClips,
		int numAnimationClips)
	{
		// グローバルフラグが有効、かつフォワードレンダリング指定でない場合のみ
		// 個別フラグに反映する。
		if (s_isToonGlobalEnabled && !m_isForwardRender)
		{
			m_isToonEnabled = true;
		}

		// 外部ロード済みデータをローカルに反映
		m_animationClips = animationeClips;
		m_numAnimationClips = numAnimationClips;

		ModelInitData modelInitData = initData;

		// シェーダーのエントリーポイント設定（アニメ有無でスキン用を切替）
		SetupShaderEntryPointFunc(modelInitData);

		// スケルトンが渡された場合は参照する（コピーしない）
		m_skeletonRef = skeleton;
		if (m_skeletonRef != nullptr)
		{
			modelInitData.m_skeleton = m_skeletonRef;
		}

		// シーンライトが未設定ならデフォルトを補完
		if (modelInitData.m_expandConstantBuffer == nullptr)
		{
			modelInitData.m_expandConstantBuffer = g_sceneLight->GetLight();
			modelInitData.m_expandConstantBufferSize = sizeof(Light);
		}

		// 影モデル・GetModel()用の k2EngineLow::Model を初期化する
		m_model.Init(modelInitData);

		if (m_isForwardRender)
		{
			m_forwardRenderModel->Init(modelInitData);
		}
		else
		{
			InitRenderToGBufferModel(modelInitData);
		}

		// トゥーン有効なら追加モデルを初期化する
		if (IsToonEnabled())
		{
			InitToonModels(modelInitData);
		}

		// シャドウマップ描画用モデルを初期化する
		// トゥーン・フォワードのモデルも影は落とすため、分岐せず常に作る
		InitShadowModel(modelInitData);

		// アニメーション初期化
		if (m_animationClips != nullptr && numAnimationClips > 0 && m_skeletonRef != nullptr)
		{
			m_animation.Init(*m_skeletonRef, m_animationClips, numAnimationClips);
		}

		// スケルトンを持つか記録する
		m_hasSkeleton = (m_skeletonRef != nullptr) && m_skeletonRef->IsInited();

		// ローカルAABBをtkmから計算する（スケルトンなしの場合に使用）
		CalcLocalAABBFromTkm(initData.m_tkmFilePath);
	}


	void ModelRender::InitRenderToGBufferModel(const ModelInitData& baseInitData)
	{
		ModelInitData gBufferInitData = baseInitData;

		// GBuffer書き込み用シェーダーに差し替える（カスタムパスが指定されていればそちらを使用）
		gBufferInitData.m_fxFilePath = m_customGBufferFxPath.empty()
			? "Assets/shader/RenderToGBuffer.fx"
			: m_customGBufferFxPath.c_str();

		// エントリーポイントを明示的に設定する
		gBufferInitData.m_vsEntryPointFunc = "VSMain";
		gBufferInitData.m_vsSkinEntryPointFunc = "VSMainSkin";

		// シャドウは現在未実装のためPSMainを使用
		gBufferInitData.m_psEntryPointFunc = "PSMain";

		// GBufferのカラーバッファフォーマットを設定する
		gBufferInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// アルベド
		gBufferInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_UNORM;		// 法線
		// dirLightScale・ambientScaleが1.0を超える値を扱うためR32G32B32A32_FLOATを使用する
		gBufferInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// PBRパラメータ

		// PBR補正パラメータをb2に設定する
		gBufferInitData.m_expandConstantBuffer2 = &m_pbrParam;
		gBufferInitData.m_expandConstantBufferSize2 = sizeof(PBRParam);

		// ディザリングCB（b3）のプレースホルダーを設定する
		// OcclusionDitherManager::Register後にSetExpandConstantBuffer3()で実際のCBに差し替えられる
		gBufferInitData.m_expandConstantBuffer3 = &m_ditherCbPlaceholder;
		gBufferInitData.m_expandConstantBufferSize3 = sizeof(SDitherCbPlaceholder);

		// モデル単位ディザリングCB（b4）を設定する
		// SetDitherAlpha()でmodelDitherAlphaを更新すると次のDraw()でGPUに自動転送される
		gBufferInitData.m_expandConstantBuffer4 = &m_modelDitherCb;
		gBufferInitData.m_expandConstantBufferSize4 = sizeof(SModelDitherCb);

		m_renderToGBufferModel->Init(gBufferInitData);
	}


	void ModelRender::InitToonModels(const ModelInitData& baseInitData)
	{
		m_toonModel = std::make_unique<BeastModel>();
		m_outlineModel = std::make_unique<BeastModel>();

		// ----- トゥーンモデル（toon.fx、フォワードパスで描画） -----
		ModelInitData toonInitData = baseInitData;

		toonInitData.m_fxFilePath = "Assets/shader/toon.fx";
		toonInitData.m_vsEntryPointFunc = "VSMain";
		toonInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		toonInitData.m_psEntryPointFunc = "PSMain";

		// フォワードレンダリング用カラーバッファフォーマット
		toonInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		// b2にトゥーンパラメータを設定する
		toonInitData.m_expandConstantBuffer2 = &m_toonCb;
		toonInitData.m_expandConstantBufferSize2 = sizeof(SToonCb);

		m_toonModel->Init(toonInitData);

		// ----- アウトラインモデル（outline.fx、前面カリングで背面だけ描画） -----
		ModelInitData outlineInitData = baseInitData;

		outlineInitData.m_fxFilePath = "Assets/shader/outline.fx";
		outlineInitData.m_vsEntryPointFunc = "VSMain";
		outlineInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		outlineInitData.m_psEntryPointFunc = "PSMain";

		// フォワードレンダリング用カラーバッファフォーマット
		outlineInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

		// 背面法線押し出しのため前面をカリングする
		outlineInitData.m_cullMode = D3D12_CULL_MODE_FRONT;

		// b2にアウトラインパラメータを設定する
		outlineInitData.m_expandConstantBuffer2 = &m_outlineCb;
		outlineInitData.m_expandConstantBufferSize2 = sizeof(SOutlineCb);

		m_outlineModel->Init(outlineInitData);
	}


	void ModelRender::InitShadowModel(const ModelInitData& baseInitData)
	{
		ModelInitData shadowInitData = baseInitData;

		// 深度だけを書き込むシェーダーに差し替える
		shadowInitData.m_fxFilePath = SHADOW_MAP_FX_PATH;
		shadowInitData.m_vsEntryPointFunc = "VSMain";
		shadowInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		shadowInitData.m_psEntryPointFunc = "PSMain";

		// シャドウマップは深度のみのR32_FLOAT。半透明合成はしない
		shadowInitData.m_alphaBlendMode = AlphaBlendMode_None;
		shadowInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
		for (int i = 1; i < MAX_RENDERING_TARGET; i++)
		{
			shadowInitData.m_colorBufferFormat[i] = DXGI_FORMAT_UNKNOWN;
		}

		// GBuffer用に差し込んでいた拡張定数バッファは不要なので外す
		shadowInitData.m_expandConstantBuffer2 = nullptr;
		shadowInitData.m_expandConstantBufferSize2 = 0;

		m_shadowModels.Init(shadowInitData);
		m_isShadowModelInited = true;
	}


	void ModelRender::OnRenderShadowMap(
		RenderContext& rc,
		const Matrix& lightViewMatrix,
		const Matrix& lightProjMatrix)
	{
		// 非表示・影を落とさない設定・未初期化のいずれかなら描かない
		if (!m_visible || !m_isCastShadow || !m_isShadowModelInited) return;

		// カメラではなくライトの行列で描画する
		m_shadowModels.Draw(rc, lightViewMatrix, lightProjMatrix, m_maxInstance);
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


	void ModelRender::InitAnimation(
		AnimationClip* animtionClips,
		int numAnimationClips,
		EnModelUpAxis enModelUpAxis)
	{
		m_animationClips = animtionClips;
		m_numAnimationClips = numAnimationClips;
		if (m_animationClips != nullptr && m_skeletonRef != nullptr)
		{
			m_animation.Init(*m_skeletonRef, m_animationClips, m_numAnimationClips);
		}
	}


	void ModelRender::SetupShaderEntryPointFunc(ModelInitData& modelInitData)
	{
		modelInitData.m_vsEntryPointFunc = "VSMain";
		modelInitData.m_vsSkinEntryPointFunc = "VSMain";
		/** アニメーションがある場合 */
		if (m_animationClips != nullptr)
		{
			modelInitData.m_vsSkinEntryPointFunc = "VSMainSkin";
		}
	}


	void ModelRender::UpdateWorldMatrixInModes()
	{
		m_model.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_renderToGBufferModel->UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_forwardRenderModel->UpdateWorldMatrix(m_position, m_rotation, m_scale);
		m_shadowModels.UpdateWorldMatrix(m_position, m_rotation, m_scale);

		if (m_toonModel != nullptr)
		{
			m_toonModel->UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}
		if (m_outlineModel != nullptr)
		{
			m_outlineModel->UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}
	}


	void ModelRender::CalcLocalAABBFromTkm(const char* filePath)
	{
		if (filePath == nullptr)
		{
			// ファイルパスがない場合はカリングを無効化する
			m_isCullingEnabled = false;
			return;
		}

		// バンクからtkmファイルを取得する
		const TkmFile* tkmFile = g_engine->GetTkmFileFromBank(filePath);
		if (tkmFile == nullptr)
		{
			// tkmファイルが取得できない場合はカリングを無効化する
			m_isCullingEnabled = false;
			return;
		}

		Vector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
		Vector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		bool hasVertex = false;

		// 全メッシュの全頂点を走査してAABBを計算する
		tkmFile->QueryMeshParts([&](const TkmFile::SMesh& mesh)
			{
				for (const auto& vertex : mesh.vertexBuffer)
				{
					vMin.x = min(vMin.x, vertex.pos.x);
					vMin.y = min(vMin.y, vertex.pos.y);
					vMin.z = min(vMin.z, vertex.pos.z);

					vMax.x = max(vMax.x, vertex.pos.x);
					vMax.y = max(vMax.y, vertex.pos.y);
					vMax.z = max(vMax.z, vertex.pos.z);

					hasVertex = true;
				}
			});

		if (!hasVertex)
		{
			// 頂点が1つも存在しない場合はカリングを無効化する
			m_isCullingEnabled = false;
			return;
		}

		m_localAABB.Init(vMax, vMin);
	}


	Vector3 ModelRender::GetBoneWorldPosition(const wchar_t* boneName) const
	{
		if (!m_hasSkeleton || m_skeletonRef == nullptr || !m_skeletonRef->IsInited())
		{
			K2_ASSERT(false, "スケルトンを持たないモデルに対してボーン座標を要求しました。");
			return m_position;
		}

		const int boneNo = m_skeletonRef->FindBoneID(boneName);
		K2_ASSERT(boneNo != -1, "指定した名前のボーンが見つかりませんでした。");
		if (boneNo == -1)
		{
			return m_position;
		}

		// v[3] がワールド行列の平行移動成分（ボーンのワールド座標）
		const Matrix& boneWorldMatrix = m_skeletonRef->GetBone(boneNo)->GetWorldMatrix();
		return Vector3(boneWorldMatrix.v[3].x, boneWorldMatrix.v[3].y, boneWorldMatrix.v[3].z);
	}


	void ModelRender::UpdateWorldAABB()
	{
		if (m_hasSkeleton && m_skeletonRef != nullptr && m_skeletonRef->IsInited())
		{
			// スケルトンあり: 各ボーンのワールド行列の平行移動成分からAABBを構築する
			// GetBone(i)->GetWorldMatrix() はボーン自身のワールド座標を持つ行列であり、
			// GetBoneMatricesTopAddress() のスキニング行列（invBindPose * worldMatrix）とは異なる
			const int numBones = m_skeletonRef->GetNumBones();

			Vector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (int i = 0; i < numBones; i++)
			{
				// v[3] がワールド行列の平行移動成分（ボーンのワールド座標）
				const Vector3 bonePos(
					m_skeletonRef->GetBone(i)->GetWorldMatrix().v[3].x,
					m_skeletonRef->GetBone(i)->GetWorldMatrix().v[3].y,
					m_skeletonRef->GetBone(i)->GetWorldMatrix().v[3].z
				);

				vMin.x = min(vMin.x, bonePos.x);
				vMin.y = min(vMin.y, bonePos.y);
				vMin.z = min(vMin.z, bonePos.z);

				vMax.x = max(vMax.x, bonePos.x);
				vMax.y = max(vMax.y, bonePos.y);
				vMax.z = max(vMax.z, bonePos.z);
			}

			// アニメーションで頂点がボーン位置より外に出る可能性があるため安全マージンを加算する
			vMin.x -= BONE_AABB_MARGIN;
			vMin.y -= BONE_AABB_MARGIN;
			vMin.z -= BONE_AABB_MARGIN;

			vMax.x += BONE_AABB_MARGIN;
			vMax.y += BONE_AABB_MARGIN;
			vMax.z += BONE_AABB_MARGIN;

			m_worldAABBMin = vMin;
			m_worldAABBMax = vMax;
		}
		else
		{
			// スケルトンなし: ローカルAABBの8頂点をワールド行列で変換して包含AABBを求める
			Vector3 worldVertices[8];
			m_localAABB.CalcVertexPositions(worldVertices, m_model.GetWorldMatrix());

			Vector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
			Vector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

			for (int i = 0; i < 8; i++)
			{
				vMin.x = min(vMin.x, worldVertices[i].x);
				vMin.y = min(vMin.y, worldVertices[i].y);
				vMin.z = min(vMin.z, worldVertices[i].z);

				vMax.x = max(vMax.x, worldVertices[i].x);
				vMax.y = max(vMax.y, worldVertices[i].y);
				vMax.z = max(vMax.z, worldVertices[i].z);
			}

			m_worldAABBMin = vMin;
			m_worldAABBMax = vMax;
		}
	}


	void ModelRender::Update()
	{
		/** モデルのワールド行列を更新する */
		UpdateWorldMatrixInModes();

		/** スケルトンのボーン行列を更新する */
		if (m_skeletonRef != nullptr && m_skeletonRef->IsInited())
		{
			m_skeletonRef->Update(m_model.GetWorldMatrix());
		}

		/** アニメーションを進める */
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);

		/** ワールドAABBを更新する */
		UpdateWorldAABB();
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		if (!m_visible) return;

		if (IsToonEnabled() || m_isForwardRender)
		{
			// トゥーン有効またはフォワード指定: フォワードリストに登録する
			// トゥーン有効時はGBufferへの書き込みをスキップし、
			// フォワードパスの中でtoon.fx・outline.fxで描画する
			g_renderingEngine->AddForwardModelList(this);
		}
		else
		{
			// ディファードレンダリングで描画するなら
			g_renderingEngine->AddDeferredModelList(this);
		}
	}


	void ModelRender::OnDraw(RenderContext& rc)
	{
		/** 描画が有効でない場合は処理しない */
		if (!m_visible) return;

		const Frustum& frustum = g_renderingEngine->GetActiveFrustum();

		if (IsToonEnabled())
		{
			// トゥーン有効: GBufferへの書き込みをスキップし、フォワードパスで描画する
			// アウトラインを先に描画して、トゥーンモデルで上書きする
			if (m_outlineModel != nullptr)
			{
				m_outlineModel->Draw(rc, frustum, m_maxInstance);
			}
			if (m_toonModel != nullptr)
			{
				m_toonModel->Draw(rc, frustum, m_maxInstance);
			}
		}
		else if (m_isForwardRender)
		{
			// トゥーン無効・フォワード: 通常のフォワード描画
			m_forwardRenderModel->Draw(rc, frustum, m_maxInstance);
		}
		else
		{
			// トゥーン無効・ディファード: 通常のGBuffer描画
			m_renderToGBufferModel->Draw(rc, frustum, m_maxInstance);
		}
	}
}