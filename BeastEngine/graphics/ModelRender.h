/**
 * @file ModelRender.h
 * @brief モデルレンダー
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	// BeastModel は前方宣言のみ（cpp でインクルード）
	class BeastModel;
	// Frustum は参照でのみ使用するため前方宣言で十分
	class Frustum;

	/**
	 * @brief PBR補正パラメータ
	 * @details モデルごとに個別設定できるPBRライティングの補正値。
	 *          RenderToGBuffer.fx の PBRParamCb (b2) に渡される。
	 */
	struct PBRParam
	{
		float m_dirLightScale = 1.0f;	/** ディレクションライト強度倍率 */
		float m_ambientScale = 1.0f;	/** 環境光強度倍率 */
		float m_metallicOffset = 0.0f;	/** metallicオフセット */
		float m_smoothOffset = 0.0f;	/** smoothオフセット */
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
	 * @brief トゥーンシェーダー用定数バッファ（b2）
	 * @details
	 *   toon.fx の cbuffer ToonCb : register(b2) に対応する。
	 *   toon.fx は独立した fxファイルのため b2 が空いている。
	 *   閾値・係数は明るい側から順に設定すること（thresholds[0] > thresholds[1] > ...）。
	 *   stepCount で実際に使う段階数を指定する（1〜4）。
	 *   SetToonParam()で更新し、次のDraw()でGPUに自動転送される。
	 */
	struct SToonCb
	{
		/** 各段階の閾値（x=段階1, y=段階2, z=段階3, w=段階4）明るい側から順に */
		Vector4 shadowThresholds = { 0.0f, -0.3f, -0.6f, -1.0f };
		/** 各段階の暗さ係数（x=段階1, y=段階2, z=段階3, w=段階4） */
		Vector4 shadowColorRates = { 0.8f,  0.6f,  0.4f,  0.2f };
		/** 実際に使う段階数（1〜4） */
		int     stepCount = 2;
		/** パディング（16バイト境界用） */
		float   pad[3] = { 0.0f, 0.0f, 0.0f };
	};

	/**
	 * @brief アウトラインシェーダー用定数バッファ（b2）
	 * @details
	 *   outline.fx の cbuffer OutlineCb : register(b2) に対応する。
	 *   outline.fx は独立した fxファイルのため b2 が空いている。
	 *   SetOutlineParam()で更新し、次のDraw()でGPUに自動転送される。
	 */
	struct SOutlineCb
	{
		float   outlineWidth = 0.005f;					/** 輪郭線の太さ（法線押し出し量） */
		float   pad[3] = { 0.0f, 0.0f, 0.0f };			/** パディング（16バイト境界用） */
		Vector4 outlineColor = { 0.0f, 0.0f, 0.0f, 1.0f };	/** 輪郭線の色（RGBA） */
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
		inline void SetPosition(const float& x, const float& y, const float& z)
		{
			m_position = Vector3(x, y, z);
		}

		/**
		 * @brief 位置の取得
		 * @return 位置
		 */
		inline const Vector3& GetPosition() const { return m_position; }

		/**
		 * @brief 指定した名前のボーンのワールド座標を取得する
		 * @details エフェクトの発生位置など、特定ボーンに追従させたい場合に使用する。
		 * @param boneName ボーン名
		 * @return ボーンのワールド座標。スケルトンが無い、またはボーンが見つからない場合は GetPosition() を返す
		 */
		Vector3 GetBoneWorldPosition(const wchar_t* boneName) const;

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
		inline void SetScale(const float& x, const float& y, const float& z)
		{
			m_scale = Vector3(x, y, z);
		}

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

			m_animationSpeed = std::max<float>(minAnimationSpeed, speed);
			m_animationSpeed = std::min<float>(maxAnimationSpeed, m_animationSpeed);
		}

		/**
		 * @brief モデルの取得
		 * @details 影モデルなど k2EngineLow::Model が必要な箇所向けに残す
		 * @return モデル
		 */
		inline Model& GetModel() { return m_model; }

		/**
		 * @brief 実際の描画・スキニングに使用されているスケルトンを取得する
		 * @details Init()なら内部で.tksから読み込んだ自前のm_skeletonへのポインタ、
		 *          InitFromLoaded()なら外部から渡されたスケルトンへのポインタ（m_skeletonRef）を返す。
		 *          いずれの場合も、GetBoneMatricesTopAddress()の中身が実際のスキニングに使われる本体。
		 *          CharacterBase側で別途保持しているSkeletonとは別物なので注意すること。
		 * @return スケルトンへのポインタ（スケルトンを持たないモデルの場合はnullptr）
		 */
		inline Skeleton* GetSkeleton() const { return m_skeletonRef; }

		/**
		 * @brief モデルの全マテリアルに乗算カラーを設定する
		 * @details ディファード描画用の m_renderToGBufferModel にも同時に適用する
		 * @param mulColor 乗算カラー (RGBA, 1.0f=変更なし)
		 */
		void SetMulColor(const Vector4& mulColor);

		/**
		 * @brief モデルの透明度を設定する
		 * @details
		 *   model.fxを使用するモデル（m_model・m_forwardRenderModel）には
		 *   乗算カラーのα成分として渡され、実際の半透明ブレンドで反映される。
		 *   GBufferパス（m_renderToGBufferModel）は本来アルファブレンドができないため、
		 *   代わりにモデル単位ディザリング（b4）の透過率として反映する。
		 * @param alpha 透明度（1.0f=不透明, 0.0f=完全透明）
		 */
		void SetAlpha(const float alpha);

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
		 * @brief GBuffer描画パスで使用するシェーダーのパスを上書きする
		 * @details 未設定時は RenderToGBuffer.fx が使用される。
		 *          地形など独自のGBufferシェーダーが必要なモデルに使用する。
		 *          Init() / InitFromLoaded() より前に呼ぶこと。
		 * @param path シェーダーファイルパス（例: "Assets/shader/Terrain.fx"）
		 */
		inline void SetGBufferFxFilePath(const std::string& path) { m_customGBufferFxPath = path; }

		/**
		 * @brief アニメーションが再生中か
		 * @return 再生中ならtrue
		 */
		inline bool IsPlayingAnimation() const { return m_animation.IsPlaying(); }

		/**
		 * @brief ユーザー拡張の定数バッファ（b2）のデータポインタをInit後に差し替える
		 * @details
		 *   GBufferモデルはb2をPBRParamで使用しているため、
		 *   m_model と m_forwardRenderModel のみに設定する。
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

		/**
		 * @brief このモデルに対してトゥーンシェーダーを有効にする
		 * @details
		 *   Init()より前に呼ぶこと。
		 *   s_isToonGlobalEnabled が true の場合は本フラグに関わらず全モデルで有効になる。
		 * @param enabled trueでトゥーン有効
		 */
		inline void SetToonEnabled(const bool enabled) { m_isToonEnabled = enabled; }

		/**
		 * @brief トゥーンシェーダーが有効か取得する
		 * @details 個別フラグのみで判定する。
		 *          グローバルフラグはInit()時に個別フラグへ焼き込まれる。
		 * @return トゥーン有効ならtrue
		 */
		inline bool IsToonEnabled() const { return m_isToonEnabled; }

		/**
		 * @brief トゥーンシェーダーのパラメータを設定する
		 * @details Init()より前でも後でも設定可能。次のDraw()からGPUに転送される。
		 *          閾値は明るい側から順に設定すること（thresholds.x > thresholds.y > ...）。
		 * @param shadowThresholds	各段階の閾値（x=段階1, y=段階2, z=段階3, w=段階4）
		 * @param shadowColorRates	各段階の暗さ係数（x=段階1, y=段階2, z=段階3, w=段階4）
		 * @param stepCount			実際に使う段階数（1〜4）
		 */
		inline void SetToonParam(
			const Vector4& shadowThresholds,
			const Vector4& shadowColorRates,
			const int stepCount = 2)
		{
			m_toonCb.shadowThresholds = shadowThresholds;
			m_toonCb.shadowColorRates = shadowColorRates;
			m_toonCb.stepCount = max(1, min(stepCount, 4));
		}

		/**
		 * @brief アウトラインのパラメータを設定する
		 * @details Init()より前でも後でも設定可能。次のDraw()からGPUに転送される。
		 * @param outlineWidth	輪郭線の太さ（法線押し出し量）
		 * @param outlineColor	輪郭線の色（RGBA）
		 */
		inline void SetOutlineParam(const float outlineWidth, const Vector4& outlineColor)
		{
			m_outlineCb.outlineWidth = outlineWidth;
			m_outlineCb.outlineColor = outlineColor;
		}

		/**
		 * @brief 全モデルに対してトゥーンシェーダーを一括で有効/無効にする
		 * @details RenderingEngineなどから呼ぶことを想定している。
		 * @param enabled trueで全モデルのトゥーンが有効になる
		 */
		static void SetToonGlobalEnabled(const bool enabled)
		{
			s_isToonGlobalEnabled = enabled;
		}

		/**
		 * @brief 全体トゥーンフラグの状態を取得する
		 * @return 全体フラグが有効ならtrue
		 */
		static bool IsToonGlobalEnabled()
		{
			return s_isToonGlobalEnabled;
		}


	public:
		ModelRender();
		~ModelRender();

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
		 *          ディファードの場合はm_renderToGBufferModelを描画する。
		 *          どちらもトライアングルカリングを適用する。
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

		/**
		 * @brief シャドウマップへの描画パスから呼ばれる処理
		 * @details ShadowMap から呼ばれる。カメラではなくライトの行列で描画する。
		 * @param rc                レンダリングコンテキスト
		 * @param cascadeIndex      カスケードの番号（0が最も近景）
		 * @param lightViewMatrix   ライトのビュー行列
		 * @param lightProjMatrix   ライトのプロジェクション行列
		 */
		void OnRenderShadowMap(
			RenderContext& rc,
			const int cascadeIndex,
			const Matrix& lightViewMatrix,
			const Matrix& lightProjMatrix
		) override;

		/**
		 * @brief 影を落とすかどうかを設定
		 * @param isCastShadow 影を落とすかどうか
		 */
		void SetCastShadow(const bool isCastShadow) { m_isCastShadow = isCastShadow; }

		/**
		 * @brief 影を落とすかどうかを取得
		 * @return 影を落とすかどうか
		 */
		bool IsCastShadow() const { return m_isCastShadow; }


	private:
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
		 * @brief シャドウマップ描画用モデルの初期化
		 * @details 深度だけを書き込む shadowMap.fx で初期化する。
		 *          同じtkmを別シェーダーで持つ点は GBuffer 用モデルと同じ作り。
		 * @param baseInitData m_modelの初期化データをベースに使用する
		 */
		void InitShadowModel(const ModelInitData& baseInitData);

		/**
		 * @brief トゥーンモデル・アウトラインモデルの初期化
		 * @details IsToonEnabled()がtrueの場合のみ呼ばれる。
		 * @param baseInitData m_modelの初期化データをベースに使用する
		 */
		void InitToonModels(const ModelInitData& baseInitData);

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
		Vector3        m_position;
		/** 大きさ */
		Vector3        m_scale;
		/** 回転 */
		Quaternion     m_rotation;

		/**
		 * @brief 影モデル・GetModel()用の k2EngineLow::Model
		 * @details トライアングルカリングは不要なためそのまま使用する
		 */
		Model          m_model;
		/**
		 * @brief シャドウマップ用モデル（カスケードごとに1つ）
		 * @details モデルの定数バッファ（mWorld/mView/mProj）はメッシュごとに1つしかなく、
		 *          描画のたびに同じ場所へ上書きされる。
		 *          1フレームに同じモデルを複数のカスケードへ描くと、
		 *          コマンドが実行される時点では最後に書いたカスケードの行列しか残らず、
		 *          全カスケードが同じ（最も広い）範囲で描かれてしまう。
		 *          カスケードごとに別のモデルを持たせて定数バッファを分ける。
		 */
		std::array<Model, NUM_SHADOW_CASCADES> m_shadowModels;
		/** シャドウマップ用モデルを初期化済みか */
		bool           m_isShadowModelInited = false;
		/** 影を落とすかどうか */
		bool           m_isCastShadow = true;

		/**
		 * @brief GBuffer描画用モデル（BeastModel、前方宣言のためunique_ptrで保持）
		 * @details トライアングルカリングを OnDraw() 内で適用する
		 */
		std::unique_ptr<BeastModel> m_renderToGBufferModel;

		/**
		 * @brief フォワードレンダリング描画用モデル（BeastModel、前方宣言のためunique_ptrで保持）
		 * @details トライアングルカリングを OnDraw() 内で適用する
		 */
		std::unique_ptr<BeastModel> m_forwardRenderModel;

		/**
		 * @brief トゥーンシェーダー描画用モデル（フォワードパスで描画）
		 * @details IsToonEnabled()がtrueの場合のみ InitToonModels() で初期化される。
		 *          GBufferへの書き込みをスキップし、toon.fx でフォワード描画する。
		 */
		std::unique_ptr<BeastModel> m_toonModel;

		/**
		 * @brief アウトライン描画用モデル（フォワードパスで描画）
		 * @details IsToonEnabled()がtrueの場合のみ InitToonModels() で初期化される。
		 *          背面法線押し出し方式で輪郭線を描画する。カリングはフロントフェース。
		 */
		std::unique_ptr<BeastModel> m_outlineModel;

		/** ボーン（自前保有） */
		Skeleton       m_skeleton;
		/** ボーン参照（外部注入または自前） */
		Skeleton* m_skeletonRef = nullptr;
		/** アニメーション */
		Animation      m_animation;
		/** アニメーションクリップ */
		AnimationClip* m_animationClips;
		/** シャドウマップ用のカメラパラメータを格納する定数バッファ */
		ConstantBuffer m_drawShadowMapCameraParamCB;
		/** 最大インスタンス数 */
		int            m_maxInstance;
		/** アニメーションクリップの数 */
		int            m_numAnimationClips;
		/** アニメーションの再生速度 */
		float          m_animationSpeed;

		/** フォワードレンダリングで描画するか */
		bool           m_isForwardRender = false;
		/** 描画するかどうか */
		bool           m_visible = true;
		/** このモデル個別のトゥーン有効フラグ */
		bool           m_isToonEnabled = false;
		/** PBR補正パラメータ */
		PBRParam       m_pbrParam;

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

		/** トゥーンシェーダー用定数バッファデータ（b2） */
		SToonCb m_toonCb;
		/** アウトラインシェーダー用定数バッファデータ（b2） */
		SOutlineCb m_outlineCb;

		/**
		 * @brief ローカル空間AABB（Init時に計算・以降不変）
		 * @details スケルトンなしモデルのカリング判定の基準として使用する。
		 */
		AABB           m_localAABB;

		/**
		 * @brief ワールド空間AABBの最小点（毎フレーム更新）
		 * @details RenderingEngineのカリング判定で参照される。
		 */
		Vector3        m_worldAABBMin = Vector3::Zero;

		/**
		 * @brief ワールド空間AABBの最大点（毎フレーム更新）
		 * @details RenderingEngineのカリング判定で参照される。
		 */
		Vector3        m_worldAABBMax = Vector3::Zero;

		/**
		 * @brief フラスタムカリング有効フラグ
		 * @details falseにすると常に描画される。デフォルトはtrue。
		 */
		bool           m_isCullingEnabled = true;

		/**
		 * @brief スケルトンを持つか（ボーンAABB更新の分岐用）
		 * @details Init時にスケルトンが有効な場合にtrueが設定される。
		 */
		bool           m_hasSkeleton = false;

		/**
		 * @brief ボーンAABBの安全マージン（ワールド単位）
		 * @details アニメーション中にボーン位置がAABBを超えないよう余裕を持たせる。
		 */
		static constexpr float BONE_AABB_MARGIN = 20.0f;

		/**
		 * @brief デバッグ用モデル名（調査用・確認後に削除）
		 * @details WhiteBear など特定モデルのログ絞り込みに使用する
		 */
		std::string    m_debugName;

		/** 全モデル共通のトゥーン有効フラグ（staticで全インスタンスに共有） */
		static bool s_isToonGlobalEnabled;

		/** GBuffer描画パスで使用するカスタムシェーダーパス（空文字時はRenderToGBuffer.fxを使用） */
		std::string    m_customGBufferFxPath;
	};
}