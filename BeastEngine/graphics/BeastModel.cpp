/**
 * @file BeastModel.cpp
 * @brief トライアングルカリング対応モデルクラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/BeastModel.h"
#include "Geometry/Frustum.h"
 // Material.h は BeastEnginePreCompile.h 経由でインクルード済み


namespace nsBeastEngine
{
	void BeastModel::Init(const ModelInitData& initData)
	{
		K2_ASSERT(
			initData.m_tkmFilePath,
			"error : initData.m_tkmFilePathが指定されていません。"
		);

		if (initData.m_skeleton != nullptr)
		{
			m_meshParts.BindSkeleton(*initData.m_skeleton);
		}

		m_modelUpAxis = initData.m_modelUpAxis;

		// tkmファイルをバンクから取得する（未登録なら新規ロードして登録する）
		auto tkmFile = g_engine->GetTkmFileFromBank(initData.m_tkmFilePath);
		if (tkmFile == nullptr)
		{
			tkmFile = new TkmFile;
			tkmFile->Load(initData.m_tkmFilePath, false);
			g_engine->RegistTkmFileToBank(initData.m_tkmFilePath, tkmFile);
		}
		m_tkmFile = tkmFile;

		m_meshParts.InitFromTkmFile(
			*m_tkmFile,
			initData.m_fxFilePath,
			initData.m_vsEntryPointFunc,
			initData.m_vsSkinEntryPointFunc,
			initData.m_psEntryPointFunc,
			initData.m_expandConstantBuffer,
			initData.m_expandConstantBufferSize,
			initData.m_expandConstantBuffer2,
			initData.m_expandConstantBufferSize2,
			initData.m_expandConstantBuffer3,
			initData.m_expandConstantBufferSize3,
			initData.m_expandConstantBuffer4,
			initData.m_expandConstantBufferSize4,
			initData.m_expandShaderResoruceView,
			initData.m_colorBufferFormat,
			initData.m_alphaBlendMode,
			initData.m_isDepthWrite,
			initData.m_isDepthTest,
			initData.m_cullMode
		);

		UpdateWorldMatrix(g_vec3Zero, g_quatIdentity, g_vec3One);
		m_isInited = true;
	}


	void BeastModel::ReInitMaterials(MaterialReInitData& reInitData)
	{
		m_meshParts.ReInitMaterials(reInitData);
	}


	Matrix BeastModel::CalcWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale)
	{
		Matrix mBias, mWorld;
		if (m_modelUpAxis == enModelUpAxisZ)
		{
			// Z-up モデル用バイアス回転
			mBias.MakeRotationX(Math::PI * -0.5f);
		}
		Matrix mTrans, mRot, mScale;
		mTrans.MakeTranslation(pos);
		mRot.MakeRotationFromQuaternion(rot);
		mScale.MakeScaling(scale);
		mWorld = mBias * mScale * mRot * mTrans;
		return mWorld;
	}


	void BeastModel::ChangeAlbedoMap(const char* materialName, Texture& albedoMap)
	{
		m_meshParts.QueryMeshs([&](const SBeastMesh& mesh)
			{
				// todo: マテリアル名をtkmファイルに出力してないため、今は全マテリアル差し替え
				for (Material* material : mesh.m_materials)
				{
					material->GetAlbedoMap().InitFromD3DResource(albedoMap.Get());
				}
			});
		m_meshParts.CreateDescriptorHeaps();
	}


	void BeastModel::Draw(nsK2EngineLow::RenderContext& rc, int numInstance)
	{
		if (!m_isInited)
		{
			return;
		}
		m_meshParts.Draw(
			rc,
			m_worldMatrix,
			CameraSystem::Get().GetActiveCamera()->GetViewMatrix(),
			CameraSystem::Get().GetActiveCamera()->GetProjectionMatrix(),
			numInstance,
			nullptr
		);
	}


	void BeastModel::Draw(
		nsK2EngineLow::RenderContext& rc,
		Camera& camera,
		int numInstance
	)
	{
		if (!m_isInited)
		{
			return;
		}
		m_meshParts.Draw(
			rc,
			m_worldMatrix,
			camera.GetViewMatrix(),
			camera.GetProjectionMatrix(),
			numInstance,
			nullptr
		);
	}


	void BeastModel::Draw(
		nsK2EngineLow::RenderContext& rc,
		const Matrix& viewMatrix,
		const Matrix& projMatrix,
		int numInstance
	)
	{
		if (!m_isInited)
		{
			return;
		}
		m_meshParts.Draw(
			rc,
			m_worldMatrix,
			viewMatrix,
			projMatrix,
			numInstance,
			nullptr
		);
	}


	void BeastModel::Draw(
		nsK2EngineLow::RenderContext& rc,
		const Frustum& frustum,
		int numInstance
	)
	{
		if (!m_isInited)
		{
			return;
		}
		m_meshParts.Draw(
			rc,
			m_worldMatrix,
			CameraSystem::Get().GetActiveCamera()->GetViewMatrix(),
			CameraSystem::Get().GetActiveCamera()->GetProjectionMatrix(),
			numInstance,
			&frustum
		);
	}


	void BeastModel::Draw(
		nsK2EngineLow::RenderContext& rc,
		Camera& camera,
		const Frustum& frustum,
		int numInstance
	)
	{
		if (!m_isInited)
		{
			return;
		}
		m_meshParts.Draw(
			rc,
			m_worldMatrix,
			camera.GetViewMatrix(),
			camera.GetProjectionMatrix(),
			numInstance,
			&frustum
		);
	}
}