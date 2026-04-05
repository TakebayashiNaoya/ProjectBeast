#pragma once

namespace nsBeastEngine
{
	/// <summary>
	/// 海のグリッドメッシュクラス。
	/// tkmを使わず、C++で動的にグリッド頂点を生成して描画する。
	/// </summary>
	class OceanMesh : public Noncopyable
	{
		/// <summary>
		/// 頂点構造体。
		/// Material::InitPipelineStateの頂点レイアウトに合わせたレイアウト。
		/// </summary>
		struct OceanVertex
		{
			Vector3 pos;		// POSITION  offset:0
			Vector3 normal;		// NORMAL    offset:12
			Vector3 tangent;	// TANGENT   offset:24
			Vector3 biNormal;	// BINORMAL  offset:36
			Vector2 uv;			// TEXCOORD  offset:48
			int     indices[4];	// BLENDINDICES offset:56（ゼロ埋め）
			Vector4 weights;	// BLENDWEIGHT  offset:72（ゼロ埋め）
		};

		/// <summary>
		/// 共通定数バッファ（b0）。
		/// MeshParts::SConstantBufferと同じレイアウト。
		/// </summary>
		struct SCommonConstantBuffer
		{
			Matrix  mWorld;		// ワールド行列
			Matrix  mView;		// ビュー行列
			Matrix  mProj;		// プロジェクション行列
			Vector4 mulColor;	// 乗算カラー
		};

	public:
		/// <summary>
		/// コンピュートシェーダー用定数バッファ（b0）。
		/// OceanWaveCS.hlsl の WaveCb と同じレイアウト。
		/// Ocean::BuildWaveCb() から参照するため public に定義する。
		/// </summary>
		struct SWaveConstantBuffer
		{
			float waveScroll;       // 波のスクロール値
			float wave1Amplitude;   // 波①の振幅
			float wave1Frequency;   // 波①の空間周波数
			float wave2Amplitude;   // 波②の振幅
			float wave2Frequency;   // 波②の空間周波数
			float gridHalfSize;     // グリッド半辺長（= GRID_SIZE / 2）
			float cellSize;         // セルサイズ（= GRID_SIZE / GRID_DIVISION）
			int   numVertsPerRow;   // 1行あたりの頂点数（= GRID_DIVISION + 1）
		};

		// グリッド設定
		// Ocean::BuildWaveCb() / SampleWaveHeight() から参照するため public に定義する
		static constexpr int   GRID_DIVISION = 512;		// 分割数（N×N）
		static constexpr float GRID_SIZE = 5000.0f;	// 1辺の長さ（ワールド単位）


	public:
		OceanMesh() = default;
		~OceanMesh();

		/// <summary>
		/// 初期化。
		/// グリッド頂点・インデックスを生成し、シェーダー・パイプラインを構築する。
		/// </summary>
		void Init(
			const char* fxFilePath,
			const char* vsEntryPoint,
			const char* psEntryPoint,
			void* expandConstantBuffer,
			int expandConstantBufferSize,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			const wchar_t* albedoMapFilePath,
			const wchar_t* normalMapFilePath,
			const wchar_t* specularMapFilePath
		);

		/// <summary>
		/// 描画。
		/// 冒頭でコンピュートシェーダーをディスパッチし、
		/// Readback してキャッシュを更新してから通常描画を行う。
		/// </summary>
		void Draw(RenderContext& rc, const Matrix& mWorld, const SWaveConstantBuffer& waveCb);

		/// <summary>
		/// 拡張定数バッファを更新する。
		/// </summary>
		void UpdateExpandConstantBuffer()
		{
			if (m_expandData != nullptr)
			{
				m_expandConstantBuffer.CopyToVRAM(m_expandData);
			}
		}

		/// <summary>
		/// 波高さキャッシュを取得する。
		/// インデックス = iz * (GRID_DIVISION + 1) + ix。
		/// </summary>
		const float* GetWaveHeightCache() const
		{
			return m_waveHeightCache.data();
		}


	private:
		void CreateGridMesh();

		void InitShaders(
			const char* fxFilePath,
			const char* vsEntryPoint,
			const char* psEntryPoint
		);

		void InitRootSignature();

		void InitPipelineState(
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat
		);

		void InitDescriptorHeap();

		/// <summary>
		/// コンピュートシェーダー関連リソースを初期化する。
		/// ルートシグネチャ・PSO・UAVバッファ・Readbackバッファ・
		/// ディスクリプタヒープ・フェンスをすべて生DX12 APIで構築する。
		/// </summary>
		void InitComputeShader();

		/// <summary>
		/// コンピュートシェーダーをディスパッチし、結果をReadbackする。
		/// </summary>
		void DispatchWaveCS(RenderContext& rc, const SWaveConstantBuffer& waveCb);


	private:
		// NUM_VERTS は内部専用（GRID_DIVISION は public で定義済み）
		static constexpr int NUM_VERTS = (GRID_DIVISION + 1) * (GRID_DIVISION + 1);

		// テクスチャ
		Texture m_albedoMap;
		Texture m_normalMap;
		Texture m_specularMap;

		// 頂点・インデックスバッファ
		VertexBuffer m_vertexBuffer;
		IndexBuffer  m_indexBuffer;
		int          m_indexCount = 0;

		// 描画用シェーダー
		Shader* m_vs = nullptr;
		Shader* m_ps = nullptr;

		// 描画用パイプライン（エンジンラッパー）
		RootSignature m_rootSignature;
		PipelineState m_pipelineState;

		// 描画用定数バッファ
		ConstantBuffer m_commonConstantBuffer;
		ConstantBuffer m_expandConstantBuffer;
		void* m_expandData = nullptr;

		// 描画用ディスクリプタヒープ
		DescriptorHeap m_descriptorHeap;

		//------------------------------------------------------------
		// コンピュートシェーダー関連（生DX12 API で管理）
		//------------------------------------------------------------

		// CS用シェーダー（エンジンラッパーの LoadCS を使う）
		Shader m_csShader;

		// CS用ルートシグネチャ（生ComPtr）
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_csRootSignature;

		// CS用パイプラインステート（生ComPtr）
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_csPipelineState;

		// CS用定数バッファ（CPU側データ）
		// Map/Unmap で毎フレーム更新する
		Microsoft::WRL::ComPtr<ID3D12Resource> m_csCbResource;
		void* m_csCbMapped = nullptr;

		// CS用ディスクリプタヒープ（生ComPtr）
		// CBV(b0) + UAV(u0) の2エントリ
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_csDescHeap;
		UINT m_csDescriptorSize = 0;

		// UAVバッファ（GPU書き込み先）
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uavBuffer;

		// Readbackバッファ（CPU読み出し用）
		Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;

		// CPU側波高さキャッシュ
		std::array<float, NUM_VERTS> m_waveHeightCache = {};

		// GPU完了待ち用フェンス
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		HANDLE                              m_fenceEvent = nullptr;
		UINT64                              m_fenceValue = 0;
	};
}