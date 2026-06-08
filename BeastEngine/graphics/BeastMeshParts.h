/**
 * @file BeastMeshParts.h
 * @brief トライアングルカリング対応メッシュパーツクラス
 * @author 竹林
 */
#pragma once


namespace nsBeastEngine
{
	// Frustum はポインタでのみ使用するため前方宣言で十分
	class Frustum;

	/**
	 * @brief トライアングルカリング対応メッシュ構造体
	 * @details
	 *   k2EngineLow::SMesh をベースに、トライアングルカリング用の
	 *   CPUキャッシュを追加した構造体。
	 *   スキンなしメッシュのみカリングを適用する。
	 */
	struct SBeastMesh
	{
		VertexBuffer                        m_vertexBuffer;		/** 頂点バッファ */
		std::vector<IndexBuffer*>           m_indexBufferArray;	/** 元インデックスバッファ配列（読み取り専用） */
		std::vector<Material*>              m_materials;		/** マテリアル配列 */
		std::vector<int>                    skinFlags;			/** スキンフラグ（1=スキンあり） */

		// トライアングルカリング用CPUキャッシュ
		// スキンなしメッシュのみ使用する（スキンありは毎フレームボーン変形が走るため非対応）
		std::vector<Vector3>                m_localVertexPositions;	/** ローカル座標の頂点位置 */
		std::vector<std::vector<uint32_t>>  m_srcIndexArrays;		/** マテリアルごとの元インデックス */
		std::vector<std::vector<uint32_t>>  m_visibleIndices;		/** マテリアルごとの可視インデックス（毎フレーム更新） */

		// トライアングルカリング書き込み先のインデックスバッファ（ダブルバッファ）
		// 定数バッファと同様に、メインビュー（frameIdx=0）とサブビュー（frameIdx=1）で
		// 別スロットに書き込むことで同フレーム内の上書き競合を防ぐ
		std::vector<std::array<IndexBuffer*, 2>> m_visibleIndexBuffers;
	};


	/**
	 * @brief トライアングルカリング対応メッシュパーツクラス
	 * @details
	 *   k2EngineLow::MeshParts のコードをベースに BeastEngine へ移植し、
	 *   Draw() にトライアングルカリングを組み込んだクラス。
	 *   ModelRender の描画用モデル（GBuffer・フォワード）に使用する。
	 *   k2EngineLow は一切変更しない。
	 */
	class BeastMeshParts : public Noncopyable
	{
	public:
		/**
		 * @brief デストラクタ
		 */
		~BeastMeshParts();

		/**
		 * @brief tkmファイルから初期化する
		 * @param tkmFile					tkmファイル
		 * @param fxFilePath				FXファイルパス
		 * @param vsEntryPointFunc			頂点シェーダーエントリーポイント
		 * @param vsSkinEntryPointFunc		スキンあり頂点シェーダーエントリーポイント
		 * @param psEntryPointFunc			ピクセルシェーダーエントリーポイント
		 * @param expandData				拡張定数バッファ（b1）データ
		 * @param expandDataSize			拡張定数バッファ（b1）サイズ
		 * @param expandData2				拡張定数バッファ（b2）データ
		 * @param expandDataSize2			拡張定数バッファ（b2）サイズ
		 * @param expandData3				拡張定数バッファ（b3）データ
		 * @param expandDataSize3			拡張定数バッファ（b3）サイズ
		 * @param expandData4				拡張定数バッファ（b4）データ
		 * @param expandDataSize4			拡張定数バッファ（b4）サイズ
		 * @param expandShaderResourceView	拡張シェーダーリソースビュー配列
		 * @param colorBufferFormat			カラーバッファフォーマット
		 * @param alphaBlendMode			アルファブレンドモード
		 * @param isDepthWrite				深度書き込みフラグ
		 * @param isDepthTest				深度テストフラグ
		 * @param cullMode					カリングモード
		 */
		void InitFromTkmFile(
			const TkmFile& tkmFile,
			const char* fxFilePath,
			const char* vsEntryPointFunc,
			const char* vsSkinEntryPointFunc,
			const char* psEntryPointFunc,
			void* expandData,
			int expandDataSize,
			void* expandData2,
			int expandDataSize2,
			void* expandData3,
			int expandDataSize3,
			void* expandData4,
			int expandDataSize4,
			const std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV>& expandShaderResourceView,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode
		);

		/**
		 * @brief 描画する
		 * @details
		 *   frustum が渡された場合、スキンなしメッシュに対してトライアングルカリングを行う。
		 *   スキンありメッシュはボーン変形後の頂点座標がCPUにないためカリングをスキップする。
		 * @param rc			レンダリングコンテキスト
		 * @param mWorld		ワールド行列
		 * @param mView			ビュー行列
		 * @param mProj			プロジェクション行列
		 * @param numInstance	インスタンス数
		 * @param frustum		トライアングルカリングに使用する視錐台（nullptrでカリングなし）
		 */
		void Draw(
			nsK2EngineLow::RenderContext& rc,
			const Matrix& mWorld,
			const Matrix& mView,
			const Matrix& mProj,
			int numInstance,
			const Frustum* frustum = nullptr
		);

