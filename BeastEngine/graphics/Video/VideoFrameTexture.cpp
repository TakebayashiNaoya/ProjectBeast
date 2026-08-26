/**
 * @file VideoFrameTexture.cpp
 * @brief CPU→GPU の動的テクスチャ更新クラスの実装
 */
#include "BeastEnginePreCompile.h"
#include "VideoFrameTexture.h"


namespace nsBeastEngine
{
	VideoFrameTexture::~VideoFrameTexture()
	{
		// k2EngineLow の遅延解放を通じて GPU 使用完了後に解放する
		if (m_gpuTexture)
		{
			nsK2EngineLow::ReleaseD3D12Object(m_gpuTexture);
			m_gpuTexture = nullptr;
		}
		if (m_uploadBuffer)
		{
			nsK2EngineLow::ReleaseD3D12Object(m_uploadBuffer);
			m_uploadBuffer = nullptr;
		}
		// m_k2Texture のデストラクタも ReleaseD3D12Object を呼ぶ（参照カウント -1）
	}


	void VideoFrameTexture::Init(int width, int height)
	{
		m_width = width;
		m_height = height;

		auto* device = g_graphicsEngine->GetD3DDevice();

		// ----- 1. GPU テクスチャリソースのディスクリプタ -----
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = static_cast<UINT64>(width);
		texDesc.Height = static_cast<UINT>(height);
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// ----- 2. DEFAULT ヒープにテクスチャを確保（初期状態: COPY_DEST）-----
		auto defaultHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = device->CreateCommittedResource(
			&defaultHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_gpuTexture)
		);

		if (FAILED(hr))
		{
			K2_LOG("VideoFrameTexture::Init: GPU テクスチャの作成に失敗しました (hr=0x%08X)\n", hr);
			return;
		}

		// ----- 3. UPLOAD ヒープにステージングバッファを確保 -----
		UINT   numRows = 0;
		UINT64 rowBytes = 0;
		UINT64 totalBytes = 0;
		device->GetCopyableFootprints(&texDesc, 0, 1, 0, &m_footprint, &numRows, &rowBytes, &totalBytes);

		auto uploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto bufDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
		hr = device->CreateCommittedResource(
			&uploadHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&bufDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadBuffer)
		);

		if (FAILED(hr))
		{
			K2_LOG("VideoFrameTexture::Init: アップロードバッファの作成に失敗しました (hr=0x%08X)\n", hr);
			return;
		}

		// ----- 4. 初回同期アップロード（透明黒）+ PIXEL_SHADER_RESOURCE へ遷移 -----
		// Init() はゲームループ外から呼ばれることもあるため、
		// 専用のコマンドリストを作成して同期実行する
		{
			ComPtr<ID3D12CommandAllocator>  allocator;
			ComPtr<ID3D12GraphicsCommandList> cmdList;
			hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
			if (FAILED(hr))
			{
				K2_LOG("VideoFrameTexture::Init: コマンドアロケータの作成に失敗しました (hr=0x%08X)\n", hr);
				return;
			}
			hr = device->CreateCommandList(
				0,
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				allocator.Get(),
				nullptr,
				IID_PPV_ARGS(&cmdList)
			);
			if (FAILED(hr))
			{
				K2_LOG("VideoFrameTexture::Init: コマンドリストの作成に失敗しました (hr=0x%08X)\n", hr);
				return;
			}

			// ゼロクリアデータをアップロードバッファに書き込む
			uint8_t* mapped = nullptr;
			D3D12_RANGE readRange = { 0, 0 };
			m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mapped));
			memset(mapped, 0, static_cast<size_t>(totalBytes));
			m_uploadBuffer->Unmap(0, nullptr);

			// CopyTextureRegion でアップロードバッファ → GPU テクスチャにコピー
			D3D12_TEXTURE_COPY_LOCATION dst = {};
			dst.pResource = m_gpuTexture;
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = 0;

			D3D12_TEXTURE_COPY_LOCATION src = {};
			src.pResource = m_uploadBuffer;
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint = m_footprint;

			cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

			// COPY_DEST → PIXEL_SHADER_RESOURCE に遷移
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_gpuTexture,
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			);
			cmdList->ResourceBarrier(1, &barrier);
			cmdList->Close();

			// コマンドを実行してフェンスで GPU 完了を待機
			ID3D12CommandList* lists[] = { cmdList.Get() };
			g_graphicsEngine->GetCommandQueue()->ExecuteCommandLists(1, lists);

			ComPtr<ID3D12Fence> fence;
			device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
			HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			g_graphicsEngine->GetCommandQueue()->Signal(fence.Get(), 1);
			fence->SetEventOnCompletion(1, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
			CloseHandle(fenceEvent);
		}

		// ----- 5. k2EngineLow::Texture にラップ（SRV 登録のため）-----
		// InitFromD3DResource は AddRef するため、
		// m_gpuTexture（自分の参照）と合わせて参照カウント 2 になる
		m_k2Texture.InitFromD3DResource(m_gpuTexture);

		m_isInitialized = true;
	}


	void VideoFrameTexture::UploadFrame(const uint8_t* rgbaPixels)
	{
		if (!m_isInitialized || !rgbaPixels) return;

		auto* cmdList = g_graphicsEngine->GetCommandList();

		// ----- アップロードバッファに CPU データを書き込む -----
		uint8_t* mapped = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mapped));

		const int srcRowPitch = m_width * 4;
		const int dstRowPitch = static_cast<int>(m_footprint.Footprint.RowPitch);
		for (int y = 0; y < m_height; ++y)
		{
			memcpy(
				mapped + y * dstRowPitch,
				rgbaPixels + y * srcRowPitch,
				srcRowPitch
			);
		}

		m_uploadBuffer->Unmap(0, nullptr);

		// ----- PIXEL_SHADER_RESOURCE → COPY_DEST -----
		auto toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
			m_gpuTexture,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_COPY_DEST
		);
		cmdList->ResourceBarrier(1, &toCopyDest);

		// ----- CopyTextureRegion -----
		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = m_gpuTexture;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = 0;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = m_uploadBuffer;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = m_footprint;

		cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		// ----- COPY_DEST → PIXEL_SHADER_RESOURCE -----
		auto toSRV = CD3DX12_RESOURCE_BARRIER::Transition(
			m_gpuTexture,
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);
		cmdList->ResourceBarrier(1, &toSRV);
	}
}
