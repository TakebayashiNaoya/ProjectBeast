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
		 * @brief 基点の設定
		 * @param pivot	基点
		 */
		void SetPivot(const Vector2& pivot)
		{
			m_pivot = pivot;
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
}

