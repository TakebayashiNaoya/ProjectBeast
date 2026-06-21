/**
 * @file FontRender.h
 * @brief フォントレンダークラス
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	class FontRender :public IRenderer
	{
	public:
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
			m_scale = { scale, scale };
		}

		void SetScale(const Vector2& scale)
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
		 * @brief 影のパラメーターを設定
		 * @param enable  影を描画するか
		 * @param offset  影のオフセット (ピクセル)
		 * @param color   影の色
		 */
		void SetShadowParam(bool enable, float offset, const Vector4& color)
		{
			m_isDrawShadow = enable;
			m_shadowOffset = offset;
			m_shadowColor = color;
		}

		/**
		 * @brief 水平アライメントを設定
		 * @param align Left / Center / Right
		 */
		void SetTextAlign(TextAlign align) { m_textAlign = align; }

		/**
		 * @brief 行間倍率を設定
		 * @param lineSpacing 1.0 = 隙間なし、1.2 = 1.2倍 (デフォルト)
		 */
		void SetLineSpacing(float lineSpacing) { m_lineSpacing = lineSpacing; }


	public:
		static const int MAX_TEXT_SIZE = 256;

		FontRender()
			: m_position(Vector3::Zero)
			, m_rotation(0.0f)
			, m_scale(1.0f, 1.0f)
			, m_pivot(Sprite::DEFAULT_PIVOT)
			, m_text(L"")
			, m_color(Vector4::White)
			, m_isDrawShadow(false)
			, m_shadowOffset(0.0f)
			, m_shadowColor({ 0.0f,0.0f,0.0f,0.0f })
			, m_textAlign(TextAlign::Left)
			, m_lineSpacing(1.2f)
		{}
		~FontRender() = default;

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
			auto& sdfFont = GetSDFFontEngine();
			sdfFont.SetShadowParam(m_isDrawShadow, m_shadowOffset, m_shadowColor);
			sdfFont.SetTextAlign(m_textAlign);
			sdfFont.SetLineSpacing(m_lineSpacing);
			sdfFont.BeginDraw(rc, m_rotation, Vector2(m_position.x, m_position.y));
			sdfFont.Draw(m_text, Vector2(m_position.x, m_position.y), m_color, m_rotation, m_scale, m_pivot);
			sdfFont.EndDraw(rc);
		}


	private:
		Vector3	m_position;				/** 位置 */
		float	m_rotation;				/** 回転 */
		Vector2	m_scale;				/** 大きさ (x=横幅, y=縦幅) */
		Vector2	m_pivot;				/** 基点 */
		wchar_t	m_text[MAX_TEXT_SIZE];	/** 描画する文字列 */
		Vector4	m_color;				/** 色 */
		bool      m_isDrawShadow;			/** 影を描画するか */
		float     m_shadowOffset;			/** 影のオフセット */
		Vector4   m_shadowColor;			/** 影の色 */
		TextAlign m_textAlign;				/** 水平アライメント */
		float     m_lineSpacing;			/** 行間倍率 */
	};
}