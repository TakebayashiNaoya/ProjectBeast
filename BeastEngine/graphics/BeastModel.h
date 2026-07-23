/**
 * @file BeastModel.h
 * @brief トライアングルカリング対応モデルクラス
 * @author 竹林
 */
#pragma once
#include "Graphics/BeastMeshParts.h"


namespace nsBeastEngine
{
	// Frustum はポインタ・参照でのみ使用するため前方宣言で十分
	class Frustum;
	/**
	 * @brief トライアングルカリング対応モデルクラス
	 * @details
	 *   k2EngineLow::Model のコードをベースに BeastEngine へ移植し、
	 *   BeastMeshParts を使用することでトライアングルカリングに対応したクラス。
	 *   ModelRender の描画用モデル（GBuffer・フォワード）として使用する。
	 *   k2EngineLow は一切変更しない。
	 */
	class BeastModel : public Noncopyable
	{
	public:
		/**
		 * @brief tkmファイルから初期化する
		 * @param initData 初期化データ
		 */
		void Init(const ModelInitData& initData);

		/**
		 * @brief ワールド行列を更新する
		 * @param pos	座標
		 * @param rot	回転
		 * @param scale	拡大率
		 */
		void UpdateWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale)
		{
			m_worldMatrix = CalcWorldMatrix(pos, rot, scale);
		}

		/**
		 * @brief ワールド行列を計算して返す
		 * @details
		 *   m_worldMatrix を更新するわけではない。
		 *   本クラスのデータをもとに計算されたワールド行列が必要な場合に使用する。
		 * @param pos	座標
		 * @param rot	回転
		 * @param scale	拡大率
		 * @return 計算済みワールド行列
		 */
		Matrix CalcWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale);

		/**
		 * @brief ワールド行列を取得する
		 * @return ワールド行列
		 */
		const Matrix& GetWorldMatrix() const
		{
			return m_worldMatrix;
		}

		/**
		 * @brief メッシュに対して問い合わせを行う
		 * @param queryFunc 問い合わせ関数
		 */
		void QueryMeshs(std::function<void(const SBeastMesh& mesh)> queryFunc)
		{
			m_meshParts.QueryMeshs(queryFunc);
		}

		/**
		 * @brief メッシュとディスクリプタヒープに対して問い合わせを行う
		 * @param queryFunc 問い合わせ関数
		 */
		void QueryMeshAndDescriptorHeap(
			std::function<void(const SBeastMesh& mesh, const DescriptorHeap& ds)> queryFunc
		)
		{
			m_meshParts.QueryMeshAndDescriptorHeap(queryFunc);
		}

		/**
		 * @brief アルベドマップを変更する
		 * @details ディスクリプタヒープの再構築が伴うため毎フレーム呼ばないこと
		 * @param materialName	変更対象のマテリアル名
		 * @param albedoMap		新しいアルベドマップ
		 */
		void ChangeAlbedoMap(const char* materialName, Texture& albedoMap);

		/**
		 * @brief モデルの乗算カラーを設定する
		 * @param mulColor 乗算カラー
		 */
		void SetMulColor(const Vector4& mulColor)
		{
			m_meshParts.SetMulColor(mulColor);
		}

		/**
		 * @brief モデルの透明度(乗算カラーのα成分)を設定する
		 * @param alpha 透明度(1.0f=不透明, 0.0f=完全透明)
		 */
		void SetAlpha(const float alpha)
		{
			m_meshParts.SetAlpha(alpha);
		}

		/**
		 * @brief 拡張定数バッファ（b2）のデータポインタをInit後に差し替える
		 * @param data 新しいデータポインタ
		 */
		void SetExpandData2(void* data)
		{
			m_meshParts.SetExpandData2(data);
		}

		/**
		 * @brief 拡張定数バッファ（b3）のデータポインタをInit後に差し替える
		 * @param data 新しいデータポインタ
		 */
		void SetExpandData3(void* data)
		{
			m_meshParts.SetExpandData3(data);
		}

		/**
		 * @brief 拡張定数バッファ（b4）のデータポインタをInit後に差し替える
		 * @param data 新しいデータポインタ
		 */
		void SetExpandData4(void* data)
		{
			m_meshParts.SetExpandData4(data);
		}

		/**
		 * @brief マテリアルを再初期化する
		 * @param reInitData 再初期化データ
		 */
		void ReInitMaterials(MaterialReInitData& reInitData);

		/**
		 * @brief 初期化されているか調べる
		 * @return 初期化済みならtrue
		 */
		bool IsInited() const
		{
			return m_isInited;
		}

		/**
		 * @brief 描画する（カリングなし）
		 * @param rc			レンダリングコンテキスト
		 * @param numInstance	インスタンス数
		 */
		void Draw(nsK2EngineLow::RenderContext& rc, int numInstance = 1);

		/**
		 * @brief 指定カメラで描画する（カリングなし）
		 * @param rc			レンダリングコンテキスト
		 * @param camera		使用するカメラ
		 * @param numInstance	インスタンス数
		 */
		void Draw(nsK2EngineLow::RenderContext& rc, Camera& camera, int numInstance = 1);

		/**
		 * @brief 指定ビュー・プロジェクション行列で描画する（カリングなし）
		 * @param rc			レンダリングコンテキスト
		 * @param viewMatrix	ビュー行列
		 * @param projMatrix	プロジェクション行列
		 * @param numInstance	インスタンス数
		 */
		void Draw(
			nsK2EngineLow::RenderContext& rc,
			const Matrix& viewMatrix,
			const Matrix& projMatrix,
			int numInstance = 1
		);

		/**
		 * @brief トライアングルカリングを行いながら描画する
		 * @param rc			レンダリングコンテキスト
		 * @param frustum		カリングに使用する視錐台
		 * @param numInstance	インスタンス数
		 */
		void Draw(nsK2EngineLow::RenderContext& rc, const Frustum& frustum, int numInstance = 1);

		/**
		 * @brief 指定カメラとフラスタムでトライアングルカリングを行いながら描画する
		 * @param rc			レンダリングコンテキスト
		 * @param camera		使用するカメラ
		 * @param frustum		カリングに使用する視錐台
		 * @param numInstance	インスタンス数
		 */
		void Draw(
			nsK2EngineLow::RenderContext& rc,
			Camera& camera,
			const Frustum& frustum,
			int numInstance = 1
		);


	private:
		bool             m_isInited = false;					/** 初期化済みフラグ */
		Matrix           m_worldMatrix;							/** ワールド行列 */
		TkmFile* m_tkmFile = nullptr;				/** tkmファイル */
		Skeleton         m_skeleton;							/** スケルトン */
		BeastMeshParts   m_meshParts;							/** メッシュパーツ */
		EnModelUpAxis    m_modelUpAxis = enModelUpAxisZ;		/** モデル上方向 */
	};
}