		/**
		 * @brief スケルトンを関連付ける
		 * @param skeleton スケルトン
		 */
		void BindSkeleton(Skeleton& skeleton);

		/**
		 * @brief メッシュに対して問い合わせを行う
		 * @param queryFunc 問い合わせ関数
		 */
		void QueryMeshs(std::function<void(const SBeastMesh& mesh)> queryFunc)
		{
			for (const auto& mesh : m_meshs)
			{
				queryFunc(*mesh);
			}
		}

		/**
		 * @brief メッシュとディスクリプタヒープに対して問い合わせを行う
		 * @param queryFunc 問い合わせ関数
		 */
		void QueryMeshAndDescriptorHeap(
			std::function<void(const SBeastMesh& mesh, const DescriptorHeap& ds)> queryFunc
		)
		{
			for (const auto& mesh : m_meshs)
			{
				queryFunc(*mesh, m_descriptorHeap);
			}
		}

		/**
		 * @brief モデルの乗算カラーを設定する
		 * @param mulColor 乗算カラー
		 */
		void SetMulColor(const Vector4& mulColor)
		{
			m_mulColor = mulColor;
		}

		/**
		 * @brief 拡張定数バッファ（b2）のデータポインタを差し替える
		 * @param data 新しいデータポインタ
		 */
		void SetExpandData2(void* data)
		{
			m_expandData2 = data;
		}

		/**
		 * @brief 拡張定数バッファ（b3）のデータポインタを差し替える
		 * @param data 新しいデータポインタ
		 */
		void SetExpandData3(void* data)
		{
			m_expandData3 = data;
		}

		/**
		 * @brief 拡張定数バッファ（b4）のデータポインタを差し替える
		 * @param data 新しいデータポインタ
		 */
		void SetExpandData4(void* data)
		{
			m_expandData4 = data;
		}

		/**
		 * @brief ディスクリプタヒープを再作成する
		 * @details アルベドマップ差し替え後などに呼ぶこと
		 */
		void CreateDescriptorHeaps();

		/**
		 * @brief マテリアルを再初期化する
		 * @param reInitData 再初期化データ
		 */
		void ReInitMaterials(const MaterialReInitData& reInitData);


	private:
		/**
		 * @brief tkmメッシュから SBeastMesh を構築する
		 */
		void CreateMeshFromTkmMesh(
			const TkmFile::SMesh& tkmMesh,
			int meshNo,
			int& materialNum,
			const char* fxFilePath,
			const char* vsEntryPointFunc,
			const char* vsSkinEntryPointFunc,
			const char* psEntryPointFunc,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode
		);


	private:
		/** 拡張SRVが設定されるレジスタの開始番号 */
		const int EXPAND_SRV_REG__START_NO = 10;
		/** 1マテリアルで使用するSRV数 */
		const int NUM_SRV_ONE_MATERIAL = EXPAND_SRV_REG__START_NO + MAX_MODEL_EXPAND_SRV;
		/** 1マテリアルで使用するCBV数（b0〜b4の5つ） */
		const int NUM_CBV_ONE_MATERIAL = 5;

		/**
		 * @brief メッシュ共通定数バッファ（b0）
		 */
		struct SConstantBuffer
		{
			Matrix  mWorld;		/** ワールド行列 */
			Matrix  mView;		/** ビュー行列 */
			Matrix  mProj;		/** プロジェクション行列 */
			Vector4 mulColor;	/** 乗算カラー */
		};

		ConstantBuffer  m_commonConstantBuffer;		/** メッシュ共通定数バッファ（b0） */
		ConstantBuffer  m_expandConstantBuffer;		/** 拡張定数バッファ（b1） */
		ConstantBuffer  m_expandConstantBuffer2;	/** 拡張定数バッファ（b2） */
		ConstantBuffer  m_expandConstantBuffer3;	/** 拡張定数バッファ（b3） */
		ConstantBuffer  m_expandConstantBuffer4;	/** 拡張定数バッファ（b4） */
		Vector4         m_mulColor = Vector4::One;	/** 乗算カラー */

		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResourceView = { nullptr };
		/** ボーン行列の構造化バッファ */
		StructuredBuffer m_boneMatricesStructureBuffer;

		std::vector<SBeastMesh*> m_meshs;			/** メッシュ配列 */
		DescriptorHeap           m_descriptorHeap;	/** ディスクリプタヒープ */
		Skeleton* m_skeleton = nullptr;	/** スケルトン参照 */

		void* m_expandData = nullptr;	/** 拡張定数バッファ（b1）データポインタ */
		void* m_expandData2 = nullptr;	/** 拡張定数バッファ（b2）データポインタ */
		void* m_expandData3 = nullptr;	/** 拡張定数バッファ（b3）データポインタ */
		void* m_expandData4 = nullptr;	/** 拡張定数バッファ（b4）データポインタ */

		/** トライアングルカリング用ワールド頂点バッファ（Draw()内で毎フレーム更新） */
		std::vector<Vector3> m_worldVertexCache;
	};
}