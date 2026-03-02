/**
 * @file SpriteRender.h
 * @brief フォントレンダークラス
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	class FontRender :public IRenderer
	{
	public:
		static const int MAX_TEXT_SIZE = 256;

		FontRender()
			: m_position(Vector3::Zero)
			, m_rotation(0.0f)
			, m_scale(1.0f)
			, m_pivot(Sprite::DEFAULT_PIVOT)
			, m_text(L"")
			, m_color(Vector4::White)
		{
		}
		~FontRender() = default;

		/**
		 * @brief 文字の設定
		 * @param text 文字列
		 */
		void SetText(const wchar_t* text)
		{
			swprintf_s(m_text, text);
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
		void SetRotation(const float rotation)
		{
			m_rotation = rotation;
		}

		/**
		 * @brief 大きさの設定
		 * @param scale	大きさ
		 */
		void SetScale(const float scale)
		{
			m_scale = scale;
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
		 * @brief 色の設定
		 * @param red	赤成分
		 * @param green	緑成分
		 * @param blue	青成分
		 * @param alpha	アルファ成分
		 */
		void SetColor(float red, float green, float blue, float alpha)
		{
			SetColor({ red, green, blue, alpha });
		}
		/**
		 * @brief 色の設定
		 * @param color 色
		 */
		void SetColor(const Vector4& color)
		{
			m_color = color;
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
			m_font.Begin(rc);
			m_font.Draw(m_text, Vector2(m_position.x, m_position.y), m_color, m_rotation, m_scale, m_pivot);
			m_font.End(rc);
		}


	private:
		/** フォント */
		Font		m_font;
		/** 位置 */
		Vector3		m_position;
		/** 回転 */
		float		m_rotation;
		/** 大きさ */
		float		m_scale;
		/** 基点 */
		Vector2		m_pivot;
		/** 描画する文字列 */
		wchar_t		m_text[MAX_TEXT_SIZE];
		/** 色 */
		Vector4		m_color;
	};
}

