/**
 * @file ModelRender.h
 * @brief モデルレンダー
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
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
		 * @brief フォワードレンダリングで描画するかどうかを設定する
		 * @details SkyCubeなど、GBufferを経由せず直接フォワードで描画したいモデルに使用する。
		 *          trueにした場合は m_frowardRenderModel が描画に使用される。
		 * @param isForward trueならフォワード、falseならディファード
		 */
		inline void SetForwardRendering(const bool isForward) { m_isForwardRender = isForward; }

		/**
		 * @brief アニメーションが再生中か
		 * @return 再生中ならtrue
		 */
		inline bool IsPlayingAnimation() const { return m_animation.IsPlaying(); }


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
		 * @details フォワードの場合はm_frowardRenderModel、
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
		bool m_isForwardRender = false;
		/** フォワードレンダリングで描画されるモデル */
		Model m_forwardRenderModel;
		/** Gバッファに描画されるモデル */
		Model m_renderToGBufferModel;
		/** 描画するかどうか */
		bool  m_visible = true;
	};
}