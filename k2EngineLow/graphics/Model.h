#pragma once

#include "MeshParts.h"
#include "Skeleton.h"

namespace nsK2EngineLow {

	/// <summary>
	/// モデルの上方向。
	/// </summary>
	enum EnModelUpAxis {
		enModelUpAxisY,		//Y軸が上方向。
		enModelUpAxisZ,		//Z軸が上方向。
	};

	/// <summary>
	/// モデルの初期化データ。
	/// </summary>
	struct ModelInitData {
		const char* m_tkmFilePath = nullptr;									//tkmファイルのファイルパス。
		const char* m_fxFilePath = nullptr;										//fxファイルのファイルパス。
		const char* m_vsEntryPointFunc = "VSMain";								//頂点シェーダーのエントリーポイントの関数名。
		const char* m_vsSkinEntryPointFunc = "VSMain";							//スキンありマテリアル用の頂点シェーダーのエントリーポイントの関数名。
		const char* m_psEntryPointFunc = "PSMain";								//ピクセルシェーダーのエントリーポイントの関数名。
		void* m_expandConstantBuffer = nullptr;									//ユーザー拡張の定数バッファ（b1）。
		int m_expandConstantBufferSize = 0;										//ユーザー拡張の定数バッファのサイズ（b1）。
		void* m_expandConstantBuffer2 = nullptr;								//ユーザー拡張の定数バッファ（b2）。
		int m_expandConstantBufferSize2 = 0;									//ユーザー拡張の定数バッファのサイズ（b2）。
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResoruceView = { nullptr };	//ユーザー拡張のシェーダーリソースビュー。
		EnModelUpAxis m_modelUpAxis = enModelUpAxisY;							//モデルの上方向。
		std::array<DXGI_FORMAT, MAX_RENDERING_TARGET> m_colorBufferFormat = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
		};	//レンダリングされるカラーバッファのフォーマット。
		AlphaBlendMode m_alphaBlendMode = AlphaBlendMode_None;					//アルファブレンドモード。
		Skeleton* m_skeleton = nullptr;											//スケルトン。
		bool m_isDepthWrite = true;												//深度値を書き込むか。
		bool m_isDepthTest = true;												//深度テストを行うか。
		D3D12_CULL_MODE m_cullMode = D3D12_CULL_MODE_BACK;						//カリングモード。
	};
	/// <summary>
	/// マテリアルを再初期化するためのデータ。
	/// </summary>
	struct MaterialReInitData {
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResoruceView = { nullptr };
	};
	/// <summary>
	/// モデルクラス。
	/// </summary>
	class Model : public Noncopyable {

	public:

		/// <summary>
		/// tkmファイルから初期化。
		/// </summary>
		/// <param name="initData">初期化データ</param>
		void Init(const ModelInitData& initData);
		/// <summary>
		/// ワールド行列の更新。
		/// </summary>
		/// <param name="pos">座標</param>
		/// <param name="rot">回転</param>
		/// <param name="scale">拡大率</param>
		void UpdateWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale)
		{
			m_worldMatrix = CalcWorldMatrix(pos, rot, scale);
		}

		/// <summary>
		/// ワールド行列を計算
		/// </summary>
		/// <remark>
		/// ModelクラスのデータとのWORLD行列の計算を行います。
		/// 計算されたワールド行列が戻り値として返されます。
		/// 本関数はUpdateWorldMatrixから呼ばれています。
		/// 本関数はワールド行列を計算して、返すだけです。
		/// Model::m_worldMatrixを更新するわけではないので、ご注意ください。
		/// 本クラスのデータをもとに計算されたワールド行列が必要な場合に使用してください
		/// </remark>
		/// <param name="pos">座標</param>
		/// <param name="rot">回転</param>
		/// <param name="scale">拡大率</param>
		Matrix CalcWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale);

		/// <summary>
		/// ワールド行列の取得。
		/// </summary>
		const Matrix& GetWorldMatrix() const
		{
			return m_worldMatrix;
		}
		/// <summary>
		/// メッシュに対して問い合わせを行う。
		/// </summary>
		void QueryMeshs(std::function<void(const SMesh& mesh)> queryFunc)
		{
			m_meshParts.QueryMeshs(queryFunc);
		}
		void QueryMeshAndDescriptorHeap(std::function<void(const SMesh& mesh, const DescriptorHeap& ds)> queryFunc)
		{
			m_meshParts.QueryMeshAndDescriptorHeap(queryFunc);
		}
		/// <summary>
		/// アルベドマップを変更。
		/// </summary>
		/// <remarks>
		/// この関数を呼び出すとディスクリプタヒープの再構築が行われるため、重い処理です。
		/// 毎フレーム呼び出す必要がない場合は呼び出さないようにしてください。
		/// </remarks>
		/// <param name="materialName">変更したいマテリアルの名前</param>
		/// <param name="albedoMap">アルベドマップ</param>
		void ChangeAlbedoMap(const char* materialName, Texture& albedoMap);
		/// <summary>
		/// モデルの全マテリアルに乗算カラーを設定する。
		/// </summary>
		/// <param name="mulColor">乗算カラー</param>
		void SetMulColor(const Vector4& mulColor)
		{
			m_meshParts.SetMulColor(mulColor);
		}
		/// <summary>
		/// TKMファイルを取得。
		/// </summary>
		/// <returns></returns>
		const TkmFile& GetTkmFile() const
		{
			return *m_tkmFile;
		}
		/// <summary>
		/// 初期化されているか調べる。
		/// </summary>
		/// <returns></returns>
		bool IsInited() const
		{
			return m_isInited;
		}
		/// <summary>
		/// マテリアルを再初期化。
		/// </summary>
		/// <remark>
		/// モデルに貼り付けるテクスチャを変更したい場合などに利用してください。
		/// </remark>
		/// <param name="reInitData">再初期化データ。</param>
		void ReInitMaterials(MaterialReInitData& reInitData);

		/// <summary>
		/// 描画。
		/// </summary>
		/// <param name="rc">レンダリングコンテキスト</param>
		/// <param name="numInstance">インスタンスの数</param>
		void Draw(
			RenderContext& rc,
			int numInstance = 1
		);
		void Draw(
			RenderContext& rc,
			Camera& camera,
			int numInstance = 1
		);
		void Draw(
			RenderContext& rc,
			const Matrix& viewMatrix,
			const Matrix& projMatrix,
			int numInstance = 1
		);


	private:
		bool m_isInited = false;						//初期化されているか？
		Matrix m_worldMatrix;							//ワールド行列。
		TkmFile* m_tkmFile;								//tkmファイル。
		Skeleton m_skeleton;							//スケルトン。
		MeshParts m_meshParts;							//メッシュパーツ。
		EnModelUpAxis m_modelUpAxis = enModelUpAxisY;	//モデルの上方向。
	};
}