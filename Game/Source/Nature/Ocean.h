/**
 * @file Ocean.h
 * @brief 海のクラス（OceanMeshを内包）
 * @author 竹林
 */
#pragma once
#include "Nature/INatureObject.h"
#include "OceanParameter.h"


namespace app
{
	namespace nature
	{
		/**
		 * @brief 海のグリッドメッシュクラス
		 * @details tkmを使わず、C++で動的にグリッド頂点を生成して描画する。
		 *          コンピュートシェーダーで波を計算し、CPUでReadbackして波高さキャッシュを保持する。
		 */
		class OceanMesh : public Noncopyable
		{
		private:
			/**
			 * @brief 頂点構造体
			 * @details Material::InitPipelineStateの頂点レイアウトに合わせたレイアウト
			 */
			struct OceanVertex
			{
				Vector3 pos;		/** 座標 */
				Vector3 normal;		/** 法線 */
				Vector3 tangent;	/** 接線 */
				Vector3 biNormal;	/** 従法線 */
				Vector2 uv;			/** テクスチャ座標 */
				int     indices[4];	/** ボーンインデックス（未使用・ゼロ埋め） */
				Vector4 weights;	/** ボーンウェイト（未使用・ゼロ埋め） */
			};

			/**
			 * @brief 共通定数バッファ（b0）
			 * @details MeshParts::SConstantBufferと同じレイアウト
			 */
			struct SCommonConstantBuffer
			{
				Matrix  mWorld;		/** ワールド行列 */
				Matrix  mView;		/** ビュー行列 */
				Matrix  mProj;		/** プロジェクション行列 */
				Vector4 mulColor;	/** 乗算カラー */
			};

			/**
			 * @brief チャンクAABB構造体
			 * @details チャンク単位のフラスタムカリング判定に使用する
			 */
			struct SChunkAABB
			{
				Vector3 min;	/** ワールド空間AABB最小点 */
				Vector3 max;	/** ワールド空間AABB最大点 */
			};


		public:
			/**
			 * @brief コンピュートシェーダー用定数バッファ（b0）
			 * @details OceanWaveCS.hlslのWaveCbと同じレイアウト
			 */
			struct SWaveConstantBuffer
			{
				float waveScroll;		/** 波のスクロール値 */
				float wave1Amplitude;	/** 波①の振幅 */
				float wave1Frequency;	/** 波①の空間周波数 */
				float wave2Amplitude;	/** 波②の振幅 */
				float wave2Frequency;	/** 波②の空間周波数 */
				float gridHalfSize;		/** グリッド半辺長（= GRID_SIZE / 2） */
				float cellSize;			/** セルサイズ（= GRID_SIZE / GRID_DIVISION） */
				int   numVertsPerRow;	/** 1行あたりの頂点数（= GRID_DIVISION + 1） */
			};

			/** グリッドの1辺の長さ（ワールド単位） */
			static constexpr float GRID_SIZE = 12500.0f;
			/** グリッドの分割数（N×N）。GRID_SIZEに比例させて頂点密度を維持する */
			static constexpr int   GRID_DIVISION = 128;
			/**
			 * @brief チャンク分割数のデフォルト値（縦横共通）
			 * @details GRID_DIVISIONをこの値で均等割りしてチャンクを構成する。
			 *          constexprではないため再コンパイルなしに変更できる。
			 *          GRID_DIVISIONの約数を指定すること（例: 4, 8, 16, 32）。
			 */
			static const int DEFAULT_CHUNK_DIVISION = 8;


		public:
			OceanMesh() = default;
			~OceanMesh();

			/**
			 * @brief 初期化
			 * @param fxFilePath				描画用シェーダーFXファイルのパス
			 * @param vsEntryPoint				描画用頂点シェーダーのエントリポイント名
			 * @param psEntryPoint				描画用ピクセルシェーダーのエントリポイント名
			 * @param expandConstantBuffer		拡張定数バッファの初期データへのポインタ（nullptr可）
			 * @param expandConstantBufferSize	拡張定数バッファのサイズ（バイト単位）
			 * @param colorBufferFormat			描画用のカラーバッファのフォーマット
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
			 * @brief コンピュートシェーダーを独立実行して波高さキャッシュを更新する
			 * @details グラフィクスコマンドリスト（rc）とは独立した専用コマンドリストで
			 *          実行するため、描画パスを汚染しない。Ocean::Update()から呼ぶこと。
			 * @param waveCb コンピュートシェーダー用定数バッファ
			 */
			void DispatchWaveCS(const SWaveConstantBuffer& waveCb);

			/**
			 * @brief チャンクAABBを波高さキャッシュから構築する
			 * @details DispatchWaveCS()のReadback完了後に呼ぶこと。
			 *          各チャンク内の頂点の波高さmin/maxをY範囲として使用する。
			 * @param maxWaveHeight 波の理論的最大高さ（wave1Amplitude + wave2Amplitude）
			 */
			void BuildChunkAABBs(float maxWaveHeight);

