/**
 * @file OceanMesh.h
 * @brief 海のグリッドメッシュクラス
 * @author 竹林
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * 海のグリッドメッシュクラス。
	 * tkmを使わず、C++で動的にグリッド頂点を生成して描画する。
	 */
	class OceanMesh : public Noncopyable
	{
	private:
		/**
		 * @brief 頂点構造体
		 * @details Material::InitPipelineStateの頂点レイアウトに合わせたレイアウト。
		 */
		struct OceanVertex
		{
			Vector3 pos;		/** 座標 */
			Vector3 normal;		/** 法線（面の表を示す単位ベクトル。ライティングで必要。） */
			Vector3 tangent;	/** 接線（法線マップを正しく適用するための補助ベクトル。） */
			Vector3 biNormal;	/** 従法線（tangent・normal と合わせて3本で「面の座標系（TBN行列）」を作る第3のベクトル。） */
			Vector2 uv;			/** テクスチャ座標（このメッシュのどの位置に、テクスチャのどのピクセルを貼るかを示す2D座標。） */

			/** 以下は使用しないが、スキニング（骨格アニメーション）に対応させるためのフィールド。ゼロ埋めしている。*/
			int     indices[4];	/** ボーンインデックス（スキニングのために「どのボーンの影響を受けるか」を最大4本分記録するフィールド。） */
			Vector4 weights;	/** ボーンウェイト（スキニングのために「各ボーンからどれだけ影響を受けるか」の割合（0.0〜1.0）を最大4本分記録するフィールド。） */
		};

		/**
		 * @brief 共通定数バッファ（b0）
		 * @details MeshParts::SConstantBufferと同じレイアウト。
		 */
		struct SCommonConstantBuffer
		{
			Matrix  mWorld;		/** ワールド行列 */
			Matrix  mView;		/** ビュー行列 */
			Matrix  mProj;		/** プロジェクション行列 */
			Vector4 mulColor;	/** 乗算カラー */
		};


	public:
		/**
		 * コンピュートシェーダー用定数バッファ（b0）
		 * OceanWaveCS.hlsl の WaveCb と同じレイアウト。
		 * Ocean::BuildWaveCb() から参照するため public に定義する。
		 */
		struct SWaveConstantBuffer
		{
			float waveScroll;       /** 波のスクロール値 */
			float wave1Amplitude;   /** 波①の振幅 */
			float wave1Frequency;   /** 波①の空間周波数 */
			float wave2Amplitude;   /** 波②の振幅 */
			float wave2Frequency;   /** 波②の空間周波数 */
			float gridHalfSize;     /** グリッド半辺長（= GRID_SIZE / 2） */
			float cellSize;         /** セルサイズ（= GRID_SIZE / GRID_DIVISION） */
			int   numVertsPerRow;   /** 1行あたりの頂点数（= GRID_DIVISION + 1） */
		};

		/**
		 * グリッド設定
		 * Ocean::BuildWaveCb() / SampleWaveHeight() から参照するため public に定義する
		 */
		static constexpr float GRID_SIZE = 5000.0f;		/** 1辺の長さ（ワールド単位） */
		static constexpr int   GRID_DIVISION = 512;		/** 分割数（N×N） */


	public:
		OceanMesh() = default;
		~OceanMesh();

		/**
		 * @brief 初期化
		 * @details グリッド頂点・インデックスを生成し、シェーダー・パイプラインを構築する。
		 * @param fxFilePath				描画用シェーダーFXファイルのパス
		 * @param vsEntryPoint				描画用頂点シェーダーのエントリポイント名
		 * @param psEntryPoint				描画用ピクセルシェーダーのエントリポイント名
		 * @param expandConstantBuffer		拡張定数バッファの初期データへのポインタ（nullptr可）
		 * @param expandConstantBufferSize	拡張定数バッファのサイズ（バイト単位。0ならば expandConstantBuffer は nullptr 扱い）
		 * @param colorBufferFormat			描画用のカラーバッファのフォーマット（アルファチャンネルの有無などに応じて、シェーダー内で分岐処理を行うために必要）
		 * @param albedoMapFilePath			アルベドマップのファイルパス
		 * @param normalMapFilePath			法線マップのファイルパス
		 * @param specularMapFilePath		スペキュラマップのファイルパス
		 */
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

		/**
		 * @brief 描画
		 * @details 冒頭でコンピュートシェーダーをディスパッチし、Readback してキャッシュを更新してから通常描画を行う。
		 * @param rc		描画コンテキスト
		 * @param mWorld	ワールド行列
		 * @param waveCb	コンピュートシェーダー用定数バッファ（SWaveConstantBuffer）のインスタンス。Ocean::BuildWaveCb() で構築される想定。
		 */
		void Draw(RenderContext& rc, const Matrix& mWorld, const SWaveConstantBuffer& waveCb);

		/**
		 * @brief 拡張定数バッファを更新する。
		 */
		void UpdateExpandConstantBuffer()
		{
			if (m_expandData != nullptr)
			{
				m_expandConstantBuffer.CopyToVRAM(m_expandData);
			}
		}

		/**
		 * @brief 波高さキャッシュを取得する。
		 * @details コンピュートシェーダーの結果をCPU側でReadbackして格納しているキャッシュ。インデックス = iz * (GRID_DIVISION + 1) + ix。
		 * 			波の高さをCPU側で参照したい場合に利用する。描画前にDraw()を呼び出してキャッシュを最新化しておく必要がある。
		 */
		const float* GetWaveHeightCache() const
		{
			return m_waveHeightCache.data();
		}


	private:
		/**
		 * @brief	グリッド頂点・インデックスを生成し、頂点バッファ・インデックスバッファを初期化する。
		 * @details	生成されるグリッドはXY平面上に配置され、中心が原点になるようにする。
		 *			頂点の法線はすべて(0, 0, 1)（Z軸正方向）で、接線はすべて(1, 0, 0)（X軸正方向）で、従法線はすべて(0, 1, 0)（Y軸正方向）で初期化する。
		 *			UV座標はグリッド全体で(0,0)〜(1,1)になるように割り当てる。
		 */
		void CreateGridMesh();

		/**
		 * @brief 描画用シェーダーを初期化する。
		 * @details fxFilePath で指定されたFXファイルから、vsEntryPoint で指定された頂点シェーダーと、psEntryPoint で指定されたピクセルシェーダーをそれぞれ読み込んで初期化する。
		 * @param fxFilePath		描画用シェーダーFXファイルのパス
		 * @param vsEntryPoint		描画用頂点シェーダーのエントリポイント名
		 * @param psEntryPoint		描画用ピクセルシェーダーのエントリポイント名
		 */
		void InitShaders(
			const char* fxFilePath,
			const char* vsEntryPoint,
			const char* psEntryPoint
		);

		/**
		 * @brief 描画用ルートシグネチャを初期化する。
		 * @details 描画用シェーダーのリソースバインディングに合わせて、ルートパラメータを設定する必要がある。
			 - b0: 共通定数バッファ（SCommonConstantBuffer）
			 - t0: アルベドマップ
			 - t1: 法線マップ
			 - t2: スペキュラマップ
			 なお、描画用シェーダーは、これらのリソースをルートパラメータに基づいてアクセスする必要がある。
		 */
		void InitRootSignature();

		/**
		 * @brief 描画用パイプラインステートを初期化する。
		 * @details 描画用シェーダーの入力レイアウトは、OceanVertex のレイアウトに合わせる必要がある。
		 * 			また、blendState はアルファブレンド有効の設定にする必要がある（アルファチャンネルの有無に応じてシェーダー内で分岐処理を行うため）。
		 * 			colorBufferFormat は、描画用のカラーバッファのフォーマットを指定する。これもシェーダー内で分岐処理を行うために必要。
		 * @param colorBufferFormat 描画用のカラーバッファのフォーマット（アルファチャンネルの有無などに応じて、シェーダー内で分岐処理を行うために必要）
		 */
		void InitPipelineState(const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat);

		/**
		 * @brief 描画用ディスクリプタヒープを初期化する。
		 * @details アルベドマップ・法線マップ・スペキュラマップのSRVを格納するためのディスクリプタヒープを初期化する。
		 */
		void InitDescriptorHeap();

		/**
		 * @brief コンピュートシェーダー関連リソースを初期化する。
		 * @details ルートシグネチャ・PSO・UAVバッファ・Readbackバッファ・ディスクリプタヒープ・フェンスをすべて生DX12 APIで構築する。
		 *			ルートシグネチャは、b0: SWaveConstantBuffer、u0: UAVバッファの2エントリで構成する。
		 *			PSOは、m_csShader をコンピュートシェーダーとしてセットして構築する。
		 *			UAVバッファは、NUM_VERTS 個の float を格納できるサイズで構築する。
		 *			Readbackバッファも同様のサイズで構築する。
		 *			ディスクリプタヒープは、CBV(b0) + UAV(u0) の2エントリを格納できるサイズで構築し、ルートシグネチャのエントリに対応させてCBVとUAVのディスクリプタを作成してセットする。
		 *			フェンスは、コンピュートシェーダーのGPU完了待ちに使用するために構築する。
		 *			これらのリソースは、DispatchWaveCS() 内でコンピュートシェーダーをディスパッチして結果をReadbackする際に使用される。
		 */
		void InitComputeShader();

		/**
		 * @brief コンピュートシェーダーをディスパッチし、結果をReadbackする。
		 * @details m_csRootSignature と m_csPipelineState をセットして、引数の waveCb をコンピュートシェーダー用定数バッファに転送し、m_uavBuffer を UAV としてバインドして、グリッド頂点数に応じたスレッドグループ数で Dispatch する。
		 * @param rc		描画コンテキスト
		 * @param waveCb	コンピュートシェーダー用定数バッファ（SWaveConstantBuffer）のインスタンス。Ocean::BuildWaveCb() で構築される想定。
		 */
		void DispatchWaveCS(RenderContext& rc, const SWaveConstantBuffer& waveCb);


	private:
		/**
		 * @brief	グリッド頂点数
		 * @details NUM_VERTS は内部専用（GRID_DIVISION は public で定義済み）
		 */
		static constexpr int NUM_VERTS = (GRID_DIVISION + 1) * (GRID_DIVISION + 1);

		Texture m_albedoMap;					/** アルベドマップ */
		Texture m_normalMap;					/** 法線マップ */
		Texture m_specularMap;					/** スペキュラマップ */

		VertexBuffer m_vertexBuffer;			/** 頂点バッファ */
		IndexBuffer  m_indexBuffer;				/** インデックスバッファ */
		int          m_indexCount = 0;			/** インデックスの数 */

		Shader* m_vs = nullptr;					/** 頂点シェーダー */
		Shader* m_ps = nullptr;					/** ピクセルシェーダー */

		RootSignature m_rootSignature;			/** 描画用ルートシグネチャ */
		PipelineState m_pipelineState;			/** 描画用パイプラインステート */

		ConstantBuffer m_commonConstantBuffer;	/** 共通定数バッファ（SCommonConstantBuffer） */
		ConstantBuffer m_expandConstantBuffer;	/** 拡張定数バッファ（ユーザーが任意のデータを転送するための定数バッファ。サイズは Init() の引数 expandConstantBufferSize で指定する。） */
		void* m_expandData = nullptr;			/** 拡張定数バッファに転送するデータへのポインタ。ユーザーが任意のデータをセットしておくためのフィールド。Init() の引数 expandConstantBuffer で指定された値がセットされる。） */

		DescriptorHeap m_descriptorHeap;		/** アルベドマップ・法線マップ・スペキュラマップのSRVを格納するためのディスクリプタヒープ */




		//================================================//
		// コンピュートシェーダー関連（生DX12 API で管理）
		//================================================//

	private:
		/** CS用シェーダー（エンジンラッパーの LoadCS を使う） */
		Shader m_csShader;

		/** CS用ルートシグネチャ（生ComPtr） */
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_csRootSignature;

		/** CS用パイプラインステート（生ComPtr） */
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_csPipelineState;

		/**
		 * @brief	CS用定数バッファ（CPU側データ）
		 * @details Map/Unmap で毎フレーム更新する
		 */
		Microsoft::WRL::ComPtr<ID3D12Resource> m_csCbResource;
		void* m_csCbMapped = nullptr;

		/**
		 * CS用ディスクリプタヒープ（生ComPtr）
		 * CBV(b0) + UAV(u0) の2エントリ
		 */
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_csDescHeap;
		UINT m_csDescriptorSize = 0;

		/** UAVバッファ（GPU書き込み先） */
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uavBuffer;

		/** Readbackバッファ（CPU読み出し用） */
		Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;

		/** CPU側波高さキャッシュ */
		std::array<float, NUM_VERTS> m_waveHeightCache = {};

		/** GPU完了待ち用フェンス */
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;				/** 生ComPtrで管理 */
		HANDLE                              m_fenceEvent = nullptr;	/** フェーン完了待ち用イベントハンドル */
		UINT64                              m_fenceValue = 0;		/** フェンス値 */
	};
}