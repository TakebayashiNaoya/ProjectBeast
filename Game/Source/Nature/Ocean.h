/**
 * @file Ocean.h
 * @brief 海のクラス（OceanMeshを内包）
 * @author 竹林
 */
#pragma once
#include "Nature/INatureObject.h"


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
			static constexpr float GRID_SIZE = 5000.0f;
			/** グリッドの分割数（N×N） */
			static constexpr int   GRID_DIVISION = 512;


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
			 * @brief 描画
			 * @details 冒頭でコンピュートシェーダーをディスパッチし、Readbackしてキャッシュを更新してから通常描画を行う
			 * @param rc		描画コンテキスト
			 * @param mWorld	ワールド行列
			 * @param waveCb	コンピュートシェーダー用定数バッファ
			 */
			void Draw(RenderContext& rc, const Matrix& mWorld, const SWaveConstantBuffer& waveCb);

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
			void DispatchWaveCS(RenderContext& rc, const SWaveConstantBuffer& waveCb);


		private:
			/** グリッド頂点数 */
			static constexpr int NUM_VERTS = (GRID_DIVISION + 1) * (GRID_DIVISION + 1);

			Texture        m_albedoMap;					/** アルベドマップ */
			Texture        m_normalMap;					/** 法線マップ */
			Texture        m_specularMap;				/** スペキュラマップ */

			VertexBuffer   m_vertexBuffer;				/** 頂点バッファ */
			IndexBuffer    m_indexBuffer;				/** インデックスバッファ */
			int            m_indexCount = 0;			/** インデックスの数 */

			Shader* m_vs = nullptr;				/** 頂点シェーダー */
			Shader* m_ps = nullptr;				/** ピクセルシェーダー */

			RootSignature  m_rootSignature;				/** 描画用ルートシグネチャ */
			PipelineState  m_pipelineState;				/** 描画用パイプラインステート */

			ConstantBuffer m_commonConstantBuffer;		/** 共通定数バッファ（b0） */
			ConstantBuffer m_expandConstantBuffer;		/** 拡張定数バッファ（b1） */
			void* m_expandData = nullptr;		/** 拡張定数バッファに転送するデータへのポインタ */

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
			 */
			struct OceanConstantBuffer
			{
				Light light;					/** ライト */
				float baseReflectance = 0.0f;	/** 基本反射率 */
				float waveScroll = 0.0f;	/** 波のスクロール値（頂点移動用） */
				float textureScroll = 0.0f;	/** テクスチャのスクロール値（模様流れ用） */
				float wave1Amplitude = 5.0f;	/** 波①の振幅 */
				float wave1Frequency = 0.025f;	/** 波①の空間周波数 */
				float wave2Amplitude = 2.0f;	/** 波②の振幅 */
				float wave2Frequency = 0.06f;	/** 波②の空間周波数 */
			};


		public:
			/**
			 * @brief 初期化処理
			 * @details OceanMeshを初期化し、RenderingEngineに自身を登録する
			 */
			void Start();

			/**
			 * @brief 更新処理
			 */
			void Update();

			/**
			 * @brief 描画処理
			 * @details RenderingEngine::Execute()内から呼ばれる
			 * @param rc レンダリングコンテキスト
			 */
			void Render(RenderContext& rc) override;

			/**
			 * @brief 波のスクロール速度を設定
			 */
			inline void SetWaveSpeed(float speed) { m_waveSpeed = speed; }

			/**
			 * @brief 波①の振幅を設定
			 */
			inline void SetWave1Amplitude(float amplitude) { m_constantBuffer.wave1Amplitude = amplitude; }

			/**
			 * @brief 波①の空間周波数を設定
			 */
			inline void SetWave1Frequency(float frequency) { m_constantBuffer.wave1Frequency = frequency; }

			/**
			 * @brief 波②の振幅を設定
			 */
			inline void SetWave2Amplitude(float amplitude) { m_constantBuffer.wave2Amplitude = amplitude; }

			/**
			 * @brief 波②の空間周波数を設定
			 */
			inline void SetWave2Frequency(float frequency) { m_constantBuffer.wave2Frequency = frequency; }

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
			OceanMesh           m_oceanMesh;					/** 海のグリッドメッシュ */
			OceanConstantBuffer m_constantBuffer;				/** 定数バッファ */
			Vector3             m_position = g_vec3Zero;		/** 位置 */
			Vector3             m_scale = g_vec3One * 5.0f;		/** スケール */
			Quaternion          m_rotation = Quaternion::Identity;	/** 回転 */
			float               m_waveSpeed = 1.5f;			/** 波のスクロール速度 */
			float               m_textureSpeed = 0.03f;			/** テクスチャのスクロール速度 */


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