			/**
			 * @brief チャンク単位のフラスタムカリングを行いながら描画する
			 * @details DispatchWaveCS()・BuildChunkAABBs()がUpdate()で完了済みであることを前提とする。
			 *          視錐台と交差するチャンクのインデックスのみをGPUバッファに書き込んで描画する。
			 * @param rc		描画コンテキスト
			 * @param mWorld	ワールド行列
			 * @param view		使用するビュー（カメラ行列・フラスタムを含む）
			 */
			void Draw(RenderContext& rc, const Matrix& mWorld, const nsBeastEngine::RenderViewContext& view);

			/**
			 * @brief 拡張定数バッファを更新する
			 */
			void UpdateExpandConstantBuffer()
			{
				if (m_expandData != nullptr)
				{
					m_expandConstantBuffer.CopyToVRAM(m_expandData);
				}
			}

			/**
			 * @brief 波高さキャッシュを取得する
			 * @return 波高さキャッシュの先頭ポインタ
			 */
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
			void InitPipelineState(const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat);
			void InitDescriptorHeap();
			void InitComputeShader();

			/**
			 * @brief 共通の描画コマンドを発行する
			 * @details Draw()の共通処理（定数バッファ更新・パイプライン設定・頂点バッファ設定）をまとめる
			 * @param rc		描画コンテキスト
			 * @param mWorld	ワールド行列
			 * @param view		使用するビュー（カメラ行列の取得に使用）
			 */
			void SetupDrawCommands(RenderContext& rc, const Matrix& mWorld, const nsBeastEngine::RenderViewContext& view);


		private:
			/** グリッド頂点数 */
			static constexpr int NUM_VERTS = (GRID_DIVISION + 1) * (GRID_DIVISION + 1);

			Texture        m_albedoMap;					/** アルベドマップ */
			Texture        m_normalMap;					/** 法線マップ */
			Texture        m_specularMap;				/** スペキュラマップ */

			VertexBuffer   m_vertexBuffer;				/** 頂点バッファ */
			IndexBuffer    m_indexBuffer;				/** 元インデックスバッファ（カリングなし描画用） */
			IndexBuffer    m_visibleIndexBuffer;		/** 可視インデックスバッファ（カリングあり描画用） */
			int            m_indexCount = 0;			/** 元インデックスの総数 */

			/** 元インデックス配列（BuildChunkAABBs・カリング描画用CPUキャッシュ） */
			std::vector<uint32_t> m_srcIndexArray;
			/** チャンクごとの元インデックス先頭オフセット（m_srcIndexArray内） */
			std::vector<int>      m_chunkIndexOffsets;
			/** チャンクごとのインデックス数 */
			std::vector<int>      m_chunkIndexCounts;
			/** 毎フレーム更新される可視インデックス配列 */
			std::vector<uint32_t> m_visibleIndexArray;
			/** チャンクAABB配列（BuildChunkAABBs()で毎フレーム更新） */
			std::vector<SChunkAABB> m_chunkAABBs;
			/** チャンク分割数（縦横共通） */
			int m_chunkDivision = DEFAULT_CHUNK_DIVISION;

			Shader* m_vs = nullptr;						/** 頂点シェーダー */
			Shader* m_ps = nullptr;						/** ピクセルシェーダー */

			RootSignature  m_rootSignature;				/** 描画用ルートシグネチャ */
			PipelineState  m_pipelineState;				/** 描画用パイプラインステート */

			ConstantBuffer m_commonConstantBuffer;		/** 共通定数バッファ（b0） */
			ConstantBuffer m_expandConstantBuffer;		/** 拡張定数バッファ（b1） */
			void* m_expandData = nullptr;				/** 拡張定数バッファに転送するデータへのポインタ */

			DescriptorHeap m_descriptorHeap;			/** ディスクリプタヒープ */

			/** CS用シェーダー */
			Shader m_csShader;
			/** CS用ルートシグネチャ（生ComPtr） */
			Microsoft::WRL::ComPtr<ID3D12RootSignature> m_csRootSignature;
			/** CS用パイプラインステート（生ComPtr） */
			Microsoft::WRL::ComPtr<ID3D12PipelineState> m_csPipelineState;
			/** CS用定数バッファ（永続Map） */
			Microsoft::WRL::ComPtr<ID3D12Resource> m_csCbResource;
			void* m_csCbMapped = nullptr;
			/** CS用ディスクリプタヒープ（CBV + UAV） */
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_csDescHeap;
			UINT m_csDescriptorSize = 0;
			/** UAVバッファ（GPU書き込み先） */
			Microsoft::WRL::ComPtr<ID3D12Resource> m_uavBuffer;
			/** Readbackバッファ（CPU読み出し用） */
			Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;
			/** CPU側波高さキャッシュ */
			std::array<float, NUM_VERTS> m_waveHeightCache = {};
			/** GPU完了待ち用フェンス */
			Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
			HANDLE m_fenceEvent = nullptr;
			UINT64 m_fenceValue = 0;
			/** CS専用コマンドアロケータ（グラフィクスrcと独立して実行するために使用） */
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_csCommandAllocator;
			/** CS専用コマンドリスト（グラフィクスrcと独立して実行するために使用） */
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_csCommandList;
		};


