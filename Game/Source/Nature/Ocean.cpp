/**
 * @file Ocean.cpp
 * @brief 海のクラス（OceanMeshを内包）
 * @author 竹林
 */
#include "stdafx.h"
#include "Ocean.h"


namespace app
{
	namespace nature
	{
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
		}


		void OceanMesh::Draw(RenderContext& rc, const Matrix& mWorld)
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
			rc.SetIndexBuffer(m_indexBuffer);
			rc.DrawIndexedInstance(m_indexCount, 1);
		}


		void OceanMesh::CreateGridMesh()
		{
			const int   numDivision = GRID_DIVISION;
			const float gridHalfSize = GRID_SIZE * 0.5f;
			const float cellSize = GRID_SIZE / static_cast<float>(numDivision);
			const int   numVertsPerRow = numDivision + 1;
			const int   numVerts = numVertsPerRow * numVertsPerRow;

			std::vector<OceanVertex> vertices(numVerts);
			for (int z = 0; z <= numDivision; ++z)
			{
				for (int x = 0; x <= numDivision; ++x)
				{
					OceanVertex& v = vertices[z * numVertsPerRow + x];
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

			const int numIndices = numDivision * numDivision * 6;
			std::vector<uint32_t> indices(numIndices);
			int idx = 0;
			for (int z = 0; z < numDivision; ++z)
			{
				for (int x = 0; x < numDivision; ++x)
				{
					const int topLeft = z * numVertsPerRow + x;
					const int topRight = z * numVertsPerRow + x + 1;
					const int bottomLeft = (z + 1) * numVertsPerRow + x;
					const int bottomRight = (z + 1) * numVertsPerRow + x + 1;

					// 三角形①
					indices[idx++] = topLeft;
					indices[idx++] = bottomLeft;
					indices[idx++] = topRight;

					// 三角形②
					indices[idx++] = topRight;
					indices[idx++] = bottomLeft;
					indices[idx++] = bottomRight;
				}
			}

			m_indexCount = numIndices;
			m_indexBuffer.Init(
				static_cast<int>(sizeof(uint32_t) * indices.size()),
				sizeof(uint32_t)
			);
			m_indexBuffer.Copy(indices.data());
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
				rsDesc.NumStaticSamplers = 0;
				rsDesc.pStaticSamplers = nullptr;
				rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

				Microsoft::WRL::ComPtr<ID3DBlob> serialized;
				Microsoft::WRL::ComPtr<ID3DBlob> error;
				D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &error);
				device->CreateRootSignature(
					0,
					serialized->GetBufferPointer(),
					serialized->GetBufferSize(),
					IID_PPV_ARGS(m_csRootSignature.GetAddressOf())
				);
			}

			//------------------------------------------------------------
			// CS用パイプラインステートを構築する
			//------------------------------------------------------------
			{
				D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
				psoDesc.pRootSignature = m_csRootSignature.Get();
				psoDesc.CS = CD3DX12_SHADER_BYTECODE(m_csShader.GetCompiledBlob());
				device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(m_csPipelineState.GetAddressOf()));
			}

			//------------------------------------------------------------
			// CS用定数バッファを作成する（永続Map）
			//------------------------------------------------------------
			{
				const UINT64 cbSize = (sizeof(SWaveConstantBuffer) + 255) & ~255;

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

				D3D12_RESOURCE_DESC resDesc = {};
				resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resDesc.Width = cbSize;
				resDesc.Height = 1;
				resDesc.DepthOrArraySize = 1;
				resDesc.MipLevels = 1;
				resDesc.Format = DXGI_FORMAT_UNKNOWN;
				resDesc.SampleDesc.Count = 1;
				resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				device->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(m_csCbResource.GetAddressOf())
				);

				D3D12_RANGE readRange = { 0, 0 };
				m_csCbResource->Map(0, &readRange, &m_csCbMapped);
			}

			//------------------------------------------------------------
			// UAVバッファを作成する（GPU書き込み先）
			//------------------------------------------------------------
			{
				const UINT64 bufferSize = sizeof(float) * NUM_VERTS;

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

				D3D12_RESOURCE_DESC resDesc = {};
				resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resDesc.Width = bufferSize;
				resDesc.Height = 1;
				resDesc.DepthOrArraySize = 1;
				resDesc.MipLevels = 1;
				resDesc.Format = DXGI_FORMAT_UNKNOWN;
				resDesc.SampleDesc.Count = 1;
				resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

				device->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
					nullptr,
					IID_PPV_ARGS(m_uavBuffer.GetAddressOf())
				);
			}

			//------------------------------------------------------------
			// Readbackバッファを作成する（CPU読み出し用）
			//------------------------------------------------------------
			{
				const UINT64 bufferSize = sizeof(float) * NUM_VERTS;

				D3D12_HEAP_PROPERTIES heapProps = {};
				heapProps.Type = D3D12_HEAP_TYPE_READBACK;

				D3D12_RESOURCE_DESC resDesc = {};
				resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resDesc.Width = bufferSize;
				resDesc.Height = 1;
				resDesc.DepthOrArraySize = 1;
				resDesc.MipLevels = 1;
				resDesc.Format = DXGI_FORMAT_UNKNOWN;
				resDesc.SampleDesc.Count = 1;
				resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				device->CreateCommittedResource(
					&heapProps,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(m_readbackBuffer.GetAddressOf())
				);
			}

			//------------------------------------------------------------
			// CS用ディスクリプタヒープを生DX12 APIで構築する
			// エントリ0: CBV(b0)、エントリ1: UAV(u0)
			//------------------------------------------------------------
			{
				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
				heapDesc.NumDescriptors = 2;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				device->CreateDescriptorHeap(
					&heapDesc,
					IID_PPV_ARGS(m_csDescHeap.GetAddressOf())
				);

				m_csDescriptorSize = device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
				);

				D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle =
					m_csDescHeap->GetCPUDescriptorHandleForHeapStart();

				// エントリ0: CBV
				{
					const UINT64 cbSize = (sizeof(SWaveConstantBuffer) + 255) & ~255;
					D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
					cbvDesc.BufferLocation = m_csCbResource->GetGPUVirtualAddress();
					cbvDesc.SizeInBytes = static_cast<UINT>(cbSize);
					device->CreateConstantBufferView(&cbvDesc, cpuHandle);
				}

				// エントリ1: UAV
				cpuHandle.ptr += m_csDescriptorSize;
				{
					D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
					uavDesc.Format = DXGI_FORMAT_UNKNOWN;
					uavDesc.Buffer.FirstElement = 0;
					uavDesc.Buffer.NumElements = NUM_VERTS;
					uavDesc.Buffer.StructureByteStride = sizeof(float);
					uavDesc.Buffer.CounterOffsetInBytes = 0;
					uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
					device->CreateUnorderedAccessView(
						m_uavBuffer.Get(),
						nullptr,
						&uavDesc,
						cpuHandle
					);
				}
			}

			//------------------------------------------------------------
			// CS専用コマンドアロケータ・コマンドリストを作成する
			// グラフィクスrcとは独立して実行するために専用のものを使用する
			//------------------------------------------------------------
			{
				device->CreateCommandAllocator(
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(m_csCommandAllocator.GetAddressOf())
				);
				device->CreateCommandList(
					0,
					D3D12_COMMAND_LIST_TYPE_DIRECT,
					m_csCommandAllocator.Get(),
					nullptr,
					IID_PPV_ARGS(m_csCommandList.GetAddressOf())
				);
				// 作成直後はオープン状態なので閉じておく
				m_csCommandList->Close();
			}

			// フェンスを作成する（GPU完了待ち用）
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			m_fenceValue = 0;
		}


		void OceanMesh::DispatchWaveCS(const SWaveConstantBuffer& waveCb)
		{
			// CS用定数バッファをCPUから更新する（永続Mapにコピー）
			memcpy(m_csCbMapped, &waveCb, sizeof(SWaveConstantBuffer));

			// 専用コマンドアロケータ・コマンドリストをリセットして記録開始
			m_csCommandAllocator->Reset();
			m_csCommandList->Reset(m_csCommandAllocator.Get(), nullptr);

			// ディスクリプタヒープをセットする
			ID3D12DescriptorHeap* heaps[] = { m_csDescHeap.Get() };
			m_csCommandList->SetDescriptorHeaps(1, heaps);

			// ルートシグネチャとパイプラインステートをセットする
			m_csCommandList->SetComputeRootSignature(m_csRootSignature.Get());
			m_csCommandList->SetPipelineState(m_csPipelineState.Get());

			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
				m_csDescHeap->GetGPUDescriptorHandleForHeapStart();

			// テーブル0: CBV(b0)
			m_csCommandList->SetComputeRootDescriptorTable(0, gpuHandle);

			// テーブル1: UAV(u0)
			gpuHandle.ptr += m_csDescriptorSize;
			m_csCommandList->SetComputeRootDescriptorTable(1, gpuHandle);

			// グループ数：(GRID_DIVISION+1)頂点を8スレッド単位でカバーする
			const UINT groupCount = (GRID_DIVISION + 1 + 7) / 8;
			m_csCommandList->Dispatch(groupCount, groupCount, 1);

			// UAV書き込み完了を保証するバリアを張る
			auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_uavBuffer.Get());
			m_csCommandList->ResourceBarrier(1, &uavBarrier);

			// UAVバッファをCOPY_SOURCEに遷移させてReadbackにコピーする
			auto toCopySrc = CD3DX12_RESOURCE_BARRIER::Transition(
				m_uavBuffer.Get(),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				D3D12_RESOURCE_STATE_COPY_SOURCE
			);
			m_csCommandList->ResourceBarrier(1, &toCopySrc);

			m_csCommandList->CopyResource(m_readbackBuffer.Get(), m_uavBuffer.Get());

			// UAVバッファを元のステートに戻す
			auto toUav = CD3DX12_RESOURCE_BARRIER::Transition(
				m_uavBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS
			);
			m_csCommandList->ResourceBarrier(1, &toUav);

			// コマンドリストを閉じてコマンドキューにサブミットする
			m_csCommandList->Close();
			ID3D12CommandQueue* commandQueue = g_graphicsEngine->GetCommandQueue();
			ID3D12CommandList* cmdLists[] = { m_csCommandList.Get() };
			commandQueue->ExecuteCommandLists(1, cmdLists);

			// フェンスをシグナルしてGPUの完了を待つ
			m_fenceValue++;
			commandQueue->Signal(m_fence.Get(), m_fenceValue);

			if (m_fence->GetCompletedValue() < m_fenceValue)
			{
				m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
				WaitForSingleObject(m_fenceEvent, INFINITE);
			}

			// ReadbackバッファをMapしてCPU側キャッシュに書き出す
			{
				const UINT64 bufferSize = sizeof(float) * NUM_VERTS;
				D3D12_RANGE readRange = { 0, static_cast<SIZE_T>(bufferSize) };
				void* pData = nullptr;
				m_readbackBuffer->Map(0, &readRange, &pData);
				memcpy(m_waveHeightCache.data(), pData, bufferSize);
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
			if (g_renderingEngine != nullptr)
			{
				g_renderingEngine->UnregisterNatureObject(this);
			}
		}


		void Ocean::Start()
		{
			m_constantBuffer.light = *g_sceneLight->GetLight();

			std::array<DXGI_FORMAT, MAX_RENDERING_TARGET> colorBufferFormat = {
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
			};

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
		}


		void Ocean::Render(RenderContext& rc)
		{
			// DispatchWaveCS()はUpdate()で完了済みのため、
			// ここでは純粋に描画コマンドのみを発行する
			m_oceanMesh.Draw(rc, CalcWorldMatrix());
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