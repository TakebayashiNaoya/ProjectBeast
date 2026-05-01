/// <summary>
/// メッシュパーツクラス。
/// </summary>

#pragma once

#include "tkFile/TkmFile.h"
#include "StructuredBuffer.h"
#include "Material.h"

namespace nsK2EngineLow {

	class RenderContext;
	class Skeleton;
	class Material;
	class IShaderResource;
	struct MaterialReInitData;


	const int MAX_MODEL_EXPAND_SRV = 32;	//拡張SRVの最大数。

	/// <summary>
	/// メッシュ
	/// </summary>
	struct SMesh {
		VertexBuffer m_vertexBuffer;						//頂点バッファ。
		std::vector< IndexBuffer* >		m_indexBufferArray;	//インデックスバッファ。
		std::vector< Material* >		m_materials;			//マテリアル。
		std::vector<int>				skinFlags;				//スキンが設定されているかどうかのフラグ。
	};

	/// <summary>
	/// メッシュパーツ。
	/// </summary>
	class MeshParts : public Noncopyable {
	public:
		/// <summary>
		/// デストラクタ。
		/// </summary>
		~MeshParts();
		/// <summary>
		/// tkmファイルから初期化
		/// </summary>
		/// <param name="tkmFile">tkmファイル。</param>
		/// /// <param name="fxFilePath">fxファイルのファイルパス</param>
		/// <param name="vsEntryPointFunc">頂点シェーダーのエントリーポイントの関数名</param>
		/// <param name="vsSkinEntryPointFunc">スキンありマテリアル用の頂点シェーダーのエントリーポイントの関数名</param>
		/// <param name="psEntryPointFunc">ピクセルシェーダーのエントリーポイントの関数名</param>
		/// <param name="colorBufferFormat">このモデルがレンダリングされるカラーバッファのフォーマット</param>
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
			const std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV>& expandShaderResourceView,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode
		);
		/// <summary>
		/// 描画。
		/// </summary>
		/// <param name="rc">レンダリングコンテキスト</param>
		/// <param name="mWorld">ワールド行列</param>
		/// <param name="mView">ビュー行列</param>
		/// <param name="mProj">プロジェクション行列</param>
		/// <param name="numInstance">インスタンスの数</param>
		void Draw(
			RenderContext& rc,
			const Matrix& mWorld,
			const Matrix& mView,
			const Matrix& mProj,
			int numInstance);
		/// <summary>
		/// スケルトンを関連付ける。
		/// </summary>
		/// <param name="skeleton">スケルトン</param>
		void BindSkeleton(Skeleton& skeleton);
		/// <summary>
		/// メッシュに対して問い合わせを行う。
		/// </summary>
		/// <param name="queryFunc">クエリ関数</param>
		void QueryMeshs(std::function<void(const SMesh& mesh)> queryFunc)
		{
			for (const auto& mesh : m_meshs) {
				queryFunc(*mesh);
			}
		}
		void QueryMeshAndDescriptorHeap(std::function<void(const SMesh& mesh, const DescriptorHeap& ds)> queryFunc)
		{
			for (int i = 0; i < m_meshs.size(); i++) {
				queryFunc(*m_meshs[i], m_descriptorHeap);
			}
		}
		/// <summary>
		/// モデルの乗算カラーを設定する。
		/// </summary>
		/// <param name="mulColor">乗算カラー</param>
		void SetMulColor(const Vector4& mulColor)
		{
			m_mulColor = mulColor;
		}
		/// <summary>
		/// ディスクリプタヒープを作成。
		/// </summary>
		void CreateDescriptorHeaps();
		/// <summary>
		/// マテリアルを再初期化。
		/// </summary>
		void ReInitMaterials(const MaterialReInitData& reInitData);
	private:
		/// <summary>
		/// tkmメッシュからメッシュを作成。
		/// </summary>
		/// <param name="mesh">メッシュ</param>
		/// <param name="meshNo">メッシュ番号</param>
		/// <param name="fxFilePath">fxファイルのファイルパス</param>
		/// <param name="vsEntryPointFunc">頂点シェーダーのエントリーポイントの関数名</param>
		/// <param name="vsSkinEntryPointFunc">スキンありマテリアル用の頂点シェーダーのエントリーポイントの関数名</param>
		/// <param name="psEntryPointFunc">ピクセルシェーダーのエントリーポイントの関数名</param>
		/// <param name="colorBufferFormat">このモデルがレンダリングされるカラーバッファのフォーマット</param>
		void CreateMeshFromTkmMesh(
			const TkmFile::SMesh& mesh,
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
		//拡張SRVが設定されるレジスタの開始番号。
		const int EXPAND_SRV_REG__START_NO = 10;
		//1つのマテリアルで使用されるSRVの数。
		const int NUM_SRV_ONE_MATERIAL = EXPAND_SRV_REG__START_NO + MAX_MODEL_EXPAND_SRV;
		//1つのマテリアルで使用されるCBVの数。
		const int NUM_CBV_ONE_MATERIAL = 3;
		/// <summary>
		/// 定数バッファ。
		/// </summary>
		/// <remarks>
		/// この構造体を変更すると、SimpleModel.fxのCBを変更するように。
		/// </remarks>
		struct SConstantBuffer {
			Matrix mWorld;		//ワールド行列。
			Matrix mView;		//ビュー行列。
			Matrix mProj;		//プロジェクション行列。
			Vector4 mulColor;	//乗算カラー。
		};
		ConstantBuffer m_commonConstantBuffer;					//メッシュ共通の定数バッファ。
		ConstantBuffer m_expandConstantBuffer;					//ユーザー用の定数バッファ（b1）
		ConstantBuffer m_expandConstantBuffer2;					//ユーザー用の定数バッファ（b2）
		Vector4 m_mulColor = Vector4::One;						//モデルの乗算カラー(b1にセット)。
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResourceView = { nullptr };	//ユーザーシェーダーリソースビュー。
		StructuredBuffer m_boneMatricesStructureBuffer;	//ボーン行列の構造体バッファ。
		std::vector< SMesh* > m_meshs;						//メッシュ。
		//std::vector< DescriptorHeap > m_descriptorHeap;	//ディスクリプタヒープ。
		DescriptorHeap m_descriptorHeap;					//ディスクリプタヒープ。
		Skeleton* m_skeleton = nullptr;						//スケルトン。
		void* m_expandData = nullptr;						//ユーザーデータ（b1）。
		void* m_expandData2 = nullptr;						//ユーザーデータ（b2）。
	};
}