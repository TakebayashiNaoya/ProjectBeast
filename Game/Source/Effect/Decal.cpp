/**
 * @file Decal.cpp
 * @brief 投影デカール（地形の凹凸に沿った足跡）1枚分の本体クラス
 */
#include "stdafx.h"
#include "Decal.h"

namespace
{
	/** @brief このスロット専用バンクキーのプレフィックス（"decal_patch_0" など） */
	const char* DECAL_TKM_KEY_PREFIX = "decal_patch_";
	/** @brief デカール用シェーダーパス */
	const char* DECAL_FX_PATH = "Assets/shader/Decal.fx";
}

namespace app
{
	namespace effect
	{
		void Decal::Prepare(int slotIndex)
		{
			if (m_tkmKey.empty())
			{
				m_tkmKey = DECAL_TKM_KEY_PREFIX + std::to_string(slotIndex);
			}

			if (!m_modelRender)
			{
				m_modelRender = std::make_unique<nsBeastEngine::ModelRender>();
			}
		}


		void Decal::BuildGridMesh(const std::vector<Vector3>& gridPositions, int gridResolution)
		{
			std::vector<TkmFile::SVertex> vertices;
			vertices.reserve(gridPositions.size());

			for (int j = 0; j < gridResolution; ++j)
			{
				for (int i = 0; i < gridResolution; ++i)
				{
					const int idx = j * gridResolution + i;

					TkmFile::SVertex v;
					// ★頂点にワールド座標をそのまま埋め込む（TerrainObjectのチャンクと同じ考え方）
					v.pos = gridPositions[idx];
					v.normal = Vector3(0.0f, 1.0f, 0.0f);
					v.tangent = Vector3(1.0f, 0.0f, 0.0f);
					v.binormal = Vector3(0.0f, 0.0f, 1.0f);

					const float u = (gridResolution > 1) ? float(i) / float(gridResolution - 1) : 0.5f;
					const float vCoord = (gridResolution > 1) ? float(j) / float(gridResolution - 1) : 0.5f;
					v.uv = Vector2(u, vCoord);

					v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0;
					v.skinWeights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);

					vertices.push_back(v);
				}
			}

			std::vector<uint32_t> indices;
			indices.reserve(static_cast<size_t>(gridResolution - 1) * (gridResolution - 1) * 6);

			for (int j = 0; j < gridResolution - 1; ++j)
			{
				for (int i = 0; i < gridResolution - 1; ++i)
				{
					const uint32_t i00 = j * gridResolution + i;
					const uint32_t i10 = j * gridResolution + (i + 1);
					const uint32_t i01 = (j + 1) * gridResolution + i;
					const uint32_t i11 = (j + 1) * gridResolution + (i + 1);

					// ★巻き順は既存のBuildSharedBoxMesh(0,2,1 / 1,2,3)に合わせている
					indices.push_back(i00);
					indices.push_back(i10);
					indices.push_back(i01);

					indices.push_back(i10);
					indices.push_back(i11);
					indices.push_back(i01);
				}
			}

			TkmFile::SMaterial mat = {};

			TkmFile::SIndexBuffer32 ib;
			ib.indices = std::move(indices);

			TkmFile::SMesh mesh;
			mesh.isFlatShading = false;
			mesh.materials.push_back(mat);
			mesh.vertexBuffer = std::move(vertices);
			mesh.indexBuffer32Array.push_back(std::move(ib));

			std::vector<TkmFile::SMesh> meshes;
			meshes.push_back(std::move(mesh));

			auto* tkm = new nsK2EngineLow::TkmFile();
			tkm->Build(std::move(meshes));
			g_engine->ReplaceTkmFileInBank(m_tkmKey.c_str(), tkm);
		}


		void Decal::SetupProjected(
			const std::vector<Vector3>& gridPositions,
			int gridResolution,
			nsK2EngineLow::Texture* texture,
			const Vector4& color)
		{
			BuildGridMesh(gridPositions, gridResolution);

			nsK2EngineLow::ModelInitData initData;
			initData.m_tkmFilePath = m_tkmKey.c_str();
			initData.m_fxFilePath = DECAL_FX_PATH;
			initData.m_expandShaderResoruceView[0] = texture;
			initData.m_modelUpAxis = EnModelUpAxis::enModelUpAxisY;

			m_modelRender->SetForwardRendering(true);

			m_cb.alpha = 1.0f;
			initData.m_expandConstantBuffer2 = &m_cb;
			initData.m_expandConstantBufferSize2 = sizeof(cbDecal);

			// ★毎回呼ぶ（地形が変わってテクスチャの種類が変わっても正しく反映されるようにするため）
			m_modelRender->InitFromLoaded(initData);
			m_modelRender->SetMulColor(color);

			// ★頂点にワールド座標を直接ベイクしているので、変換行列は単位行列にする
			m_modelRender->SetTRS(Vector3::Zero, Quaternion::Identity, Vector3::One);
			m_modelRender->Update();

			m_isActive = true;
		}


		bool Decal::Update(float deltaTime)
		{
			if (!m_isActive) return false;

			m_remainingLife -= deltaTime;
			if (m_remainingLife <= 0.0f)
			{
				m_isActive = false;
				return false;
			}

			if (m_remainingLife < m_fadeOutSeconds)
			{
				m_cb.alpha = m_remainingLife / m_fadeOutSeconds;
			}
			else
			{
				m_cb.alpha = 1.0f;
			}
			m_modelRender->SetExpandConstantBuffer2(&m_cb);

			m_modelRender->Update();

			return true;
		}


		void Decal::Render(RenderContext& rc)
		{
			if (!m_isActive) return;
			m_modelRender->Draw(rc);
		}
	}
}
