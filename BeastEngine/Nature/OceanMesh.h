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
		OceanMesh() = default;
		~OceanMesh() = default;

		/// <summary>
		/// 初期化。
		/// グリッド頂点・インデックスを生成し、シェーダー・パイプラインを構築する。
		/// </summary>
		/// <param name="fxFilePath">fx ファイルパス</param>
		/// <param name="vsEntryPoint">頂点シェーダーエントリーポイント</param>
		/// <param name="psEntryPoint">ピクセルシェーダーエントリーポイント</param>
		/// <param name="expandConstantBuffer">拡張定数バッファ（b1）のポインタ</param>
		/// <param name="expandConstantBufferSize">拡張定数バッファのサイズ</param>
		/// <param name="colorBufferFormat">カラーバッファのフォーマット</param>
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
		/// </summary>
		/// <param name="rc">レンダリングコンテキスト</param>
		/// <param name="mWorld">ワールド行列</param>
		void Draw(RenderContext& rc, const Matrix& mWorld);

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


	private:
		/// <summary>
		/// グリッド頂点・インデックスを生成する。
		/// </summary>
		void CreateGridMesh();

		/// <summary>
		/// シェーダーをロードする。
		/// </summary>
		/// <param name="fxFilePath">fx ファイルパス</param>
		/// <param name="vsEntryPoint">頂点シェーダーエントリーポイント</param>
		/// <param name="psEntryPoint">ピクセルシェーダーエントリーポイント</param>
		void InitShaders(
			const char* fxFilePath,
			const char* vsEntryPoint,
			const char* psEntryPoint
		);

		/// <summary>
		/// ルートシグネチャを構築する。
		/// </summary>
		void InitRootSignature();

		/// <summary>
		/// パイプラインステートを構築する。
		/// </summary>
		/// <param name="colorBufferFormat">カラーバッファのフォーマット</param>
		void InitPipelineState(
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat
		);

		/// <summary>
		/// ディスクリプタヒープを構築する。
		/// </summary>
		void InitDescriptorHeap();


	private:
		// グリッド設定
		static constexpr int   GRID_DIVISION = 64;		// 分割数（N×N）
		static constexpr float GRID_SIZE = 5000.0f;	// 1辺の長さ（ワールド単位）

		// テクスチャ
		Texture m_albedoMap;
		Texture m_normalMap;
		Texture m_specularMap;

		// 頂点・インデックスバッファ
		VertexBuffer m_vertexBuffer;
		IndexBuffer  m_indexBuffer;
		int          m_indexCount = 0;

		// シェーダー
		Shader* m_vs = nullptr;
		Shader* m_ps = nullptr;

		// パイプライン
		RootSignature m_rootSignature;
		PipelineState m_pipelineState;

		// 定数バッファ
		ConstantBuffer m_commonConstantBuffer;		// b0：ワールド・ビュー・プロジェクション
		ConstantBuffer m_expandConstantBuffer;		// b1：拡張（OceanConstantBuffer）
		void* m_expandData = nullptr;		// 拡張定数バッファのCPU側ポインタ

		// ディスクリプタヒープ
		DescriptorHeap m_descriptorHeap;
	};
}