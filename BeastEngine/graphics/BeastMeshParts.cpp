/**
 * @file BeastMeshParts.cpp
 * @brief トライアングルカリング対応メッシュパーツクラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/BeastMeshParts.h"
#include "Geometry/TriangleCuller.h"
#include "Geometry/Frustum.h"
 // Material.h は BeastEnginePreCompile.h 経由でインクルード済み


namespace nsBeastEngine
{
	BeastMeshParts::~BeastMeshParts()
	{
		for (auto& mesh : m_meshs)
		{
			// 元インデックスバッファを削除する
			for (auto& ib : mesh->m_indexBufferArray)
			{
				delete ib;
			}
			// 可視インデックスバッファを削除する（ダブルバッファ）
			for (auto& ibArr : mesh->m_visibleIndexBuffers)
			{
				for (auto* ib : ibArr)
				{
					delete ib;
				}
			}
			// マテリアルを削除する
			for (auto& mat : mesh->m_materials)
			{
				delete mat;
			}
			// メッシュを削除する
			delete mesh;
		}
	}


	void BeastMeshParts::InitFromTkmFile(
		const TkmFile& tkmFile,
		const char* fxFilePath,
		const char* vsEntryPointFunc,
		const char* vsSkinEntryPointFunc,
		const char* psEntryPointFunc,
		void* expandData,
		int expandDataSize,
		void* expandData2,
		int expandDataSize2,
		void* expandData3,
		int expandDataSize3,
		void* expandData4,
		int expandDataSize4,
		const std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV>& expandShaderResourceView,
		const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
		AlphaBlendMode alphaBlendMode,
		bool isDepthWrite,
		bool isDepthTest,
		D3D12_CULL_MODE cullMode
	)
	{
		m_meshs.resize(tkmFile.GetNumMesh());
		int meshNo = 0;
		int materialNo = 0;
		tkmFile.QueryMeshParts([&](const TkmFile::SMesh& mesh)
			{
				CreateMeshFromTkmMesh(
					mesh,
					meshNo,
					materialNo,
					fxFilePath,
					vsEntryPointFunc,
					vsSkinEntryPointFunc,
					psEntryPointFunc,
					colorBufferFormat,
					alphaBlendMode,
					isDepthWrite,
					isDepthTest,
					cullMode
				);
				meshNo++;
			});

		// 共通定数バッファを作成する（b0）
		m_commonConstantBuffer.Init(sizeof(SConstantBuffer), nullptr);

		// 拡張定数バッファを作成する（b1）
		if (expandData)
		{
			m_expandConstantBuffer.Init(expandDataSize, nullptr);
			m_expandData = expandData;
		}
		// 拡張定数バッファを作成する（b2）
		if (expandData2)
		{
			m_expandConstantBuffer2.Init(expandDataSize2, nullptr);
			m_expandData2 = expandData2;
		}
		// 拡張定数バッファを作成する（b3）
		if (expandData3)
		{
			m_expandConstantBuffer3.Init(expandDataSize3, nullptr);
			m_expandData3 = expandData3;
		}
		// 拡張定数バッファを作成する（b4）
		if (expandData4)
		{
			m_expandConstantBuffer4.Init(expandDataSize4, nullptr);
			m_expandData4 = expandData4;
		}

		for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++)
		{
			m_expandShaderResourceView[i] = expandShaderResourceView[i];
		}

		CreateDescriptorHeaps();
	}


	void BeastMeshParts::CreateDescriptorHeaps()
	{
		// 必要なディスクリプタ数を計算する
		int srvNo = 0;
		int cbNo = 0;
		for (auto& mesh : m_meshs)
		{
			for (int matNo = 0; matNo < static_cast<int>(mesh->m_materials.size()); matNo++)
			{
				srvNo += NUM_SRV_ONE_MATERIAL;
				cbNo += NUM_CBV_ONE_MATERIAL;
			}
		}

		m_descriptorHeap.ResizeShaderResource(srvNo);
		m_descriptorHeap.ResizeConstantBuffer(cbNo);
		m_descriptorHeap.ResizeUnorderAccessResource(0);

		srvNo = 0;
		cbNo = 0;
		for (auto& mesh : m_meshs)
		{
			for (int matNo = 0; matNo < static_cast<int>(mesh->m_materials.size()); matNo++)
			{
				// SRVを登録する
				m_descriptorHeap.RegistShaderResource(srvNo, mesh->m_materials[matNo]->GetAlbedoMap());
				m_descriptorHeap.RegistShaderResource(srvNo + 1, mesh->m_materials[matNo]->GetNormalMap());
				m_descriptorHeap.RegistShaderResource(srvNo + 2, mesh->m_materials[matNo]->GetSpecularMap());
				m_descriptorHeap.RegistShaderResource(srvNo + 3, m_boneMatricesStructureBuffer);
				for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++)
				{
					if (m_expandShaderResourceView[i])
					{
						m_descriptorHeap.RegistShaderResource(
							srvNo + EXPAND_SRV_REG__START_NO + i,
							*m_expandShaderResourceView[i]
						);
					}
				}
				srvNo += NUM_SRV_ONE_MATERIAL;

				// CBVを登録する
				m_descriptorHeap.RegistConstantBuffer(cbNo, m_commonConstantBuffer);
				if (m_expandConstantBuffer.IsValid())
				{
					m_descriptorHeap.RegistConstantBuffer(cbNo + 1, m_expandConstantBuffer);
				}
				if (m_expandConstantBuffer2.IsValid())
				{
					m_descriptorHeap.RegistConstantBuffer(cbNo + 2, m_expandConstantBuffer2);
				}
				if (m_expandConstantBuffer3.IsValid())
				{
					m_descriptorHeap.RegistConstantBuffer(cbNo + 3, m_expandConstantBuffer3);
				}
				if (m_expandConstantBuffer4.IsValid())
				{
					m_descriptorHeap.RegistConstantBuffer(cbNo + 4, m_expandConstantBuffer4);
				}
				cbNo += NUM_CBV_ONE_MATERIAL;
			}
		}

		m_descriptorHeap.Commit();
	}


	void BeastMeshParts::CreateMeshFromTkmMesh(
		const TkmFile::SMesh& tkmMesh,
		int meshNo,
		int& materialNum,
		const char* fxFilePath,
		const char* vsEntryPointFunc,
		const char* vsSkinEntryPointFunc,
		const char* psEntryPointFunc,
		const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
		AlphaBlendMode alphaBlendMode,
		bool isDepthWrite,
		bool isDepthTest,
		D3D12_CULL_MODE cullMode
	)
	{
		// 1. 頂点バッファを作成する
		const int numVertex = static_cast<int>(tkmMesh.vertexBuffer.size());
		const int vertexStride = sizeof(TkmFile::SVertex);
		auto mesh = new SBeastMesh;
		mesh->skinFlags.reserve(tkmMesh.materials.size());
		mesh->m_vertexBuffer.Init(vertexStride * numVertex, vertexStride);
		mesh->m_vertexBuffer.Copy(static_cast<void*>(const_cast<TkmFile::SVertex*>(&tkmMesh.vertexBuffer[0])));

		// トライアングルカリング用にローカル頂点位置をCPUキャッシュにコピーする
		mesh->m_localVertexPositions.reserve(numVertex);
		for (const auto& vertex : tkmMesh.vertexBuffer)
		{
			mesh->m_localVertexPositions.push_back(vertex.pos);
		}

		auto SetSkinFlag = [&](int index)
			{
				if (tkmMesh.vertexBuffer[index].skinWeights.x > 0.0f)
				{
					mesh->skinFlags.push_back(1);
				}
				else
				{
					mesh->skinFlags.push_back(0);
				}
			};

		// 2. インデックスバッファを作成する
		if (!tkmMesh.indexBuffer16Array.empty())
		{
			// 16ビットインデックス
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer16Array.size());
			mesh->m_srcIndexArrays.resize(tkmMesh.indexBuffer16Array.size());
			mesh->m_visibleIndices.resize(tkmMesh.indexBuffer16Array.size());
			mesh->m_visibleIndexBuffers.resize(tkmMesh.indexBuffer16Array.size());

			int ibNo = 0;
			for (auto& tkIb : tkmMesh.indexBuffer16Array)
			{
				auto ib = new IndexBuffer;
				ib->Init(static_cast<int>(tkIb.indices.size()) * 2, 2);
				ib->Copy(const_cast<uint16_t*>(&tkIb.indices[0]));

				SetSkinFlag(tkIb.indices[0]);

				// 元インデックスをCPUキャッシュにコピーする
				auto& srcArray = mesh->m_srcIndexArrays[ibNo];
				srcArray.reserve(tkIb.indices.size());
				for (auto idx : tkIb.indices)
				{
					srcArray.push_back(static_cast<uint32_t>(idx));
				}
				mesh->m_visibleIndices[ibNo].reserve(tkIb.indices.size());

				// トライアングルカリング用の可視インデックスバッファをダブルバッファで作成する
				// buf=0 がメインビュー用、buf=1 がサブビュー用（SetFrameIndex による切り替えと対応）
				for (int buf = 0; buf < 2; buf++)
				{
					auto visIb = new IndexBuffer;
					visIb->Init(static_cast<int>(tkIb.indices.size()) * 2, 2);
					visIb->Copy(const_cast<uint16_t*>(&tkIb.indices[0]));
					mesh->m_visibleIndexBuffers[ibNo][buf] = visIb;
				}

				mesh->m_indexBufferArray.push_back(ib);
				ibNo++;
			}
		}
		else
		{
			// 32ビットインデックス
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer32Array.size());
			mesh->m_srcIndexArrays.resize(tkmMesh.indexBuffer32Array.size());
			mesh->m_visibleIndices.resize(tkmMesh.indexBuffer32Array.size());
			mesh->m_visibleIndexBuffers.resize(tkmMesh.indexBuffer32Array.size());

			int ibNo = 0;
			for (auto& tkIb : tkmMesh.indexBuffer32Array)
			{
				auto ib = new IndexBuffer;
				ib->Init(static_cast<int>(tkIb.indices.size()) * 4, 4);
				ib->Copy(const_cast<uint32_t*>(&tkIb.indices[0]));

				SetSkinFlag(tkIb.indices[0]);

				// 元インデックスをCPUキャッシュにコピーする
				auto& srcArray = mesh->m_srcIndexArrays[ibNo];
				srcArray.reserve(tkIb.indices.size());
				for (auto idx : tkIb.indices)
				{
					srcArray.push_back(idx);
				}
				mesh->m_visibleIndices[ibNo].reserve(tkIb.indices.size());

				// トライアングルカリング用の可視インデックスバッファをダブルバッファで作成する
				for (int buf = 0; buf < 2; buf++)
				{
					auto visIb = new IndexBuffer;
					visIb->Init(static_cast<int>(tkIb.indices.size()) * 4, 4);
					visIb->Copy(const_cast<uint32_t*>(&tkIb.indices[0]));
					mesh->m_visibleIndexBuffers[ibNo][buf] = visIb;
				}

				mesh->m_indexBufferArray.push_back(ib);
				ibNo++;
			}
		}

		// 3. マテリアルを作成する
		mesh->m_materials.reserve(tkmMesh.materials.size());
		for (auto& tkmMat : tkmMesh.materials)
		{
			auto mat = new Material;
			mat->InitFromTkmMaterila(
				tkmMat,
				fxFilePath,
				vsEntryPointFunc,
				vsSkinEntryPointFunc,
				psEntryPointFunc,
				colorBufferFormat,
				NUM_SRV_ONE_MATERIAL,
				NUM_CBV_ONE_MATERIAL,
				NUM_CBV_ONE_MATERIAL * materialNum,
				NUM_SRV_ONE_MATERIAL * materialNum,
				alphaBlendMode,
				isDepthWrite,
				isDepthTest,
				cullMode
			);
			materialNum++;
			mesh->m_materials.push_back(mat);
		}

		m_meshs[meshNo] = mesh;
	}


	void BeastMeshParts::BindSkeleton(Skeleton& skeleton)
	{
		m_skeleton = &skeleton;
		m_boneMatricesStructureBuffer.Init(
			sizeof(Matrix),
			m_skeleton->GetNumBones(),
			m_skeleton->GetBoneMatricesTopAddress()
		);
	}


	void BeastMeshParts::Draw(
		nsK2EngineLow::RenderContext& rc,
		const Matrix& mWorld,
		const Matrix& mView,
		const Matrix& mProj,
		int numInstance,
		const Frustum* frustum
	)
	{
		rc.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 共通定数バッファを更新する（b0）
		SConstantBuffer cb;
		cb.mWorld = mWorld;
		cb.mView = mView;
		cb.mProj = mProj;
		cb.mulColor = m_mulColor;
		m_commonConstantBuffer.CopyToVRAM(cb);

		if (m_expandData) { m_expandConstantBuffer.CopyToVRAM(m_expandData); }
		if (m_expandData2) { m_expandConstantBuffer2.CopyToVRAM(m_expandData2); }
		if (m_expandData3) { m_expandConstantBuffer3.CopyToVRAM(m_expandData3); }
		if (m_expandData4) { m_expandConstantBuffer4.CopyToVRAM(m_expandData4); }

		if (m_boneMatricesStructureBuffer.IsInited())
		{
			m_boneMatricesStructureBuffer.Update(m_skeleton->GetBoneMatricesTopAddress());
		}

		for (auto& mesh : m_meshs)
		{
			rc.SetVertexBuffer(mesh->m_vertexBuffer);

			for (int matNo = 0; matNo < static_cast<int>(mesh->m_materials.size()); matNo++)
			{
				mesh->m_materials[matNo]->BeginRender(rc, mesh->skinFlags[matNo]);
				rc.SetDescriptorHeap(m_descriptorHeap);

				// トライアングルカリングを適用する
				// スキンありメッシュ（skinFlags == 1）はCPUに変形後頂点座標がないためスキップする
				if (frustum != nullptr && mesh->skinFlags[matNo] == 0)
				{
					const int numLocalVerts = static_cast<int>(mesh->m_localVertexPositions.size());

					// ローカル頂点をワールド空間に変換する
					m_worldVertexCache.resize(numLocalVerts);
					for (int vertNo = 0; vertNo < numLocalVerts; vertNo++)
					{
						m_worldVertexCache[vertNo] = mesh->m_localVertexPositions[vertNo];
						mWorld.Apply(m_worldVertexCache[vertNo]);
					}

					// 可視三角形のインデックスを収集する
					auto& visibleIndices = mesh->m_visibleIndices[matNo];
					visibleIndices.clear();

					const TriangleCuller culler;
					const auto& srcIndices = mesh->m_srcIndexArrays[matNo];
					culler.Cull(
						m_worldVertexCache.data(),
						numLocalVerts,
						srcIndices.data(),
						static_cast<int>(srcIndices.size()),
						*frustum,
						visibleIndices
					);

					// 可視インデックスが1つもなければドローコールをスキップする
					if (visibleIndices.empty())
					{
						continue;
					}

					// メインビュー（frameIdx=0）とサブビュー（frameIdx=1）で別スロットに書き込む。
					// 定数バッファと同様に SetFrameIndex による切り替えと連動している。
					const int frameIdx = g_graphicsEngine->GetBackBufferIndex();
					auto* visIb = mesh->m_visibleIndexBuffers[matNo][frameIdx];
					visIb->Copy(visibleIndices.data());
					rc.SetIndexBuffer(*visIb);
					rc.DrawIndexedInstance(static_cast<int>(visibleIndices.size()), numInstance);
				}
				else
				{
					// カリングなし：元のインデックスバッファをそのまま使用する
					auto* ib = mesh->m_indexBufferArray[matNo];
					rc.SetIndexBuffer(*ib);
					rc.DrawIndexedInstance(ib->GetCount(), numInstance);
				}
			}
		}
	}


	void BeastMeshParts::ReInitMaterials(const MaterialReInitData& reInitData)
	{
		for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++)
		{
			m_expandShaderResourceView[i] = reInitData.m_expandShaderResoruceView[i];
		}
		CreateDescriptorHeaps();
	}
}