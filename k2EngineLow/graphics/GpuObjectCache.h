#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <cstdint>

namespace nsK2EngineLow {
	/// <summary>
	/// ルートシグネチャ・パイプラインステート（PSO）のキャッシュ。
	/// マテリアルはモデルを作るたびにルートシグネチャ1つとPSOを4つ作っていたが、PSOの作成は重く、
	/// 同じモデルを何体も作る（フィーバーで子ペンギンが降ってくる等）と1体で十数msかかっていた。
	/// 中身がまったく同じ設定なら作成済みのものを共有する。
	/// 取得した側は参照を1つ持つ（いつも通り Release すればよい）。キャッシュも参照を1つ持ち続ける。
	/// </summary>
	class GpuObjectCache {
	public:
		/// <summary>
		/// ルートシグネチャを取得する（無ければ作る）。シリアライズ済みのバイト列が同じものを共有する。
		/// </summary>
		/// <returns>参照を1つ加えたルートシグネチャ（失敗時は nullptr）</returns>
		ID3D12RootSignature* GetRootSignature(ID3D12Device* device, const void* blob, const size_t blobSize, HRESULT& outHr)
		{
			const std::string key(static_cast<const char*>(blob), blobSize);
			std::lock_guard<std::mutex> lock(m_mutex);
			auto it = m_rootSignatures.find(key);
			if (it != m_rootSignatures.end()) {
				it->second->AddRef();
				outHr = S_OK;
				return it->second;
			}
			ID3D12RootSignature* rootSignature = nullptr;
			outHr = device->CreateRootSignature(0, blob, blobSize, IID_PPV_ARGS(&rootSignature));
			if (FAILED(outHr) || rootSignature == nullptr) {
				return nullptr;
			}
			rootSignature->AddRef();	// キャッシュの分
			m_rootSignatures.emplace(key, rootSignature);
			return rootSignature;
		}

		/// <summary>
		/// グラフィックス用PSOを取得する（無ければ作る）。設定・シェーダーの中身・頂点レイアウトが同じものを共有する。
		/// </summary>
		/// <returns>参照を1つ加えたPSO（失敗時は nullptr）</returns>
		ID3D12PipelineState* GetGraphicsPipelineState(ID3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc, HRESULT& outHr)
		{
			// ストリーム出力・キャッシュ済みPSOを使うものは対象外（キーを作らずそのまま作る）
			if (desc.StreamOutput.NumEntries != 0 || desc.CachedPSO.pCachedBlob != nullptr) {
				ID3D12PipelineState* pso = nullptr;
				outHr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
				return SUCCEEDED(outHr) ? pso : nullptr;
			}

			const std::string key = MakeKey(desc);
			std::lock_guard<std::mutex> lock(m_mutex);
			auto it = m_pipelineStates.find(key);
			if (it != m_pipelineStates.end()) {
				it->second->AddRef();
				outHr = S_OK;
				return it->second;
			}
			ID3D12PipelineState* pso = nullptr;
			outHr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
			if (FAILED(outHr) || pso == nullptr) {
				return nullptr;
			}
			pso->AddRef();	// キャッシュの分
			m_pipelineStates.emplace(key, pso);
			return pso;
		}

	private:
		/// <summary>
		/// バイト列の64bitハッシュ（FNV-1a）
		/// </summary>
		static uint64_t HashBytes(const void* data, const size_t size)
		{
			uint64_t hash = 1469598103934665603ull;
			const uint8_t* bytes = static_cast<const uint8_t*>(data);
			for (size_t i = 0; i < size; ++i) {
				hash ^= bytes[i];
				hash *= 1099511628211ull;
			}
			return hash;
		}
		template<class T>
		static void Append(std::string& key, const T& value)
		{
			key.append(reinterpret_cast<const char*>(&value), sizeof(T));
		}
		static void AppendShader(std::string& key, const D3D12_SHADER_BYTECODE& shader)
		{
			Append(key, static_cast<uint64_t>(shader.BytecodeLength));
			Append(key, (shader.pShaderBytecode != nullptr) ? HashBytes(shader.pShaderBytecode, shader.BytecodeLength) : 0ull);
		}
		/// <summary>
		/// PSOのキーを作る（ポインタの先の中身まで含める。番地だけだと別の内容を取り違える恐れがあるため）
		/// </summary>
		static std::string MakeKey(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
		{
			std::string key;
			key.reserve(512);
			// ルートシグネチャは GetRootSignature() で共有されるので、同じ内容なら同じ番地になる
			Append(key, reinterpret_cast<uintptr_t>(desc.pRootSignature));
			AppendShader(key, desc.VS);
			AppendShader(key, desc.PS);
			AppendShader(key, desc.DS);
			AppendShader(key, desc.HS);
			AppendShader(key, desc.GS);
			Append(key, desc.BlendState);
			Append(key, desc.SampleMask);
			Append(key, desc.RasterizerState);
			// 深度ステンシルは UINT8 の後ろに詰め物があるので項目ごとに入れる
			const D3D12_DEPTH_STENCIL_DESC& ds = desc.DepthStencilState;
			Append(key, ds.DepthEnable);
			Append(key, ds.DepthWriteMask);
			Append(key, ds.DepthFunc);
			Append(key, ds.StencilEnable);
			Append(key, ds.StencilReadMask);
			Append(key, ds.StencilWriteMask);
			Append(key, ds.FrontFace);
			Append(key, ds.BackFace);
			Append(key, desc.InputLayout.NumElements);
			for (UINT i = 0; i < desc.InputLayout.NumElements; ++i) {
				const D3D12_INPUT_ELEMENT_DESC& element = desc.InputLayout.pInputElementDescs[i];
				if (element.SemanticName != nullptr) key.append(element.SemanticName);
				key.push_back('\0');
				Append(key, element.SemanticIndex);
				Append(key, element.Format);
				Append(key, element.InputSlot);
				Append(key, element.AlignedByteOffset);
				Append(key, element.InputSlotClass);
				Append(key, element.InstanceDataStepRate);
			}
			Append(key, desc.IBStripCutValue);
			Append(key, desc.PrimitiveTopologyType);
			Append(key, desc.NumRenderTargets);
			for (const DXGI_FORMAT format : desc.RTVFormats) Append(key, format);
			Append(key, desc.DSVFormat);
			Append(key, desc.SampleDesc);
			Append(key, desc.NodeMask);
			Append(key, desc.Flags);
			return key;
		}

		std::unordered_map<std::string, ID3D12RootSignature*> m_rootSignatures;
		std::unordered_map<std::string, ID3D12PipelineState*> m_pipelineStates;
		std::mutex m_mutex;
	};

	/// <summary>
	/// キャッシュの取得（ヘッダーだけで使えるよう関数内の static にしている）。
	/// </summary>
	inline GpuObjectCache& GetGpuObjectCache()
	{
		static GpuObjectCache s_cache;
		return s_cache;
	}
}
