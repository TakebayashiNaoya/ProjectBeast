/**
 * @file SpriteRender.h
 * @brief 2Dスプライト描画クラス
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * 2Dスプライト描画クラス
	 */
	class SpriteRender :public IRenderer
	{
	public:
		/**
		 * @brief 位置の設定
		 * @param position	位置
		 */
		void SetPosition(const Vector3& position)
		{
			m_position.x = position.x;
			m_position.y = position.y;
			m_position.z = position.z;
		}
		/**
		 * @brief 位置の設定
		 * @param position	位置
		 */
		void SetPosition(const Vector2& position)
		{
			m_position.x = position.x;
			m_position.y = position.y;
			m_position.z = 0.0f;
		}

		/**
		 * @brief 位置の設定
		 * @param x	X座標
		 * @param y	Y座標
		 */
		void SetPosition(const float& x, const float& y)
		{
			m_position = Vector3(x, y, 0.0f);
		}

		/**
		 * @brief 回転の設定
		 * @param rotation	回転
		 */
		void SetRotation(const Quaternion& rotation)
		{
			m_rotation = rotation;
		}

		/**
		 * @brief 大きさの設定
		 * @param scale	大きさ
		 */
		void SetScale(const Vector2& scale)
		{
			m_scale.x = scale.x;
			m_scale.y = scale.y;
			m_scale.z = 0.0f;
		}
		/**
		 * @brief 大きさの設定
		 * @param scale	大きさ
		 */
		void SetScale(const Vector3& scale)
		{
			m_scale.x = scale.x;
			m_scale.y = scale.y;
			m_scale.z = 0;
		}

		/**
		 * @brief 基点の設定
		 * @param pivot	基点
		 */
		void SetPivot(const Vector2& pivot)
		{
			m_pivot = pivot;
		}

		/**
		 * @brief 乗算カラーの設定
		 * @param mulColor	乗算カラー
		 */
		void SetMulColor(const Vector4& mulColor)
		{
			m_sprite.SetMulColor(mulColor);
		}


	public:
		SpriteRender()
			: m_sprite()
			, m_pivot(Sprite::DEFAULT_PIVOT)
			, m_position(Vector3::Zero)
			, m_scale(Vector3::One)
			, m_rotation(Quaternion::Identity)
		{
		}
		~SpriteRender() = default;

		/**
		 * @brief 画像の初期化用関数
		 * @param filePath			画像ファイルのパス
		 * @param width				画像の横幅のサイズ
		 * @param height			画像の縦幅のサイズ
		 * @param alphaBlendMode	アルファブレンドモード
		 */
		void Init(const char* filePath, const float width, const float height, AlphaBlendMode alphaBlendMode = AlphaBlendMode_Trans);

		/**
		 * @brief 更新処理
		 */
		void Update()
		{
			m_sprite.Update(m_position, m_rotation, m_scale, m_pivot);
		}

		/**
		 * @brief 描画処理
		 * @param rc レンダリングコンテキスト
		 */
		void Draw(RenderContext& rc);


	private:
		/**
		 * @brief 2D描画パスから呼ばれる処理
		 * @param rc レンダリングコンテキスト
		 */
		void OnRender2D(RenderContext& rc)override
		{
			m_sprite.Draw(rc);
		}


	private:
		/** スプライト */
		Sprite		m_sprite;
		/** 基点 */
		Vector2		m_pivot;
		/** 位置 */
		Vector3		m_position;
		/** 大きさ */
		Vector3		m_scale;
		/** 回転 */
		Quaternion	m_rotation;
	};


	/**
	 * @brief 円形ゲージレンダラー(任意範囲版(AND判定バージョン))
	 * @details ゲージの描画に特化したクラス。
	 */
	class GaugeRender : public IRenderer
	{
	public:
		/**
		 * @breif ゲージ描画用の定数バッファ構造体
		 * @param startProgress ゲージの開始位置(0.0f ~ 1.0f)
		 * @param endProgress ゲージの終了位置(0.0f ~ 1.0f)
		 * @param innerRadius ゲージの内径
		 * @param outerRadius ゲージの外径
		 * @param rotationAngle ゲージの回転角度(0.0f ~ 360.0f)
		 * @param gaugeColor ゲージ部分の色
		 * @param bgColor リング部分(背景)の色
		 * @details
		 * padding0~3は、定数バッファのサイズを16バイトの倍数にするためのダメ―変数です。
		 */
		struct GaugeConstantBuffer
		{
			float startProgress;
			float endProgress;
			float innerRadius;
			float outerRadius;
			float rotationAngle;
			float padding0;
			float padding1;
			float padding2;
			Vector4 gaugeColor;
			Vector4 bgColor;
		};


	public:
		GaugeRender()
			: m_position(Vector3::Zero)
			, m_scale(Vector3::One)
			, m_rotation(Quaternion::Identity)
			, m_pivot(Sprite::DEFAULT_PIVOT)
		{}

		~GaugeRender() = default;

		/**
		 * @brief 初期化
		 * @param filePath　		画像ファイルのパス
		 * @param fxName		　　シェーダーの名前
		 * @param w				　　画像の横幅のサイズ
		 * @param h				　　画像の縦幅のサイズ
		 */
		void Init(const char* filePath,const char* fxName, float w, float h);

		/**
		 * @brief 位置の設定
		 * @param position 位置
		 */
		void SetPosition(const Vector3& position) { m_position = position; }
		/**
		 * @brief 位置の取得
		 * @return 位置
		 */
		const Vector3& GetPosition()const { return m_position; }

		/**
		 * @brief 大きさの設定
		 * @param scale 大きさ
		 */
		void SetScale(const Vector3& scale) { m_scale = scale; }
		/**
		 * @brief 大きさの取得
		 * @return 大きさ
		 */
		const Vector3& GetScale()const { return m_scale; }

		/**
		 * @brief 回転の設定
		 * @param rotation 回転
		 */
		void SetRotation(const Quaternion& rotation) { m_rotation = rotation; }
		/**
		 * @brief 回転の取得
		 * @return 回転
		 */
		const Quaternion& GetRotation()const { return m_rotation; }

		/**
		 * @brief ピボットの設定
		 * @param pivot ピボット
		 */
		void SetPivot(const Vector2& pivot) { m_pivot = pivot; }
		/**
		 * @brief ピボットの取得
		 * @return ピボット
		 */
		const Vector2& GetPivot()const { return m_pivot; }

		/**
		 * @brief 乗算カラーの設定
		 * @param mulColor 乗算カラー
		 */
		void SetMulColor(const Vector4& mulColor) { m_sprite.SetMulColor(mulColor); }
		/**
		 * @brief 乗算カラーの取得
		 * @return 乗算カラー
		 */
		const Vector4& GetMulColor()const { return m_sprite.GetMulColor(); }
		

		// ---------------------------------------------
		// ゲージ範囲の指定
		// ---------------------------------------------


		/**
		 * @biref ゲージの描画範囲を0.0f～1.0fのの割合で指定
		 * @param startProgress ゲージの開始位置(0.0f ~ 1.0f)
		 * @param endProgress ゲージの終了位置(0.0f ~ 1.0f)
		 */
		void SetProgressRange(float startProgress, float endProgress)
		{
			m_gaugeCb.startProgress = max(0.0f, min(1.0f, startProgress));
			m_gaugeCb.endProgress = max(0.0f, min(1.0f, endProgress));
		}


		/**
		 * @brief ゲージの描画範囲を度数(°)で指定
		 * @param startDeg ゲージの開始角度(0.0f ~ 360.0f)
		 * @param endDeg ゲージの終了角度(0.0f ~ 360.0f)
		 * @details 折り返しの場合も自動で処理される
		 */
		void SetProgressRangeDeg(float startDeg, float endDeg)
		{
			m_gaugeCb.startProgress = fmodf(startDeg / 360.0f, 1.0f);
			if (m_gaugeCb.startProgress < 0.0f)m_gaugeCb.startProgress += 1.0f;
			m_gaugeCb.endProgress = fmodf(endDeg / 360.0f, 1.0f);
			if (m_gaugeCb.endProgress < 0.0f)m_gaugeCb.endProgress += 1.0f;
		}


		/**
		 * @brief ゲージをからprogressまでの通常ゲージとして設定。
		 * @param progress ゲージの割合(0.0f ~ 1.0f)
		 * @details SetProgressRange(0.0f,progress)の短縮版。
		 */
		void SetProgress(float progress)
		{
			SetProgressRange(0.0f,max(0.0f,min(1.0f,progress)));
		}


		/**
		 * @brief 現在の開始位置を取得
		 * @return ゲージの開始位置(0.0f ~ 1.0f)
		 */
		float GetStartProgress()const { return m_gaugeCb.startProgress; }
		/**
		 * @brief 現在の終了位置を取得
		 * @return ゲージの終了位置(0.0f ~ 1.0f)
		 */
		float GetEndProgress()const { return m_gaugeCb.endProgress; }


		// ----------------------------------------------
		// 円環の太さ
		// ----------------------------------------------


		/**
		 * @brief 円環の太さを中心半径 + 太さで指定する
		 * @param radius 円環の中心半径
		 * @param thickness 円環の太さ
		 */
		void SetThickness(float radius,float thickness)
		{
			m_gaugeCb.innerRadius = radius - thickness * 0.5f;
			m_gaugeCb.outerRadius = radius + thickness * 0.5f;
		}


		/**
		 * @brief 円環の内径と外径を直接指定する
		 * @param innerRadius 円環の内径
		 * @param outerRadius 円環の外径
		 */
		void SetRadius(float innerRadius, float outerRadius)
		{
			m_gaugeCb.innerRadius = innerRadius;
			m_gaugeCb.outerRadius = outerRadius;
		}


		// ----------------------------------------------
		// その他パラメーター
		// -----------------------------------------------


		/**
		 * @brief 全体の回転オフセットを設定
		 * @param angle 回転角度(ラジアン)
		 * @details
		 * デフォルトは -PI / 2(12時が開始位置)
		 */
		void SetRotationAngle(float angle){ m_gaugeCb.rotationAngle = angle; }
		/**
		 * @brief 回転オフセットを取得
		 * @return 回転角度(ラジアン)
		 */
		float GetRotationAngle()const { return m_gaugeCb.rotationAngle; }

		/**
		 * @brief ゲージ部分の色を設定
		 * @param color 色
		 */
		void SetGaugeColor(const Vector4& color) { m_gaugeCb.gaugeColor = color; }
		/**
		 * @brief ゲージ部分の色を取得
		 * @return 色
		 */
		const Vector4& GetGaugeColor()const { return m_gaugeCb.gaugeColor; }

		/**
		 * @brief 背景リング部分の色を設定
		 * @param color 色
		 */
		void SetBgColor(const Vector4& color) { m_gaugeCb.bgColor = color; }
		/**
		 * @brief 背景リング部分の色を取得
		 * @return 色
		 */
		const Vector4& GetBgColor()const { return m_gaugeCb.bgColor; }


		// -----------------------------------------------
		// 更新・描画処理
		// -----------------------------------------------

		/**
		 * @biref 更新処理
		 * @details
		 * 定数バッファの内容をGPUへ送信して、スプライトのワールド行列を更新する。
		 */
		void Update();

		/**
		 * @brief 定数バッファの更新
		 * @param gaugeCb ゲージ描画用の定数バッファ構造体
		 * @param size 定数バッファのサイズ
		 * @param slot 定数バッファのスロット番号(HLSL側で : register(b0)となっているところ)
		 */
		void UpdateConstantBuffer(GaugeConstantBuffer* gaugeCb, size_t size, int slot);

		/**
		 * @brief 描画処理
		 * @param rc レンダリングコンテキスト
		 * @details
		 * RenderingEngineの2D描画パスに登録することで、スプライトの描画とゲージのシェーダー処理が行われる。
		 */
		void Draw(RenderContext& rc);


	private:
		/**
		 * @brief 2D描画パスから呼ばれる処理
		 * @param rc レンダリングコンテキスト
		 */
		void OnRender2D(RenderContext& rc)override
		{
			m_sprite.Draw(rc);
		}


	private:
		/** スプライト本体 */
		Sprite m_sprite;
		/** ゲージ用定数バッファ */
		GaugeConstantBuffer m_gaugeCb;
		/** 位置 */
		Vector3 m_position;
		/** 大きさ */
		Vector3 m_scale;
		/** 回転 */
		Quaternion m_rotation;
		/** ピボット */
		Vector2 m_pivot;
	};
}

