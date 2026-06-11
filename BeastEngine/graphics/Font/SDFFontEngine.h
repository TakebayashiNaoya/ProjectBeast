/**
 * @file SDFFontEngine.h
 * @brief SDF フォント描画エンジンのクラス定義
 * @author 竹林
 */
#pragma once
#include <unordered_map>

namespace nsBeastEngine
{
	/**
	 * @brief msdf-atlas-gen が出力する1グリフ分のレイアウト情報
	 */
	struct SDFGlyph
	{
		float advance;
		// planeBounds: ベースライン原点・emスケールでのクワッド範囲 (Y上が正)
		float planeLeft, planeBottom, planeRight, planeTop;
		// atlasBounds: アトラステクスチャ上のピクセル座標 (yOrigin:bottom 前提)
		float atlasLeft, atlasBottom, atlasRight, atlasTop;
	};

	/**
	 * @brief SDF フォント描画エンジン
	 * @details
	 *   msdf-atlas-gen が生成した PNG アトラスと JSON を読み込み、
	 *   SpriteBatch + カスタムピクセルシェーダーで任意スケールに対応した
	 *   鮮明なフォント描画を行う。
	 *
	 *   GetSDFFontEngine() でシングルトンにアクセスする。
	 *   FontRender が OnRender2D で直接呼び出す。
	 *
	 * アセットパス (変更する場合はクラス内定数を編集):
	 *   - Assets/font/sdf_atlas.png  : SDF アトラス画像
	 *   - Assets/font/sdf_atlas.json : グリフメタデータ (msdf-atlas-gen 出力)
	 *   - Assets/shader/SDFFont.fx   : SDF ピクセルシェーダー
	 */
	class SDFFontEngine : public nsK2EngineLow::Noncopyable
	{
	public:
		~SDFFontEngine();

		/**
		 * @brief 初期化 (DirectX 初期化後に一度だけ呼ぶ)
		 */
		void Init();

		/**
		 * @brief 描画開始 (フレーム内で SpriteBatch::Begin を呼ぶ)
		 * @param rotation      Z軸回転 (ラジアン)。0 で無回転。
		 * @param gameSpacePos  回転の中心となるゲーム座標。FontRender の位置を渡す。
		 */
		void BeginDraw(RenderContext& rc, float rotation = 0.0f, Vector2 gameSpacePos = Vector2(0.0f, 0.0f));

		/**
		 * @brief テキスト描画リクエスト
		 * @param text      描画するワイド文字列
		 * @param position  中心原点座標系での位置 (Font.cpp と同じ座標系)
		 * @param color     RGBA カラー
		 * @param rotation  回転 (BeginDraw に渡した値と同じ値を渡すこと)
		 * @param scale     スケール倍率
		 * @param pivot     ピボット (0,0=左上 / 0.5,0.5=中央 / 1,1=右下)
		 */
		void Draw(
			const wchar_t* text,
			const Vector2& position,
			const Vector4& color,
			float           rotation,
			Vector2         scale,
			Vector2         pivot
		);

		/**
		 * @brief 描画終了 (SpriteBatch::End を呼ぶ)
		 */
		void EndDraw(RenderContext& rc);

		/**
		 * @brief 影のパラメーターを設定
		 */
		void SetShadowParam(bool enable, float offset, const Vector4& color)
		{
			m_isDrawShadow = enable;
			m_shadowOffset = offset;
			m_shadowColor = color;
		}


	private:
		/**
		 * @brief 実際に SpriteBatch::Draw を呼ぶ内部実装
		 */
		void DrawString(
			const wchar_t* text,
			const Vector2& screenPos,
			const Vector4& color,
			Vector2         scale,
			Vector2         pivot
		);

		/**
		 * @brief PNG アトラスと JSON グリフデータを読み込む
		 */
		bool LoadAtlas();


	private:
		/** アセットパス(変更する場合はここを編集) */
		static constexpr const wchar_t* ATLAS_PNG_PATH = L"Assets/font/sdf_atlas.png";
		static constexpr const char* ATLAS_JSON_PATH = "Assets/font/sdf_atlas.json";
		static constexpr const char* SHADER_PATH = "Assets/shader/SDFFont.fx";

		/** アトラステクスチャとグリフデータを読み込んで、SRV を作成する */
		std::unique_ptr<DirectX::SpriteBatch>       m_spriteBatch;
		Microsoft::WRL::ComPtr<ID3D12Resource>		m_sdfTexture;
		ID3D12DescriptorHeap* m_srvHeap = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE                 m_gpuHandle = {};		/** SRV の GPU ハンドル */
		UINT                                        m_atlasWidth = 0;		/** 幅 (ピクセル) */
		UINT                                        m_atlasHeight = 0;		/** 高さ (ピクセル) */
		float                                       m_emSize = 48.0f;		/** em スケール (ピクセル) */
		float                                       m_lineHeight = 1.0f;	/** 行の高さ (em スケール) */
		float                                       m_ascender = 0.8f;		/** ベースラインから上端までの距離 (em スケール) */
		std::unordered_map<uint32_t, SDFGlyph>      m_glyphs;				/** Unicode コードポイントをキーとするグリフデータのマップ */

		nsK2EngineLow::Shader                       m_psShader;				/** ピクセルシェーダー (SDFFont.fx) */

		bool    m_isDrawShadow = false;						/** 影を描画するかどうか */
		float   m_shadowOffset = 0.0f;						/** 影のオフセット (em スケール) */
		Vector4 m_shadowColor = { 0.0f, 0.0f, 0.0f, 0.0f };	/** 影のカラー */
	};

	/**
	 * @brief SDFFontEngine のシングルトンを返す
	 * @details 初回呼び出し時に Init() を実行する
	 */
	SDFFontEngine& GetSDFFontEngine();

} // namespace nsBeastEngine
