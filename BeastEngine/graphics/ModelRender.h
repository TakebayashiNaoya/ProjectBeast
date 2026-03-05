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
		 *@param pos 位置
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
		{
		};
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
		 * @brief 海の初期化用関数
		 * @param initData		モデルの初期化データ
		 * @param tkmFilePath	tkmファイルのファイルパス
		 */
		void InitOcean(ModelInitData& initData, const char* tkmFilePath);

		void InitModelOnZprepass(const char* tkmFilePath, EnModelUpAxis modelUpAxis, bool isSkyCube);

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
		 * @brief アニメーション頂点バッファの初期化用関数
		 * @param tkmFilePath		ファイルパス
		 * @param enModelUpAxis	モデルの上方向
		 */
		void InitComputeAnimatoinVertexBuffer(
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis
		);

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
		/** ボーン */
		Skeleton		m_skeleton;
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
		bool m_isFowardRender = false;
		/** 反射で映りこむかどうか */
		std::map<ReflectLayer, bool> m_enableReflection;
		/** フォワードレンダリングで描画されるモデル */
		Model m_frowardRenderModel;
		/** インスタンシング描画が有効か */
		bool m_isEnableInstancingDraw = false;
		/** ワールド行列の配列のストラクチャードバッファ */
		StructuredBuffer m_worldMatrixArraySB;
		/** ZPrepassで描画されるモデル */
		Model m_zprepassModel;
	};
}