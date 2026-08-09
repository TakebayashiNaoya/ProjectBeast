/**
 * @file ShadowMap.h
 * @brief カスケードシャドウマップクラス
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	/** 前方宣言 */
	class ModelRender;


	/**
	 * @brief カスケードの数
	 * @details Shadow.h の NUM_SHADOW_CASCADES と必ず一致させること。
	 *          増やすと遠景の精細さは上がるが、シーンを影用にこの回数だけ描くことになる。
	 */
	static const int NUM_SHADOW_CASCADES = 3;


	/**
	 * @brief カスケードシャドウマップ
	 * @details ディレクションライトから見た深度を書き込み、
	 *          ディファードライティングや海・渦潮で参照させるためのクラス。
	 * @details 1枚のシャドウマップでは「遠くまで覆う」と「影が精細」が両立しない。
	 *          テクセルの総数が決まっているため、広く配れば必ず粗くなる。
	 *          そこでカメラの視錐台を距離で分割し、区間ごとに専用のシャドウマップを持つ。
	 *          近景は狭い範囲を1枚で覆うので精細になり、遠景は粗くても画面上で小さいため目立たない。
	 * @details 各カスケードは、担当区間の視錐台に外接する球へ直交投影を合わせる。
	 *          球で囲むのはカメラが回転しても大きさを変えないため。
	 *          AABBで囲むと回転のたびに範囲が伸縮し、影の輪郭がちらつく。
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
		 * @details カスケードごとにライト行列を更新し、キャスターを深度のみで描画する。
		 * @param rc               レンダリングコンテキスト
		 * @param lightDirection   ディレクションライトの向き（正規化済み）
		 * @param camera           影を出す範囲を決めるカメラ（メインカメラ）
		 * @param deferredModels   ディファード描画のモデルリスト
		 * @param forwardModels    フォワード描画のモデルリスト
		 */
		void Render(
			RenderContext& rc,
			const Vector3& lightDirection,
			nsK2EngineLow::Camera& camera,
			const std::vector<ModelRender*>& deferredModels,
			const std::vector<ModelRender*>& forwardModels
		);

		/**
		 * @brief 指定したカスケードのシャドウマップのテクスチャを取得する
		 * @param cascadeIndex カスケードの番号（0が最も近景）
		 * @return シャドウマップのテクスチャの参照
		 */
		Texture& GetShadowMapTexture(const int cascadeIndex)
		{
			return m_shadowMapRenderTargets[cascadeIndex].GetRenderTargetTexture();
		}

		/**
		 * @brief 指定したカスケードのライトビュープロジェクション行列を取得する
		 * @param cascadeIndex カスケードの番号（0が最も近景）
		 * @return ライトビュープロジェクション行列の参照
		 */
		const Matrix& GetLVPMatrix(const int cascadeIndex) const { return m_lvpMatrices[cascadeIndex]; }

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

		/**
		 * @brief 影の中で直接光を何割残すかを設定
		 * @details 0.0で直接光を完全に遮る。上げると影が薄くなる。
		 * @param rate 残す割合（0.0〜1.0）
		 */
		void SetDirectLightRate(const float rate) { m_directLightRate = rate; }

		/**
		 * @brief 影の中で直接光を何割残すかを取得
		 * @return 残す割合
		 */
		float GetDirectLightRate() const { return m_directLightRate; }

		/**
		 * @brief 影の中で環境光を何割残すかを設定
		 * @details 1.0だと環境光がそのまま乗るため、環境光が強いシーンでは
		 *          影がほとんど見えなくなる。下げると影が濃くなる。
		 *          影の見え方に最も効くパラメータ。
		 * @param rate 残す割合（0.0〜1.0）
		 */
		void SetAmbientRate(const float rate) { m_ambientRate = rate; }

		/**
		 * @brief 影の中で環境光を何割残すかを取得
		 * @return 残す割合
		 */
		float GetAmbientRate() const { return m_ambientRate; }

		/**
		 * @brief 影を出す距離を設定
		 * @details カメラからこの距離までが影の対象になる。
		 *          最も遠いカスケードの終端になる。
		 * @param distance 影を出す距離
		 */
		void SetShadowDistance(const float distance) { m_shadowDistance = distance; }

		/**
		 * @brief 影を出す距離を取得
		 * @return 影を出す距離
		 */
		float GetShadowDistance() const { return m_shadowDistance; }

		/**
		 * @brief 指定したカスケードの1テクセルあたりのワールド空間の大きさを取得
		 * @details 影の粗さの目安。デバッグ表示に使う。
		 * @param cascadeIndex カスケードの番号
		 * @return 1テクセルあたりのワールド単位
		 */
		float GetTexelWorldSize(const int cascadeIndex) const { return m_texelWorldSizes[cascadeIndex]; }

		/**
		 * @brief 指定したカスケードの担当区間の終端距離を取得
		 * @param cascadeIndex カスケードの番号
		 * @return カメラからの距離
		 */
		float GetCascadeFarDistance(const int cascadeIndex) const { return m_cascadeFarDistances[cascadeIndex]; }


	private:
		/**
		 * @brief カスケードの担当区間を計算する
		 * @details near から m_shadowDistance までを対数的に分割する。
		 *          等分すると近景の区間が広くなりすぎてキャラクターの影が粗くなるため、
		 *          手前を細かく、奥を粗く配る。
		 */
		void UpdateCascadeDistances(nsK2EngineLow::Camera& camera);

		/**
		 * @brief 視錐台の一区間に外接する球を求める
		 * @param camera    対象のカメラ
		 * @param nearZ     区間の手前の距離
		 * @param farZ      区間の奥の距離
		 * @param outCenter 球の中心の出力先
		 * @param outRadius 球の半径の出力先
		 */
		void CalcFrustumSphere(
			nsK2EngineLow::Camera& camera,
			float nearZ,
			float farZ,
			Vector3& outCenter,
			float& outRadius
		) const;

		/**
		 * @brief 指定したカスケードのライト行列を更新する
		 * @param cascadeIndex   カスケードの番号
		 * @param lightDirection ディレクションライトの向き（正規化済み）
		 * @param camera         影を出す範囲を決めるカメラ
		 */
		void UpdateLightMatrix(
			const int cascadeIndex,
			const Vector3& lightDirection,
			nsK2EngineLow::Camera& camera
		);

		/**
		 * @brief 指定したカスケードへキャスターを描画する
		 * @param rc             レンダリングコンテキスト
		 * @param cascadeIndex   カスケードの番号
		 * @param deferredModels ディファード描画のモデルリスト
		 * @param forwardModels  フォワード描画のモデルリスト
		 */
		void RenderCascade(
			RenderContext& rc,
			const int cascadeIndex,
			const std::vector<ModelRender*>& deferredModels,
			const std::vector<ModelRender*>& forwardModels
		);

		/**
		 * @brief キャスターがこのカスケードの範囲に関係するかを判定する
		 * @details 影を落としうるかの判定なので、ライトの方向へ範囲を伸ばして判定する。
		 *          範囲の外にあっても、ライト側にあるものは中へ影を落としうるため。
		 * @param cascadeIndex カスケードの番号
		 * @param aabbMin      キャスターのワールドAABBの最小値
		 * @param aabbMax      キャスターのワールドAABBの最大値
		 * @return 描画する必要があるならtrue
		 */
		bool IsCasterVisibleInCascade(
			const int cascadeIndex,
			const Vector3& aabbMin,
			const Vector3& aabbMax
		) const;


	private:
		/** カスケードごとのシャドウマップ */
		std::array<RenderTarget, NUM_SHADOW_CASCADES> m_shadowMapRenderTargets;
		/** カスケードごとのライトのビュー行列 */
		std::array<Matrix, NUM_SHADOW_CASCADES> m_lightViewMatrices;
		/** カスケードごとのライトのプロジェクション行列 */
		std::array<Matrix, NUM_SHADOW_CASCADES> m_lightProjMatrices;
		/** カスケードごとのライトビュープロジェクション行列 */
		std::array<Matrix, NUM_SHADOW_CASCADES> m_lvpMatrices;

		/** カスケードごとの担当区間の終端距離（カメラからの距離） */
		std::array<float, NUM_SHADOW_CASCADES> m_cascadeFarDistances = {};
		/** カスケードごとの覆う範囲の中心（キャスターのカリングに使う） */
		// Vector3 の既定コンストラクタは explicit のため、
		// "= {}" によるコピーリスト初期化は使えない（C2512）。初期化子なしで直接初期化させる。
		std::array<Vector3, NUM_SHADOW_CASCADES> m_cascadeCenters;
		/** カスケードごとの覆う範囲の半径（キャスターのカリングに使う） */
		std::array<float, NUM_SHADOW_CASCADES> m_cascadeRadii = {};
		/** カスケードごとの1テクセルあたりのワールド空間の大きさ（デバッグ表示用） */
		std::array<float, NUM_SHADOW_CASCADES> m_texelWorldSizes = {};

		/** ライトの向き（キャスターのカリングに使う） */
		Vector3 m_lightDirection = Vector3::Down;

		/** シャドウマップが有効かどうか */
		bool m_isEnable = true;

		/** 影の中で直接光を何割残すか */
		float m_directLightRate = 0.0f;
		/** 影の中で環境光を何割残すか */
		float m_ambientRate = 0.5f;
		/** 影を出す距離 */
		float m_shadowDistance = 12000.0f;
	};

} // namespace nsBeastEngine
