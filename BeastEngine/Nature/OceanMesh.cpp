#include "BeastEnginePreCompile.h"
#include "OceanMesh.h"

namespace nsBeastEngine
{
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
		// グリッド頂点・インデックスを生成する
		CreateGridMesh();

		// シェーダーをロードする
		InitShaders(fxFilePath, vsEntryPoint, psEntryPoint);

		// テクスチャをロードする
		m_albedoMap.InitFromDDSFile(albedoMapFilePath);
		m_normalMap.InitFromDDSFile(normalMapFilePath);
		m_specularMap.InitFromDDSFile(specularMapFilePath);

		// ルートシグネチャを構築する
		InitRootSignature();

		// パイプラインステートを構築する
		InitPipelineState(colorBufferFormat);

		// 共通定数バッファを初期化する（b0）
		m_commonConstantBuffer.Init(sizeof(SCommonConstantBuffer), nullptr);

		// 拡張定数バッファを初期化する（b1）
		if (expandConstantBuffer != nullptr)
		{
			m_expandConstantBuffer.Init(expandConstantBufferSize, nullptr);
			m_expandData = expandConstantBuffer;
		}

		// ディスクリプタヒープを構築する
		InitDescriptorHeap();

		// コンピュートシェーダー関連リソースを初期化する
		InitComputeShader();
	}


	void OceanMesh::CreateGridMesh()
	{
		const int   numDivision = GRID_DIVISION;
		const float gridHalfSize = GRID_SIZE * 0.5f;
		const float cellSize = GRID_SIZE / static_cast<float>(numDivision);
		const int   numVertsPerRow = numDivision + 1;
		const int   numVerts = numVertsPerRow * numVertsPerRow;

		// 頂点を生成する
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
				// スキンなし：ゼロ埋め
				v.indices[0] = 0;
				v.indices[1] = 0;
				v.indices[2] = 0;
				v.indices[3] = 0;
				v.weights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}

		// 頂点バッファを初期化する
		m_vertexBuffer.Init(
			static_cast<int>(sizeof(OceanVertex) * vertices.size()),
			sizeof(OceanVertex)
		);
		m_vertexBuffer.Copy(vertices.data());

		// インデックスを生成する（四角形1枚を三角形2枚に分割）
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

		// インデックスバッファを初期化する
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
		// 頂点シェーダーをロードする
		m_vs = g_engine->GetShaderFromBank(fxFilePath, vsEntryPoint);
		if (m_vs == nullptr)
		{
			m_vs = new Shader;
			m_vs->LoadVS(fxFilePath, vsEntryPoint);
			g_engine->RegistShaderToBank(fxFilePath, vsEntryPoint, m_vs);
		}

		// ピクセルシェーダーをロードする
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
			2,  // サンプラー数
			2,  // CBV数（b0, b1）
			3,  // SRV数（t0, t1, t2）
			1   // UAV=0だと内部でシリアライズ失敗するので最低1にする
		);
	}


	void OceanMesh::InitPipelineState(
		const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat
	)
	{
		// Material::InitPipelineStateと同じ頂点レイアウトを使用する
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

		// カラーバッファのフォーマットを設定する
		int numRenderTargets = 0;
		for (int i = 0; i < MAX_RENDERING_TARGET; ++i)
		{
			if (colorBufferFormat[i] == DXGI_FORMAT_UNKNOWN)
			{
				break;
			}
			psoDesc.RTVFormats[i] = colorBufferFormat[i];
			numRenderTargets++;
		}
		psoDesc.NumRenderTargets = numRenderTargets;

		m_pipelineState.Init(psoDesc);
	}


	void OceanMesh::InitDescriptorHeap()
	{
		// CBV：b0（共通）、b1（拡張）の2枠
		// SRV：t0（アルベド）、t1（ノーマル）、t2（スペキュラ）の3枠
		m_descriptorHeap.ResizeConstantBuffer(2);
		m_descriptorHeap.ResizeShaderResource(3);
		m_descriptorHeap.ResizeUnorderAccessResource(1);

		m_descriptorHeap.RegistConstantBuffer(0, m_commonConstantBuffer);
		if (m_expandConstantBuffer.IsValid())
		{
			m_descriptorHeap.RegistConstantBuffer(1, m_expandConstantBuffer);
		}

		// テクスチャをSRVに登録する（t0〜t2）
		m_descriptorHeap.RegistShaderResource(0, m_albedoMap);
		m_descriptorHeap.RegistShaderResource(1, m_normalMap);
		m_descriptorHeap.RegistShaderResource(2, m_specularMap);

		m_descriptorHeap.Commit();
	}


	void OceanMesh::InitComputeShader()
	{
		ID3D12Device* device = g_graphicsEngine->GetD3DDevice();

		//------------------------------------------------------------
		// CS用シェーダーをロードする
		//------------------------------------------------------------
		m_csShader.LoadCS("Assets/shader/OceanWaveCS.hlsl", "CSMain");

		//------------------------------------------------------------
		// CS用ルートシグネチャを生DX12 APIで構築する
		// レイアウト: [0] CBV テーブル(b0), [1] UAV テーブル(u0)
		//------------------------------------------------------------
		{
			D3D12_DESCRIPTOR_RANGE ranges[2] = {};

			// b0：定数バッファ
			ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			ranges[0].NumDescriptors = 1;
			ranges[0].BaseShaderRegister = 0;
			ranges[0].RegisterSpace = 0;
			ranges[0].OffsetInDescriptorsFromTableStart = 0;

			// u0：UAV
			ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			ranges[1].NumDescriptors = 1;
			ranges[1].BaseShaderRegister = 0;
			ranges[1].RegisterSpace = 0;
			ranges[1].OffsetInDescriptorsFromTableStart = 0;

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
			D3D12SerializeRootSignature(
				&rsDesc,
				D3D_ROOT_SIGNATURE_VERSION_1,
				serialized.GetAddressOf(),
				error.GetAddressOf()
			);
			device->CreateRootSignature(
				0,
				serialized->GetBufferPointer(),
				serialized->GetBufferSize(),
				IID_PPV_ARGS(m_csRootSignature.GetAddressOf())
			);
		}

		//------------------------------------------------------------
		// CS用パイプラインステートを生DX12 APIで構築する
		//------------------------------------------------------------
		{
			D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_csRootSignature.Get();
			psoDesc.CS = CD3DX12_SHADER_BYTECODE(m_csShader.GetCompiledBlob());
			device->CreateComputePipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_csPipelineState.GetAddressOf())
			);
		}

		//------------------------------------------------------------
		// CS用定数バッファリソースを作成する（UPLOAD ヒープ、Map永続）
		//------------------------------------------------------------
		{
			// 256バイトアライン
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

			// 永続的にMapしておく（WriteOnly相当）
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
		// フェンスを作成する（GPU完了待ち用）
		//------------------------------------------------------------
		device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_fenceValue = 0;
	}


	void OceanMesh::DispatchWaveCS(RenderContext& rc, const SWaveConstantBuffer& waveCb)
	{
		// CS用定数バッファをCPUから更新する（永続Mapにコピー）
		memcpy(m_csCbMapped, &waveCb, sizeof(SWaveConstantBuffer));

		//------------------------------------------------------------
		// コンピュートパスを設定する
		// RenderContext のラッパーメソッドを使う
		//------------------------------------------------------------
		rc.SetDescriptorHeap(m_csDescHeap.Get());
		rc.SetComputeRootSignature(m_csRootSignature.Get());
		rc.SetPipelineState(m_csPipelineState.Get());

		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
			m_csDescHeap->GetGPUDescriptorHandleForHeapStart();

		// テーブル0: CBV(b0)
		rc.SetComputeRootDescriptorTable(0, gpuHandle);

		// テーブル1: UAV(u0)
		gpuHandle.ptr += m_csDescriptorSize;
		rc.SetComputeRootDescriptorTable(1, gpuHandle);

		// グループ数：(GRID_DIVISION+1) 頂点を 8スレッド単位でカバーする
		// ceil(65 / 8) = 9
		const UINT groupCount = (GRID_DIVISION + 1 + 7) / 8;
		rc.Dispatch(groupCount, groupCount, 1);

		//------------------------------------------------------------
		// UAV書き込み完了を保証するバリアを張る
		//------------------------------------------------------------
		auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_uavBuffer.Get());
		rc.ResourceBarrier(uavBarrier);

		//------------------------------------------------------------
		// UAVバッファを COPY_SOURCE に遷移させて Readback にコピーする
		//------------------------------------------------------------
		rc.TransitionResourceState(
			m_uavBuffer.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_SOURCE
		);

		rc.CopyResource(m_readbackBuffer.Get(), m_uavBuffer.Get());

		// UAVバッファを元のステートに戻す
		rc.TransitionResourceState(
			m_uavBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
		);

		//------------------------------------------------------------
		// フェンスをシグナルして GPU の完了を待つ
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
		// Readback バッファを Map して CPU 側キャッシュに書き出す
		//------------------------------------------------------------
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


	void OceanMesh::Draw(RenderContext& rc, const Matrix& mWorld, const SWaveConstantBuffer& waveCb)
	{
		// コンピュートシェーダーをディスパッチし波高さキャッシュを更新する
		DispatchWaveCS(rc, waveCb);

		//------------------------------------------------------------
		// 通常描画
		//------------------------------------------------------------

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
}