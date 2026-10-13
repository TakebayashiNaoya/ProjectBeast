#include "k2EngineLowPreCompile.h"
#include "ConstantBuffer.h"

namespace nsK2EngineLow {
	ConstantBuffer::~ConstantBuffer()
	{
		Release();
	}
	void ConstantBuffer::Release()
	{
		CD3DX12_RANGE readRange(0, 0);
		for (auto& cb : m_constantBuffer) {
			if (cb != nullptr) {
				cb->Unmap(0, &readRange);
				ReleaseD3D12Object(cb);
			}
		}
	}
	void ConstantBuffer::Init(int size, void* srcData)
	{
		Release();
		m_size = size;

		//D3Dデバイスを取得。
		auto device = g_graphicsEngine->GetD3DDevice();


		//定数バッファは256バイトアライメントが要求されるので、256の倍数に切り上げる。
		m_allocSize = (size + 256) & 0xFFFFFF00;
		//定数バッファの作成。
		int bufferNo = 0;
		auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto rDesc = CD3DX12_RESOURCE_DESC::Buffer(m_allocSize);
		for (auto& cb : m_constantBuffer) {
			device->CreateCommittedResource(
				&heapProp,
				D3D12_HEAP_FLAG_NONE,
				&rDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&cb)
			);
			//定数バッファをCPUからアクセス可能な仮想アドレス空間にマッピングする。
			//マップ、アンマップのオーバーヘッドを軽減するためにはこのインスタンスが生きている間は行わない。
			{
				CD3DX12_RANGE readRange(0, 0);        //     intend to read from this resource on the CPU.
				if (cb == nullptr ||
					FAILED(cb->Map(0, &readRange, reinterpret_cast<void**>(&m_constBufferCPU[bufferNo])))) {
					// Creation/Map fails after a device removal. Write the DRED report instead of crashing later.
					g_graphicsEngine->ReportDeviceRemoved("ConstantBuffer::Init map failed");
					return;
				}
			}
			if (srcData != nullptr) {
				memcpy(m_constBufferCPU[bufferNo], srcData, m_size);
			}
			bufferNo++;
		}
		//利用可能にする。
		m_isValid = true;
	}
	void ConstantBuffer::RegistConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, int bufferNo)
	{
		//D3Dデバイスを取得。
		auto device = g_graphicsEngine->GetD3DDevice();
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = m_constantBuffer[bufferNo]->GetGPUVirtualAddress();
		desc.SizeInBytes = m_allocSize;
		device->CreateConstantBufferView(&desc, descriptorHandle);
	}
	void ConstantBuffer::RegistConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
	{
		RegistConstantBufferView(descriptorHandle, static_cast<int>(g_graphicsEngine->GetResourceSlot()));
	}
	void ConstantBuffer::CopyToVRAM(void* data)
	{
		// 今の面に書く。メインビュー（とUpdate中）に書いた内容は、同じフレームのサブビュー用の面にも写す
		// （Update で1フレームに1回だけ書くものを、サブビューでも使えるようにするため）
		const UINT firstSlot = g_graphicsEngine->GetResourceSlot();
		const UINT lastSlot = g_graphicsEngine->IsMainViewResourceSlot() ? firstSlot + 1 : firstSlot;
		for (UINT slot = firstSlot; slot <= lastSlot; slot++) {
			if (m_constBufferCPU[slot] == nullptr) {
				// The mapping failed (device removal during Init). Report instead of writing to null.
				g_graphicsEngine->ReportDeviceRemoved("ConstantBuffer::CopyToVRAM null map");
				return;
			}
			memcpy(m_constBufferCPU[slot], data, m_size);
		}
	}
	D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUVirtualAddress()
	{
		return m_constantBuffer[g_graphicsEngine->GetResourceSlot()]->GetGPUVirtualAddress();
	}
}