		/**
		 * @brief 海のクラス
		 * @details INatureObjectを継承しRenderingEngineに登録することで、
		 *          Execute()内の正しいタイミングで描画される。
		 *          SampleWaveHeight()でキャラクターや渦潮から波面の高さを取得できる。
		 */
		class Ocean : public nsBeastEngine::INatureObject
		{
		private:
			/**
			 * @brief 定数バッファ
			 * @details HLSLのOceanCb（b1）と完全に一致させること。
			 *          メンバを追加・削除する際はOcean.fxのOceanCbも同時に更新すること。
			 *          チューニングパラメータはJSONから読み込んで設定する（oceanParameter.json参照）。
			 */
			struct OceanConstantBuffer
			{
				Light light;					/** ライト */
				float baseReflectance = 0.0f;	/** 基本反射率 */
				float waveScroll = 0.0f;		/** 波のスクロール値（頂点移動用） */
				float textureScroll = 0.0f;		/** テクスチャのスクロール値（模様流れ用） */
				float wave1Amplitude = 0.0f;	/** 波①の振幅 */
				float wave1Frequency = 0.0f;	/** 波①の空間周波数 */
				float wave2Amplitude = 0.0f;	/** 波②の振幅 */
				float wave2Frequency = 0.0f;	/** 波②の空間周波数 */
				float specularPower = 0.0f;		/** スペキュラのPhong指数（大きいほどハイライトが絞られる） */
				float specularScale = 0.0f;		/** スペキュラ強度の倍率（0.0で照り返しを消せる） */
				float ambientScale = 0.0f;		/** 海専用アンビエント強度倍率（他オブジェクトに影響しない） */
				float padding0 = 0.0f;			/** パディング */
			};


		public:
			/**
			 * @brief 初期化処理
			 * @details OceanMeshを初期化し、RenderingEngineに自身を登録する
			 */
			void Start();

			/**
			 * @brief 更新処理
			 * @details コンピュートシェーダーをここで独立実行する
			 */
			void Update();

			/**
			 * @brief 描画処理
			 * @details RenderingEngine::Execute()内から呼ばれる
			 * @param rc レンダリングコンテキスト
			 */
			void Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view) override;

			/**
			 * @brief 波のスクロール速度を設定
			 */
			inline void SetWaveSpeed(float speed) { m_waveSpeed = speed; }

			/**
			 * @brief 指定ワールドXZ座標における波面Yをバイリニア補間で取得する
			 * @details コンピュートシェーダーが計算したキャッシュから補間する
			 * @param worldX ワールドX座標
			 * @param worldZ ワールドZ座標
			 * @return 補間された波面Yオフセット
			 */
			float SampleWaveHeight(float worldX, float worldZ) const;

			/**
			 * @brief ワールド行列を計算する
			 * @return ワールド行列
			 */
			Matrix CalcWorldMatrix() const
			{
				Matrix mTrans, mRot, mScale;
				mTrans.MakeTranslation(m_position);
				mRot.MakeRotationFromQuaternion(m_rotation);
				mScale.MakeScaling(m_scale);
				return mScale * mRot * mTrans;
			}


		private:
			void UpdateWaveOffset()
			{
				const float deltaTime = g_gameTime->GetFrameDeltaTime();
				m_constantBuffer.waveScroll += deltaTime * m_waveSpeed;
				m_constantBuffer.textureScroll += deltaTime * m_textureSpeed;
			}

			OceanMesh::SWaveConstantBuffer BuildWaveCb() const;


		private:
			OceanMesh           m_oceanMesh;								/** 海のグリッドメッシュ */
			OceanConstantBuffer m_constantBuffer;							/** 定数バッファ */
			Vector3             m_position = g_vec3Zero;					/** 位置 */
			Vector3             m_scale = g_vec3One;						/** スケール */
			Quaternion          m_rotation = Quaternion::Identity;			/** 回転 */
			float               m_waveSpeed = 1.5f;						/** 波のスクロール速度 */
			float               m_textureSpeed = 0.03f;					/** テクスチャのスクロール速度 */


		public:
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new Ocean();
				}
			}

			static Ocean* GetInstance()
			{
				return m_instance;
			}

			static void DestroyInstance()
			{
				delete m_instance;
				m_instance = nullptr;
			}


		private:
			Ocean() = default;
			~Ocean();

			static Ocean* m_instance;
		};
	}
}