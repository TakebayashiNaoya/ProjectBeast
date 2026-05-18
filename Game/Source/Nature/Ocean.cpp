/**
 * @file Ocean.cpp
 * @brief 海のクラス（OceanMeshを内包）
 * @author 竹林
 */
#include "stdafx.h"
#include "Ocean.h"
#include "OceanParameter.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace nature
	{
		namespace
		{
			/** 海パラメーターJSONのパス */
			const char* OCEAN_PARAMETER_JSON_PATH = "Assets/parameter/nature/oceanParameter.json";
		}


		//============================================================
		// OceanMesh
		//============================================================

		OceanMesh::~OceanMesh()
		{
			// m_fenceEvent が未初期化（Init未呼び出し）の場合は何もしない
			if (m_fenceEvent == nullptr) return;

			//------------------------------------------------------------
			// GPUがコマンドキューを使い終わるまで待つ
			//------------------------------------------------------------
			m_fenceValue++;
			ID3D12CommandQueue* commandQueue = g_graphicsEngine->GetCommandQueue();
			commandQueue->Signal(m_fence.Get(), m_fenceValue);

			if (m_fence->GetCompletedValue() < m_fenceValue)
			{
				m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}

			//------------------------------------------------------------
			// Mapしたままのリソースを解放する
			//------------------------------------------------------------
			if (m_csCbMapped != nullptr)
			{
				m_csCbResource->Unmap(0, nullptr);
				m_csCbMapped = nullptr;
			}

			CloseHandle(m_fenceEvent);
			m_fenceEvent = nullptr;
		}


		void OceanMesh::Init(
			const char* fxFilePath,
			const char* vsEntryPoint,
			const char* psEntryPoint,
			void* expandConstantBuffer,
			int expandConstantBufferSize,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			const wchar_t* albedoMapFilePath,
			const wchar_t* normalMapFilePath,
			const wchar_t* specularMapFilePath
		)
		{
			CreateGridMesh();
			InitShaders(fxFilePath, vsEntryPoint, psEntryPoint);

			m_albedoMap.InitFromDDSFile(albedoMapFilePath);
			m_normalMap.InitFromDDSFile(normalMapFilePath);
			m_specularMap.InitFromDDSFile(specularMapFilePath);

			InitRootSignature();
			InitPipelineState(colorBufferFormat);

			m_commonConstantBuffer.Init(sizeof(SCommonConstantBuffer), nullptr);

			if (expandConstantBuffer != nullptr)
			{
				m_expandConstantBuffer.Init(expandConstantBufferSize, nullptr);
				m_expandData = expandConstantBuffer;
			}

			InitDescriptorHeap();
			InitComputeShader();

			// チャンクAABBの配列を事前確保する（BuildChunkAABBs()で毎フレーム上書き）
			const int numChunks = m_chunkDivision * m_chunkDivision;
			m_chunkAABBs.resize(static_cast<size_t>(numChunks));
		}


		void OceanMesh::BuildChunkAABBs(float maxWaveHeight)
		{
			// GRID_DIVISIONがchunkDivisionで均等割りできることを前提とする
			const int cellsPerChunk = GRID_DIVISION / m_chunkDivision;
			const int numVertsPerRow = GRID_DIVISION + 1;
			const float gridHalfSize = GRID_SIZE * 0.5f;
			const float cellSize = GRID_SIZE / static_cast<float>(GRID_DIVISION);
			const float chunkSize = cellSize * static_cast<float>(cellsPerChunk);

			const float* cache = m_waveHeightCache.data();

			for (int chunkZ = 0; chunkZ < m_chunkDivision; chunkZ++)
			{
				for (int chunkX = 0; chunkX < m_chunkDivision; chunkX++)
				{
					const int chunkIndex = chunkZ * m_chunkDivision + chunkX;

					// チャンクのXZ範囲をワールド空間で算出する
					const float minX = -gridHalfSize + chunkSize * static_cast<float>(chunkX);
					const float minZ = -gridHalfSize + chunkSize * static_cast<float>(chunkZ);
					const float maxX = minX + chunkSize;
					const float maxZ = minZ + chunkSize;

					// チャンク内の頂点の波高さmin/maxをキャッシュから収集する
					float minY = FLT_MAX;
					float maxY = -FLT_MAX;

					const int vertXStart = chunkX * cellsPerChunk;
					const int vertZStart = chunkZ * cellsPerChunk;
					const int vertXEnd = vertXStart + cellsPerChunk + 1;
					const int vertZEnd = vertZStart + cellsPerChunk + 1;

					for (int vz = vertZStart; vz < vertZEnd; vz++)
					{
						for (int vx = vertXStart; vx < vertXEnd; vx++)
						{
							const float h = cache[vz * numVertsPerRow + vx];
							if (h < minY) minY = h;
							if (h > maxY) maxY = h;
						}
					}

					// 波高さキャッシュは理論最大値に収まるが、念のためクランプする
					minY = max(minY, -maxWaveHeight);
					maxY = min(maxY, maxWaveHeight);

					m_chunkAABBs[static_cast<size_t>(chunkIndex)].min = Vector3(minX, minY, minZ);
					m_chunkAABBs[static_cast<size_t>(chunkIndex)].max = Vector3(maxX, maxY, maxZ);
				}
			}
		}


		void OceanMesh::SetupDrawCommands(RenderContext& rc, const Matrix& mWorld)
		{
			// 共通定数バッファを更新する（b0）
			SCommonConstantBuffer cb;
			cb.mWorld = mWorld;
			cb.mView = g_camera3D->GetViewMatrix();
			cb.mProj = g_camera3D->GetProjectionMatrix();
			cb.mulColor = Vector4::One;
			m_commonConstantBuffer.CopyToVRAM(cb);

			// 拡張定数バッファを更新する（b1）
			UpdateExpandConstantBuffer();

			// 描画コマンドを発行する
			rc.SetRootSignature(m_rootSignature);
			rc.SetPipelineState(m_pipelineState);
			rc.SetDescriptorHeap(m_descriptorHeap);
			rc.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			rc.SetVertexBuffer(m_vertexBuffer);
		}


		void OceanMesh::Draw(RenderContext& rc, const Matrix& mWorld)
		{
			SetupDrawCommands(rc, mWorld);
			rc.SetIndexBuffer(m_indexBuffer);
			rc.DrawIndexedInstance(m_indexCount, 1);
		}


		void OceanMesh::Draw(RenderContext& rc, const Matrix& mWorld, const nsBeastEngine::Frustum& frustum)
		{
			SetupDrawCommands(rc, mWorld);

			// 可視インデックスを収集する
			m_visibleIndexArray.clear();

			const int cellsPerChunk = GRID_DIVISION / m_chunkDivision;
			const float gridHalfSize = GRID_SIZE * 0.5f;
			const float cellSize = GRID_SIZE / static_cast<float>(GRID_DIVISION);
			const int numVertsPerRow = GRID_DIVISION + 1;

			for (int chunkZ = 0; chunkZ < m_chunkDivision; chunkZ++)
			{
				for (int chunkX = 0; chunkX < m_chunkDivision; chunkX++)
				{
					const int chunkIndex = chunkZ * m_chunkDivision + chunkX;
					const SChunkAABB& chunkAABB = m_chunkAABBs[static_cast<size_t>(chunkIndex)];

					// チャンクAABBが視錐台と交差しない場合はスキップする
					if (!frustum.IsIntersectAABBWorld(chunkAABB.min, chunkAABB.max))
					{
						continue;
					}

					// 交差チャンクはセル単位でAABB判定し、可視セルのインデックスのみ追加する
					// セルのY範囲は親チャンクのY範囲をそのまま流用する
					const int cellXStart = chunkX * cellsPerChunk;
					const int cellZStart = chunkZ * cellsPerChunk;

					for (int cz = cellZStart; cz < cellZStart + cellsPerChunk; cz++)
					{
						for (int cx = cellXStart; cx < cellXStart + cellsPerChunk; cx++)
						{
							// セルのXZ範囲をワールド空間で算出する
							const float cellMinX = -gridHalfSize + cellSize * static_cast<float>(cx);
							const float cellMinZ = -gridHalfSize + cellSize * static_cast<float>(cz);
							const float cellMaxX = cellMinX + cellSize;
							const float cellMaxZ = cellMinZ + cellSize;

							// セルAABBのY範囲は親チャンクのY範囲を流用する
							const Vector3 cellMin(cellMinX, chunkAABB.min.y, cellMinZ);
							const Vector3 cellMax(cellMaxX, chunkAABB.max.y, cellMaxZ);

							// セルAABBが視錐台と交差しない場合はスキップする
							if (!frustum.IsIntersectAABBWorld(cellMin, cellMax))
							{
								continue;
							}

							// 可視セルの三角形インデックスを追加する
							const int topLeft = cz * numVertsPerRow + cx;
							const int topRight = cz * numVertsPerRow + cx + 1;
							const int bottomLeft = (cz + 1) * numVertsPerRow + cx;
							const int bottomRight = (cz + 1) * numVertsPerRow + cx + 1;

							// 三角形①
							m_visibleIndexArray.push_back(static_cast<uint32_t>(topLeft));
							m_visibleIndexArray.push_back(static_cast<uint32_t>(bottomLeft));
							m_visibleIndexArray.push_back(static_cast<uint32_t>(topRight));

							// 三角形②
							m_visibleIndexArray.push_back(static_cast<uint32_t>(topRight));
							m_visibleIndexArray.push_back(static_cast<uint32_t>(bottomLeft));
							m_visibleIndexArray.push_back(static_cast<uint32_t>(bottomRight));
						}
					}
				}
			}

			// 可視インデックスが1つもなければドローコールをスキップする
			if (m_visibleIndexArray.empty())
			{
				return;
			}

			// 可視インデックスをGPUバッファに書き込んでドローコールを発行する
			m_visibleIndexBuffer.Copy(m_visibleIndexArray.data());
			rc.SetIndexBuffer(m_visibleIndexBuffer);
			rc.DrawIndexedInstance(static_cast<int>(m_visibleIndexArray.size()), 1);
		}


		void OceanMesh::CreateGridMesh()
		{
			const int   numDivision = GRID_DIVISION;
			const float gridHalfSize = GRID_SIZE * 0.5f;
			const float cellSize = GRID_SIZE / static_cast<float>(numDivision);
			const int   numVertsPerRow = numDivision + 1;
			const int   numVerts = numVertsPerRow * numVertsPerRow;

			std::vector<OceanVertex> vertices(static_cast<size_t>(numVerts));
			for (int z = 0; z <= numDivision; ++z)
			{
				for (int x = 0; x <= numDivision; ++x)
				{
					OceanVertex& v = vertices[static_cast<size_t>(z * numVertsPerRow + x)];
					v.pos = Vector3(
						-gridHalfSize + cellSize * static_cast<float>(x),
						0.0f,
						-gridHalfSize + cellSize * static_cast<float>(z)
					);
					v.normal = Vector3(0.0f, 1.0f, 0.0f);
					v.tangent = Vector3(1.0f, 0.0f, 0.0f);
					v.biNormal = Vector3(0.0f, 0.0f, 1.0f);
					v.uv = Vector2(
						static_cast<float>(x) / static_cast<float>(numDivision),
						static_cast<float>(z) / static_cast<float>(numDivision)
					);
					v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0;
					v.weights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
				}
			}

			m_vertexBuffer.Init(
				static_cast<int>(sizeof(OceanVertex) * vertices.size()),
				sizeof(OceanVertex)
			);
			m_vertexBuffer.Copy(vertices.data());

			// チャンクごとにインデックスを生成し、CPUキャッシュ（m_srcIndexArray）に保存する
			const int cellsPerChunk = numDivision / m_chunkDivision;
			const int numChunks = m_chunkDivision * m_chunkDivision;
			// チャンク1つあたりのインデックス数（セル数 × 三角形2枚 × 3頂点）
			const int indicesPerChunk = cellsPerChunk * cellsPerChunk * 6;
			const int totalIndices = numChunks * indicesPerChunk;

			m_srcIndexArray.reserve(static_cast<size_t>(totalIndices));
			m_chunkIndexOffsets.resize(static_cast<size_t>(numChunks));
			m_chunkIndexCounts.resize(static_cast<size_t>(numChunks));

			for (int chunkZ = 0; chunkZ < m_chunkDivision; chunkZ++)
			{
				for (int chunkX = 0; chunkX < m_chunkDivision; chunkX++)
				{
					const int chunkIndex = chunkZ * m_chunkDivision + chunkX;
					m_chunkIndexOffsets[static_cast<size_t>(chunkIndex)] =
						static_cast<int>(m_srcIndexArray.size());

					// チャンク内のセルをループしてインデックスを生成する
					const int cellXStart = chunkX * cellsPerChunk;
					const int cellZStart = chunkZ * cellsPerChunk;

					for (int cz = cellZStart; cz < cellZStart + cellsPerChunk; cz++)
					{
						for (int cx = cellXStart; cx < cellXStart + cellsPerChunk; cx++)
						{
							const int topLeft = cz * numVertsPerRow + cx;
							const int topRight = cz * numVertsPerRow + cx + 1;
							const int bottomLeft = (cz + 1) * numVertsPerRow + cx;
							const int bottomRight = (cz + 1) * numVertsPerRow + cx + 1;

							// 三角形①
							m_srcIndexArray.push_back(static_cast<uint32_t>(topLeft));
							m_srcIndexArray.push_back(static_cast<uint32_t>(bottomLeft));
							m_srcIndexArray.push_back(static_cast<uint32_t>(topRight));

							// 三角形②
							m_srcIndexArray.push_back(static_cast<uint32_t>(topRight));
							m_srcIndexArray.push_back(static_cast<uint32_t>(bottomLeft));
							m_srcIndexArray.push_back(static_cast<uint32_t>(bottomRight));
						}
					}

					m_chunkIndexCounts[static_cast<size_t>(chunkIndex)] =
						static_cast<int>(m_srcIndexArray.size())
						- m_chunkIndexOffsets[static_cast<size_t>(chunkIndex)];
				}
			}

			m_indexCount = totalIndices;

			// 元インデックスバッファ（カリングなし描画用）
			m_indexBuffer.Init(
				static_cast<int>(sizeof(uint32_t) * m_srcIndexArray.size()),
				sizeof(uint32_t)
			);
			m_indexBuffer.Copy(m_srcIndexArray.data());

			// 可視インデックスバッファ（カリングあり描画用）
			// 最大でも全インデックス数と同じサイズになるため、同サイズで確保する
			m_visibleIndexBuffer.Init(
				static_cast<int>(sizeof(uint32_t) * m_srcIndexArray.size()),
				sizeof(uint32_t)
			);

			// 可視インデックス配列を最大サイズで事前確保しておく（毎フレームのアロケーションを避ける）
			m_visibleIndexArray.reserve(static_cast<size_t>(totalIndices));
		}


		void OceanMesh::InitShaders(
			const char* fxFilePath,
			const char* vsEntryPoint,
			const char* psEntryPoint
		)
		{
			m_vs = g_engine->GetShaderFromBank(fxFilePath, vsEntryPoint);
			if (m_vs == nullptr)
			{
				m_vs = new Shader;
				m_vs->LoadVS(fxFilePath, vsEntryPoint);
				g_engine->RegistShaderToBank(fxFilePath, vsEntryPoint, m_vs);
			}

			m_ps = g_engine->GetShaderFromBank(fxFilePath, psEntryPoint);
			if (m_ps == nullptr)
			{
				m_ps = new Shader;
				m_ps->LoadPS(fxFilePath, psEntryPoint);
				g_engine->RegistShaderToBank(fxFilePath, psEntryPoint, m_ps);
			}
		}


		void OceanMesh::InitRootSignature()
		{
			D3D12_STATIC_SAMPLER_DESC samplerDescArray[2];

			samplerDescArray[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDescArray[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDescArray[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDescArray[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDescArray[0].MipLODBias = 0;
			samplerDescArray[0].MaxAnisotropy = 0;
			samplerDescArray[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			samplerDescArray[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			samplerDescArray[0].MinLOD = 0.0f;
			samplerDescArray[0].MaxLOD = D3D12_FLOAT32_MAX;
			samplerDescArray[0].ShaderRegister = 0;
			samplerDescArray[0].RegisterSpace = 0;
			samplerDescArray[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			samplerDescArray[1] = samplerDescArray[0];
			samplerDescArray[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			samplerDescArray[1].ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER;
			samplerDescArray[1].MaxAnisotropy = 1;
			samplerDescArray[1].ShaderRegister = 1;

			m_rootSignature.Init(
				samplerDescArray,
				2,	// サンプラー数
				2,	// CBV数（b0, b1）
				3,	// SRV数（t0, t1, t2）
				1	// UAV=0だと内部でシリアライズ失敗するので最低1にする
			);
		}


		void OceanMesh::InitPipelineState(
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat
		)
		{
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT,  0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs->GetCompiledBlob());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps->GetCompiledBlob());
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count = 1;

			int numRenderTargets = 0;
			for (int i = 0; i < MAX_RENDERING_TARGET; ++i)
			{
				if (colorBufferFormat[i] == DXGI_FORMAT_UNKNOWN) break;
				psoDesc.RTVFormats[i] = colorBufferFormat[i];
				numRenderTargets++;
			}
			psoDesc.NumRenderTargets = numRenderTargets;

			m_pipelineState.Init(psoDesc);
		}


		void OceanMesh::InitDescriptorHeap()
		{
			m_descriptorHeap.ResizeConstantBuffer(2);
			m_descriptorHeap.ResizeShaderResource(3);
			m_descriptorHeap.ResizeUnorderAccessResource(1);

			m_descriptorHeap.RegistConstantBuffer(0, m_commonConstantBuffer);
			if (m_expandConstantBuffer.IsValid())
			{
				m_descriptorHeap.RegistConstantBuffer(1, m_expandConstantBuffer);
			}

			m_descriptorHeap.RegistShaderResource(0, m_albedoMap);
			m_descriptorHeap.RegistShaderResource(1, m_normalMap);
			m_descriptorHeap.RegistShaderResource(2, m_specularMap);

			m_descriptorHeap.Commit();
		}


		void OceanMesh::InitComputeShader()
		{
			ID3D12Device* device = g_graphicsEngine->GetD3DDevice();

			m_csShader.LoadCS("Assets/shader/OceanWaveCS.hlsl", "CSMain");

			//------------------------------------------------------------
			// CS用ルートシグネチャを生DX12 APIで構築する
			// レイアウト: [0] CBVテーブル(b0), [1] UAVテーブル(u0)
			//------------------------------------------------------------
			{
				D3D12_DESCRIPTOR_RANGE ranges[2] = {};

				ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
				ranges[0].NumDescriptors = 1;
				ranges[0].BaseShaderRegister = 0;
				ranges[0].RegisterSpace = 0;
				ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				ranges[1].NumDescriptors = 1;
				ranges[1].BaseShaderRegister = 0;
				ranges[1].RegisterSpace = 0;
				ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				D3D12_ROOT_PARAMETER rootParams[2] = {};
				rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
				rootParams[0].DescriptorTable.pDescriptorRanges = &ranges[0];
				rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

				rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
				rootParams[1].DescriptorTable.pDescriptorRanges = &ranges[1];
				rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

				D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
				rsDesc.NumParameters = 2;
				rsDesc.pParameters = rootParams;
				rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				Microsoft::WRL::ComPtr<ID3DBlob> serialized;
				Microsoft::WRL::ComPtr<ID3DBlob> error;
				D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
				device->CreateRootSignature(
					0,
					serialized->GetBufferPointer(),
					serialized->GetBufferSize(),
					IID_PPV_ARGS(&m_csRootSignature)
				);
			}

			//------------------------------------------------------------
			// CS用パイプラインステートを構築する
			//------------------------------------------------------------
			{
				D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.pRootSignature = m_csRootSignature.Get();
				psoDesc.CS = CD3DX12_SHADER_BYTECODE(m_csShader.GetCompiledBlob());
				device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&m_csPipelineState));
			}

			//------------------------------------------------------------
			// CS用定数バッファを永続Mapで確保する
			//------------------------------------------------------------
			{
				const UINT64 cbSize =
					(sizeof(SWaveConstantBuffer) + 255) & ~static_cast<UINT64>(255);

				CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
				CD3DX12_RESOURCE_DESC   resDesc = CD3DX12_RESOURCE_DESC::Buffer(cbSize);
				device->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&m_csCbResource)
				);
				m_csCbResource->Map(0, nullptr, &m_csCbMapped);
			}

			//------------------------------------------------------------
			// UAVバッファ（GPU書き込み先）とReadbackバッファ（CPU読み出し用）を確保する
			//------------------------------------------------------------
			{
				const UINT64 bufSize = static_cast<UINT64>(sizeof(float) * NUM_VERTS);

				CD3DX12_HEAP_PROPERTIES uavHeap(D3D12_HEAP_TYPE_DEFAULT);
				CD3DX12_RESOURCE_DESC   uavDesc = CD3DX12_RESOURCE_DESC::Buffer(
					bufSize,
					D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
				);
				device->CreateCommittedResource(
					&uavHeap,
					D3D12_HEAP_FLAG_NONE,
					&uavDesc,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					nullptr,
					IID_PPV_ARGS(&m_uavBuffer)
				);

				CD3DX12_HEAP_PROPERTIES rbHeap(D3D12_HEAP_TYPE_READBACK);
				CD3DX12_RESOURCE_DESC   rbDesc = CD3DX12_RESOURCE_DESC::Buffer(bufSize);
				device->CreateCommittedResource(
					&rbHeap,
					D3D12_HEAP_FLAG_NONE,
					&rbDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(&m_readbackBuffer)
				);
			}

			//------------------------------------------------------------
			// CS用ディスクリプタヒープ（CBV + UAV）を構築する
			//------------------------------------------------------------
			{
				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.NumDescriptors = 2;
				heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_csDescHeap));
				m_csDescriptorSize = device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);

				// CBV（b0）
				{
					D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
					cbvDesc.BufferLocation = m_csCbResource->GetGPUVirtualAddress();
					cbvDesc.SizeInBytes =
						static_cast<UINT>((sizeof(SWaveConstantBuffer) + 255) & ~255u);

					CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
						m_csDescHeap->GetCPUDescriptorHandleForHeapStart()
					);
					device->CreateConstantBufferView(&cbvDesc, handle);
				}

				// UAV（u0）
				{
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
					uavDesc.Buffer.NumElements = NUM_VERTS;
					uavDesc.Buffer.StructureByteStride = sizeof(float);

					CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
						m_csDescHeap->GetCPUDescriptorHandleForHeapStart(),
						1,
						m_csDescriptorSize
					);
					device->CreateUnorderedAccessView(
						m_uavBuffer.Get(), nullptr, &uavDesc, handle
					);
				}
			}

			//------------------------------------------------------------
			// CS専用コマンドアロケータ・コマンドリストを生成する
			//------------------------------------------------------------
			{
				device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(&m_csCommandAllocator)
				);
				device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					m_csCommandAllocator.Get(),
					nullptr,
					IID_PPV_ARGS(&m_csCommandList)
				);
				m_csCommandList->Close();
			}

			//------------------------------------------------------------
			// GPU完了待ち用フェンスを生成する
			//------------------------------------------------------------
			{
				device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
				m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				m_fenceValue = 0;
			}
		}


		void OceanMesh::DispatchWaveCS(const SWaveConstantBuffer& waveCb)
		{
			// 定数バッファをCPU側で更新する（永続Mapなのでコピーするだけ）
			memcpy(m_csCbMapped, &waveCb, sizeof(SWaveConstantBuffer));

			ID3D12CommandQueue* commandQueue = g_graphicsEngine->GetCommandQueue();

			//------------------------------------------------------------
			// コマンドの記録
			//------------------------------------------------------------
			m_csCommandAllocator->Reset();
			m_csCommandList->Reset(m_csCommandAllocator.Get(), nullptr);

			m_csCommandList->SetComputeRootSignature(m_csRootSignature.Get());
			m_csCommandList->SetPipelineState(m_csPipelineState.Get());

			ID3D12DescriptorHeap* heaps[] = { m_csDescHeap.Get() };
			m_csCommandList->SetDescriptorHeaps(1, heaps);

			CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(
				m_csDescHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_csDescriptorSize
			);
			CD3DX12_GPU_DESCRIPTOR_HANDLE uavHandle(
				m_csDescHeap->GetGPUDescriptorHandleForHeapStart(), 1, m_csDescriptorSize
			);
			m_csCommandList->SetComputeRootDescriptorTable(0, cbvHandle);
			m_csCommandList->SetComputeRootDescriptorTable(1, uavHandle);

			// スレッドグループ: (8, 8, 1) × ディスパッチ (17, 17, 1) で 136×136 スレッドを起動し
			// numVertsPerRow = 129 分をカバーする（129 ÷ 8 = 16.125 → 切り上げ17）
			const int numVertsPerRow = GRID_DIVISION + 1;
			const int groupCount = (numVertsPerRow + 7) / 8;
			m_csCommandList->Dispatch(groupCount, groupCount, 1);

			// UAVバッファをReadbackバッファにコピーする（UAV→COPY_SOURCE状態遷移）
			{
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					m_uavBuffer.Get(),
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					D3D12_RESOURCE_STATE_COPY_SOURCE
				);
				m_csCommandList->ResourceBarrier(1, &barrier);
			}

			m_csCommandList->CopyResource(m_readbackBuffer.Get(), m_uavBuffer.Get());

			// UAVバッファを元の状態に戻す
			{
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					m_uavBuffer.Get(),
					D3D12_RESOURCE_STATE_COPY_SOURCE,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS
				);
				m_csCommandList->ResourceBarrier(1, &barrier);
			}

			m_csCommandList->Close();

			//------------------------------------------------------------
			// コマンドを実行してGPU完了を待つ
			//------------------------------------------------------------
			ID3D12CommandList* lists[] = { m_csCommandList.Get() };
			commandQueue->ExecuteCommandLists(1, lists);

			m_fenceValue++;
			commandQueue->Signal(m_fence.Get(), m_fenceValue);
			if (m_fence->GetCompletedValue() < m_fenceValue)
			{
				m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}

			//------------------------------------------------------------
			// Readbackバッファから波高さキャッシュに読み出す
			//------------------------------------------------------------
			{
				const UINT64 bufSize = static_cast<UINT64>(sizeof(float) * NUM_VERTS);
				D3D12_RANGE readRange = { 0, bufSize };
				void* pData = nullptr;
				m_readbackBuffer->Map(0, &readRange, &pData);
				memcpy(m_waveHeightCache.data(), pData, bufSize);
				D3D12_RANGE writeRange = { 0, 0 };
				m_readbackBuffer->Unmap(0, &writeRange);
			}
		}


		//============================================================
		// Ocean
		//============================================================

		Ocean* Ocean::m_instance = nullptr;


		Ocean::~Ocean()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterOceanParameter>();
		}


		void Ocean::Start()
		{
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET> colorBufferFormat = {
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_UNKNOWN
			};

			// JSONからパラメーターを読み込む
			core::ParameterManager::Get()->LoadParameter<MasterOceanParameter>(
				OCEAN_PARAMETER_JSON_PATH,
				[](const nlohmann::json& j, MasterOceanParameter& p)
				{
					p.baseReflectance = j["baseReflectance"].get<float>();
					p.wave1Amplitude = j["wave1Amplitude"].get<float>();
					p.wave1Frequency = j["wave1Frequency"].get<float>();
					p.wave2Amplitude = j["wave2Amplitude"].get<float>();
					p.wave2Frequency = j["wave2Frequency"].get<float>();
					p.specularPower = j["specularPower"].get<float>();
					p.specularScale = j["specularScale"].get<float>();
					p.ambientScale = j["ambientScale"].get<float>();
				}
			);

			// 読み込んだパラメーターを定数バッファに反映する
			const auto* param = core::ParameterManager::Get()->GetParameter<MasterOceanParameter>();
			if (param != nullptr)
			{
				m_constantBuffer.baseReflectance = param->baseReflectance;
				m_constantBuffer.wave1Amplitude = param->wave1Amplitude;
				m_constantBuffer.wave1Frequency = param->wave1Frequency;
				m_constantBuffer.wave2Amplitude = param->wave2Amplitude;
				m_constantBuffer.wave2Frequency = param->wave2Frequency;
				m_constantBuffer.specularPower = param->specularPower;
				m_constantBuffer.specularScale = param->specularScale;
				m_constantBuffer.ambientScale = param->ambientScale;
			}

			m_oceanMesh.Init(
				"Assets/shader/Ocean.fx",
				"VSMain",
				"PSMain",
				&m_constantBuffer,
				sizeof(m_constantBuffer),
				colorBufferFormat,
				L"Assets/modelData/Ocean/Vol_36_5_Base_Color.DDS",
				L"Assets/modelData/Ocean/Vol_36_5_Normal.DDS",
				L"Assets/modelData/Ocean/Vol_36_5_Roughness.DDS"
			);

			g_renderingEngine->RegisterNatureObject(this);
		}


		void Ocean::Update()
		{
			m_constantBuffer.light = *g_sceneLight->GetLight();
			UpdateWaveOffset();

			// コンピュートシェーダーをグラフィクスrcとは独立して実行し、
			// 波高さキャッシュをCPUに書き出す
			m_oceanMesh.DispatchWaveCS(BuildWaveCb());

			// 波高さキャッシュを元にチャンクAABBを更新する
			// 最大波高さは2つの波の振幅の和（両方が同時に最大値を取った場合）
			const float maxWaveHeight =
				m_constantBuffer.wave1Amplitude + m_constantBuffer.wave2Amplitude;
			m_oceanMesh.BuildChunkAABBs(maxWaveHeight);
		}


		void Ocean::Render(RenderContext& rc)
		{
			// DispatchWaveCS()・BuildChunkAABBs()はUpdate()で完了済みのため、
			// ここでは描画コマンドのみを発行する
			const nsBeastEngine::Frustum& frustum = g_renderingEngine->GetFrustum();
			m_oceanMesh.Draw(rc, CalcWorldMatrix(), frustum);
		}


		float Ocean::SampleWaveHeight(float worldX, float worldZ) const
		{
			constexpr float gridHalfSize = OceanMesh::GRID_SIZE * 0.5f;
			constexpr float cellSize = OceanMesh::GRID_SIZE / static_cast<float>(OceanMesh::GRID_DIVISION);
			constexpr int   numVertsPerRow = OceanMesh::GRID_DIVISION + 1;

			const float localX = (worldX + gridHalfSize) / cellSize;
			const float localZ = (worldZ + gridHalfSize) / cellSize;

			const float clampedX = max(0.0f, min(localX,
				static_cast<float>(OceanMesh::GRID_DIVISION - 1)));
			const float clampedZ = max(0.0f, min(localZ,
				static_cast<float>(OceanMesh::GRID_DIVISION - 1)));

			const int   ix = static_cast<int>(clampedX);
			const int   iz = static_cast<int>(clampedZ);
			const float fx = clampedX - static_cast<float>(ix);
			const float fz = clampedZ - static_cast<float>(iz);

			const float* cache = m_oceanMesh.GetWaveHeightCache();
			const float  p00 = cache[iz * numVertsPerRow + ix];
			const float  p10 = cache[iz * numVertsPerRow + ix + 1];
			const float  p01 = cache[(iz + 1) * numVertsPerRow + ix];
			const float  p11 = cache[(iz + 1) * numVertsPerRow + ix + 1];

			const float top = p00 + (p10 - p00) * fx;
			const float bottom = p01 + (p11 - p01) * fx;
			return top + (bottom - top) * fz;
		}


		OceanMesh::SWaveConstantBuffer Ocean::BuildWaveCb() const
		{
			OceanMesh::SWaveConstantBuffer waveCb;
			waveCb.waveScroll = m_constantBuffer.waveScroll;
			waveCb.wave1Amplitude = m_constantBuffer.wave1Amplitude;
			waveCb.wave1Frequency = m_constantBuffer.wave1Frequency;
			waveCb.wave2Amplitude = m_constantBuffer.wave2Amplitude;
			waveCb.wave2Frequency = m_constantBuffer.wave2Frequency;
			waveCb.gridHalfSize = OceanMesh::GRID_SIZE * 0.5f;
			waveCb.cellSize = OceanMesh::GRID_SIZE / static_cast<float>(OceanMesh::GRID_DIVISION);
			waveCb.numVertsPerRow = OceanMesh::GRID_DIVISION + 1;
			return waveCb;
		}
	}
}