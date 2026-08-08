/**
 * @file ShadowMap.h
 * @brief シャドウマップクラス
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	/** 前方宣言 */
	class ModelRender;


	/**
	 * @brief シャドウマップ
	 * @details ディレクションライトから見た深度を書き込み、
	 *          ディファードライティングで参照させるためのクラス。
	 * @details world全体を1枚で覆うと1テクセルあたりの範囲が広くなりすぎるため、
	 *          注視点まわりの固定範囲だけを直交投影で覆う方式にしている。
	 *          範囲外には影が出ないので、注視点はカメラの見ている位置に追従させること。
	 * @details RenderingEngine が保持し、Execute() の描画パスより前に Render() を呼ぶ。
	 */
	class ShadowMap
	{
	public:
		ShadowMap() = default;
		~ShadowMap() = default;


	public:
		/**
		 * @brief 初期化
		 */
		void Init();

		/**
		 * @brief シャドウマップへの描画を実行する
		 * @details ライト行列を更新したうえで、キャスターを深度のみで描画する。
		 * @param rc               レンダリングコンテキスト
		 * @param lightDirection   ディレクションライトの向き（正規化済み）
		 * @param focusPosition    シャドウマップで覆う範囲の中心（カメラの注視点）
		 * @param deferredModels   ディファード描画のモデルリスト
		 * @param forwardModels    フォワード描画のモデルリスト
		 */
		void Render(
			RenderContext& rc,
			const Vector3& lightDirection,
			const Vector3& focusPosition,
			const std::vector<ModelRender*>& deferredModels,
			const std::vector<ModelRender*>& forwardModels
		);

		/**
		 * @brief シャドウマップのテクスチャを取得する
		 * @return シャドウマップのテクスチャの参照
		 */
		Texture& GetShadowMapTexture() { return m_shadowMapRenderTarget.GetRenderTargetTexture(); }

		/**
		 * @brief ライトビュープロジェクション行列を取得する
		 * @return ライトビュープロジェクション行列の参照
		 */
		const Matrix& GetLVPMatrix() const { return m_lvpMatrix; }

		/**
		 * @brief シャドウマップが有効かどうかを設定
		 * @param isEnable 有効かどうか
		 */
		void SetEnable(const bool isEnable) { m_isEnable = isEnable; }

		/**
		 * @brief シャドウマップが有効かどうかを取得
		 * @return 有効かどうか
		 */
		bool IsEnable() const { return m_isEnable; }


	private:
		/**
		 * @brief ライトのビュー行列とプロジェクション行列を更新する
		 * @param lightDirection ディレクションライトの向き（正規化済み）
		 * @param focusPosition  シャドウマップで覆う範囲の中心
		 */
		void UpdateLightMatrix(const Vector3& lightDirection, const Vector3& focusPosition);


	private:
		/** シャドウマップの書き込み先 */
		RenderTarget m_shadowMapRenderTarget;

		/** ライトのビュー行列 */
		Matrix m_lightViewMatrix = Matrix::Identity;
		/** ライトのプロジェクション行列 */
		Matrix m_lightProjMatrix = Matrix::Identity;
		/** ライトビュープロジェクション行列 */
		Matrix m_lvpMatrix = Matrix::Identity;

		/** シャドウマップが有効かどうか */
		bool m_isEnable = true;
	};

} // namespace nsBeastEngine
