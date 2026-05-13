/**
 * @file ModelRender.h
 * @brief モデルレンダー
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief PBR補正パラメータ
	 * @details モデルごとに個別設定できるPBRライティングの補正値。
	 *          RenderToGBuffer.fx の PBRParamCb (b2) に渡される。
	 */
	struct PBRParam
	{
		float m_dirLightScale = 1.0f;  // ディレクションライト強度倍率
		float m_ambientScale = 1.0f;   // 環境光強度倍率
		float m_metallicOffset = 0.0f; // metallicオフセット
		float m_smoothOffset = 0.0f;   // smoothオフセット
	};

	/**
	 * @brief ディザリングCB（b3）のプレースホルダー
	 * @details
	 *   InitRenderToGBufferModel時にb3のConstantBufferを確保するために使用する。
	 *   実際のデータはOcclusionDitherManager::Register時に
	 *   SetExpandConstantBuffer3()でSDitherCbのポインタに差し替えられる。
	 *   SDitherCbと同じサイズにすること。
	 */
	struct SDitherCbPlaceholder
	{
		Vector3 cameraWorldPos = Vector3::Zero;
		float   cylinderRadius = 0.0f;
		Vector3 targetWorldPos = Vector3::Zero;
		float   depthBias = 0.0f;
		float   ditherStrength = 0.0f;
		float   pad[3] = { 0.0f, 0.0f, 0.0f };
	};

	/**
	 * @brief モデル単位ディザリング用の定数バッファ（b4）
	 * @details
	 *   RenderToGBuffer.fx の cbuffer ModelDitherCb : register(b4) に対応する。
	 *   OcclusionDitherManager（b3）によるカメラ遮蔽ディザリングとは独立して動作する。
	 *   SetDitherAlpha()で更新し、次のDraw()でGPUに自動転送される。
	 *   16バイト境界を合わせるためにパディングを含む。
	 */
	struct SModelDitherCb
	{
		/** モデル単位の透過率（0.0f=オフ, 1.0f=完全消去） */
		float modelDitherAlpha = 0.0f;
		/** パディング（16バイト境界用） */
		float pad[3] = { 0.0f, 0.0f, 0.0f };
	};


	/**
	 * @brief モデルレンダー
	 */
	class ModelRender : public IRenderer
	{
	public:
		/**
		 * @brief 位置、回転、大きさの設定
		 * @param pos 位置
		 * @param rot 回転
		 * @param sca 大きさ
		 */
		inline void SetTRS(const Vector3& pos, const Quaternion& rot, const Vector3& sca)
		{
			m_position = pos;
			m_rotation = rot;
			m_scale = sca;
		}

		/**
		 * @brief 位置の設定
		 * @param pos 位置
		 */
		inline void SetPosition(const Vector3& pos) { m_position = pos; }
		/**
		 * @brief 位置の設定
		 * @param x x座標
		 * @param y y座標
		 * @param z z座標
		 */
		inline void SetPosition(const float& x, const float& y, const float& z) { m_position = Vector3(x, y, z); }

		/**
		 * @brief 位置の取得
		 * @return 位置
		 */
		inline const Vector3& GetPosition() const { return m_position; }

		/**
		 * @brief 回転の設定
		 * @param rot 回転
		 */
		inline void SetRotation(const Quaternion& rot) { m_rotation = rot; }

		/**
		 * @brief 大きさの設定
		 * @param sca 大きさ
		 */
		inline void SetScale(const Vector3& sca) { m_scale = sca; }
		/**
		 * @brief 大きさの設定
		 * @param x x方向の大きさ
		 * @param y y方向の大きさ
		 * @param z z方向の大きさ
		 */
		inline void SetScale(const float& x, const float& y, const float& z) { m_scale = Vector3(x, y, z); }

		/**
		 * @brief アニメーション再生速度の設定
		 * @param speed 再生速度（1.0f=通常速度）
		 */
		inline void SetAnimationSpeed(float speed)
		{
			// アニメーションの再生速度の最小値
			constexpr float minAnimationSpeed = 0.1f;
			// アニメーションの再生速度の最大値
			constexpr float maxAnimationSpeed = 10.0f;

			m_animationSpeed = std::clamp(speed, minAnimationSpeed, maxAnimationSpeed);
		}

		/**
		 * @brief モデルの取得
		 * @return モデル
		 */
		inline Model& GetModel() { return m_model; }

		/**
		 * @brief モデルの全マテリアルに乗算カラーを設定する
		 * @details ディファード描画用の m_renderToGBufferModel にも同時に適用する
		 * @param mulColor 乗算カラー (RGBA, 1.0f=変更なし)
		 */
		inline void SetMulColor(const Vector4& mulColor)
		{
			m_model.SetMulColor(mulColor);
			m_renderToGBufferModel.SetMulColor(mulColor);
			m_forwardRenderModel.SetMulColor(mulColor);
		}

		/**
		 * @brief PBR補正パラメータを設定する
		 * @details Init呼び出し前に設定すること。
		 *          RenderToGBuffer.fx の b2 スロットに渡される。
		 * @param param PBR補正パラメータ
		 */
		inline void SetPBRParam(const PBRParam& param) { m_pbrParam = param; }

		/**
		 * @brief PBR補正パラメータを取得する
		 * @return PBR補正パラメータ
		 */
		inline const PBRParam& GetPBRParam() const { return m_pbrParam; }

		/**
		 * @brief フォワードレンダリングで描画するかどうかを設定する
		 * @details SkyCubeなど、GBufferを経由せず直接フォワードで描画したいモデルに使用する。
		 *          trueにした場合は m_forwardRenderModel が描画に使用される。
		 * @param isForward trueならフォワード、falseならディファード
		 */
		inline void SetForwardRendering(const bool isForward) { m_isForwardRender = isForward; }

		/**
		 * @brief アニメーションが再生中か
		 * @return 再生中ならtrue
		 */
		inline bool IsPlayingAnimation() const { return m_animation.IsPlaying(); }

		/**
		 * @brief ユーザー拡張の定数バッファ（b2）のデータポインタをInit後に差し替える
		 * @details
		 *   GBufferモデルはb2をPBRParamで使用しているため、
		 *   m_modelとm_forwardRenderModelのみに設定する。
		 * @param data 新しいデータポインタ
		 */
		void SetExpandConstantBuffer2(void* data);

		/**
		 * @brief ユーザー拡張の定数バッファ（b3）のデータポインタをInit後に差し替える
		 * @details
		 *   OcclusionDitherManagerからRegister時に呼ばれる。
		 *   GBufferパス（m_renderToGBufferModel）のb3にDitherCbをセットする。
		 *   次のDraw()からdataの中身が自動でGPUに転送される。
		 *   渡すデータはSDitherCbPlaceholderと同サイズであること。
		 * @param data 新しいデータポインタ（SDitherCb*を渡すこと）
		 */
		void SetExpandConstantBuffer3(void* data);

		/**
		 * @brief モデル単位のディザリング透過率を設定する
		 * @details
		 *   カメラ遮蔽ディザリング（OcclusionDitherManager/b3）とは独立して動作する。
		 *   GBufferパス（m_renderToGBufferModel）のb4（ModelDitherCb）を更新する。
		 *   Init呼び出し後にいつでも設定可能。次のDraw()からGPUに自動転送される。
		 * @param alpha 透過率（0.0f=オフ, 1.0f=完全消去）
		 */
		inline void SetDitherAlpha(const float alpha)
		{
			m_modelDitherCb.modelDitherAlpha = alpha;
		}

		/**
		 * @brief フラスタムカリングの有効/無効を設定する
		 * @details
		 *   デフォルトはtrue（有効）。
		 *   常に描画が必要なオブジェクトはfalseに設定する。
		 * @param enabled trueでカリング有効、falseで常に描画
		 */
		inline void SetCullingEnabled(const bool enabled) { m_isCullingEnabled = enabled; }

		/**
		 * @brief フラスタムカリングが有効か取得する
		 * @return カリング有効ならtrue
		 */
		inline bool IsCullingEnabled() const { return m_isCullingEnabled; }

		/**
		 * @brief ワールド空間AABBの最小点を取得する
		 * @details RenderingEngineのカリング判定で参照される。
		 * @return ワールド空間AABBの最小点
		 */
		inline const Vector3& GetWorldAABBMin() const { return m_worldAABBMin; }

		/**
		 * @brief ワールド空間AABBの最大点を取得する
		 * @return ワールド空間AABBの最大点
		 */
		inline const Vector3& GetWorldAABBMax() const { return m_worldAABBMax; }


	public:
		ModelRender()
			: m_position(Vector3::Zero)
			, m_scale(Vector3::One)
			, m_rotation(Quaternion::Identity)
			, m_animationClips(nullptr)
			, m_maxInstance(1)
			, m_numAnimationClips(0)
			, m_animationSpeed(1.0f)
		{};
		~ModelRender() = default;

		/**
		 * @brief モデルの初期化用関数
		 * @param filePath			ファイルパス
		 * @param animationClips	アニメーションクリップ
		 * @param numAnimationClips アニメーションの数
		 * @param islighting		ライティングの有効/無効
		 * @param enModelUpAxis		モデルの上方向
		 */
		void Init(
			const char* filePath,
			AnimationClip* animationeClips = nullptr,
			int numAnimationClips = 0,
			bool islighting = true,
			EnModelUpAxis enModelUpAxiz = enModelUpAxisZ
		);

		/**
		 * @brief 事前ロード済みのデータから初期化する
		 * @param initData			完成済みのModelInitData（tkmパスやシェーダ設定などを含む）
		 * @param skeleton			外部でロード済みのスケルトン（不要ならnullptr）
		 * @param animationeClips	外部でロード済みのアニメーションクリップ配列（不要ならnullptr）
		 * @param numAnimationClips クリップ数
		 */
		void InitFromLoaded(
			const ModelInitData& initData,
			Skeleton* skeleton = nullptr,
			AnimationClip* animationeClips = nullptr,
			int numAnimationClips = 0
		);

		/**
		 * @brief アニメーションの再生
		 * @param animNo			アニメーションクリップの番号
		 * @param interpolateTime	補完時間(単位：秒)
		 */
		void PlayAnimation(int animNo, float interpolateTime = 0.0f)
		{
			m_animation.Play(animNo, interpolateTime);
		}

		/**
		 * @brief 描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void Draw(RenderContext& rc);

		/**
		 * @brief RenderingEngineから呼ばれる実際の描画処理
		 * @details フォワードの場合はm_forwardRenderModel、
		 *          ディファードの場合はm_renderToGBufferModelを描画する
		 * @param rc レンダリングコンテキスト
		 */
		void OnDraw(RenderContext& rc);

		/**
		 * @brief アニメーションイベントリスナーの登録
		 * @param eventListener 登録するリスナー
		 */
		void AddAnimationEvent(AnimationEventListener eventListener)
		{
			m_animation.AddAnimationEventListener(eventListener);
		}

		/**
		 * @brief 更新
		 */
		void Update();


	private:
		/**
		 * @brief シャドウマップへの描画パスから呼ばれる処理
		 * @param rc レンダリングコンテキスト
		 */
		void OnRenderShadowMap(RenderContext& rc) override;

		/**
		 * @brief スケルトンの初期化用関数
		 * @param filePath ファイルパス
		 */
		void InitSkeleton(const char* filePath);

		/**
		 * @brief アニメーションの初期化用関数
		 * @param animtionClips		アニメーションクリップ
		 * @param numAnimationClips アニメーションの数
		 * @param enModelUpAxis		モデルの上方向
		 */
		void InitAnimation(
			AnimationClip* animtionClips,
			int numAnimationClips,
			EnModelUpAxis enModelUpAxis
		);

		/**
		 * @brief GBuffer描画用モデルの初期化
		 * @param baseInitData m_modelの初期化データをベースに使用する
		 */
		void InitRenderToGBufferModel(const ModelInitData& baseInitData);

		/**
		 * @brief シェーダーのエントリーポイントの設定
		 * @param modelInitData モデルの初期化データ
		 */
		void SetupShaderEntryPointFunc(ModelInitData& modelInitData);

		/**
		 * @brief 各種モデルのワールド行列の更新
		 */
		void UpdateWorldMatrixInModes();

		/**
		 * @brief tkmファイルの頂点からローカルAABBを計算する
		 * @details Init時に1回だけ呼ばれる。スケルトンなしモデルで使用する。
		 * @param filePath tkmファイルパス
		 */
		void CalcLocalAABBFromTkm(const char* filePath);

		/**
		 * @brief ワールド空間AABBを更新する
		 * @details Update()の末尾で毎フレーム呼ばれる。
		 *          スケルトンありの場合はボーン位置から、
		 *          なしの場合はローカルAABBをワールド変換して構築する。
		 */
		void UpdateWorldAABB();


	private:
		/** 位置 */
		Vector3			m_position;
		/** 大きさ */
		Vector3			m_scale;
		/** 回転 */
		Quaternion		m_rotation;
		/** モデル */
		Model			m_model;
		/** シャドウマップ用モデル */
		Model			m_shadowModels;
		/** ボーン（自前保有） */
		Skeleton		m_skeleton;
		/** ボーン参照（外部注入または自前） */
		Skeleton* m_skeletonRef = nullptr;
		/** アニメーション */
		Animation		m_animation;
		/** アニメーションクリップ */
		AnimationClip* m_animationClips;
		/** シャドウマップ用のカメラパラメータを格納する定数バッファ */
		ConstantBuffer	m_drawShadowMapCameraParamCB;
		/** 最大インスタンス数 */
		int				m_maxInstance;
		/** アニメーションクリップの数 */
		int				m_numAnimationClips;
		/** アニメーションの再生速度 */
		float			m_animationSpeed;

		/** フォワードレンダリングで描画するか */
		bool		m_isForwardRender = false;
		/** フォワードレンダリングで描画されるモデル */
		Model		m_forwardRenderModel;
		/** Gバッファに描画されるモデル */
		Model		m_renderToGBufferModel;
		/** 描画するかどうか */
		bool		m_visible = true;
		/** PBR補正パラメータ */
		PBRParam	m_pbrParam;
		/**
		 * @brief ディザリングCB（b3）のプレースホルダー
		 * @details
		 *   InitRenderToGBufferModel時にb3のConstantBufferを確保するために使用する。
		 *   OcclusionDitherManager::Register後はSetExpandConstantBuffer3()で
		 *   実際のSDitherCbのポインタに差し替えられる。
		 */
		SDitherCbPlaceholder m_ditherCbPlaceholder;
		/**
		 * @brief モデル単位ディザリングCB（b4）
		 * @details
		 *   InitRenderToGBufferModel時にb4のConstantBufferを確保し、常にこの実体を参照する。
		 *   SetDitherAlpha()でmodelDitherAlphaを更新すると次のDraw()でGPUに転送される。
		 *   OcclusionDitherManager（b3）とは独立して動作する。
		 */
		SModelDitherCb m_modelDitherCb;
		/**
		 * @brief ローカル空間AABB（Init時に計算・以降不変）
		 * @details スケルトンなしモデルのカリング判定の基準として使用する。
		 */
		AABB		m_localAABB;
		/**
		 * @brief ワールド空間AABBの最小点（毎フレーム更新）
		 * @details RenderingEngineのカリング判定で参照される。
		 */
		Vector3		m_worldAABBMin = Vector3::Zero;
		/**
		 * @brief ワールド空間AABBの最大点（毎フレーム更新）
		 * @details RenderingEngineのカリング判定で参照される。
		 */
		Vector3		m_worldAABBMax = Vector3::Zero;
		/**
		 * @brief フラスタムカリング有効フラグ
		 * @details falseにすると常に描画される。デフォルトはtrue。
		 */
		bool		m_isCullingEnabled = true;
		/**
		 * @brief スケルトンを持つか（ボーンAABB更新の分岐用）
		 * @details Init時にスケルトンが有効な場合にtrueが設定される。
		 */
		bool		m_hasSkeleton = false;
		/**
		 * @brief ボーンAABBの安全マージン（ワールド単位）
		 * @details アニメーション中にボーン位置がAABBを超えないよう余裕を持たせる。
		 */
		static constexpr float BONE_AABB_MARGIN = 20.0f;
	};
}