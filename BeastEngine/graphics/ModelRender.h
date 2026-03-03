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
		 * @brief アニメーションが再生中か
		 * @return 再生中ならtrue
		 */
		inline bool IsPlayingAnimation() const { return m_animation.IsPlaying(); }

		/**
		 * @brief 位置、回転、大きさの設定
		 * @param pos 位置
		 * @param rot 回転
		 * @param sca 大きさ
		 */
		inline void SetTRS(const Vector3& pos, const Quaternion& rot, const Vector3& sca)
		{
			m_pos = pos;
			m_rot = rot;
			m_sca = sca;
		}

		/**
		 * @brief 位置の設定
		 *@param pos 位置
		 */
		inline void SetPosition(const Vector3& pos) { m_pos = pos; }
		/**
		 * @brief 位置の設定
		 * @param x x座標
		 * @param y y座標
		 * @param z z座標
		 */
		inline void SetPosition(const float& x, const float& y, const float& z) { m_pos = Vector3(x, y, z); }

		/**
		 * @brief 回転の設定
		 * @param rot 回転
		 */
		inline void SetRotation(const Quaternion& rot) { m_rot = rot; }

		/**
		 * @brief 大きさの設定
		 * @param sca 大きさ
		 */
		inline void SetScale(const Vector3& sca) { m_sca = sca; }
		/**
		 * @brief 大きさの設定
		 * @param x x方向の大きさ
		 * @param y y方向の大きさ
		 * @param z z方向の大きさ
		 */
		inline void SetScale(const float& x, const float& y, const float& z) { m_sca = Vector3(x, y, z); }

		/**
		 * @brief モデルの取得
		 * @return モデル
		 */
		inline Model& GetModel() { return m_model; }


	public:
		ModelRender()
			: m_pos(Vector3::Zero)
			, m_sca(Vector3::One)
			, m_rot(Quaternion::Identity)
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

		Vector3			m_pos = Vector3::Zero;			//位置
		Vector3			m_sca = Vector3::One;			//大きさ
		Quaternion		m_rot = Quaternion::Identity;	//回転

		Model			m_model;					//モデル
		Model			m_shadowModels;

		Skeleton		m_skeleton;					//ボーン
		Animation		m_animation;				//アニメーション
		AnimationClip* m_animationClips = nullptr;	//アニメーションクリップ
		ConstantBuffer	m_drawShadowMapCameraParamCB;

		int				m_maxInstance = 1;			// 最大インスタンス数。
		int				m_numAnimationClips = 0;	//アニメーションクリップの数
		float			m_animationSpeed = 1.0f;	//アニメーションの再生速度
	};
}