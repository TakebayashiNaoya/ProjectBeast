/**
 * @file VideoFrameTexture.h
 * @brief CPU→GPU の動的テクスチャ更新クラス
 * @author 竹林
 */
#pragma once
#include <cstdint>


namespace nsBeastEngine
{
	/**
	 * @brief フレームごとにピクセルデータを GPU テクスチャに転送するクラス
	 * @details
	 *   DEFAULT ヒープ（GPU 読み取り用）と UPLOAD ヒープ（CPU 書き込み用）の
	 *   2 バッファ構成で CopyTextureRegion を使って更新する。
	 *   k2EngineLow::Texture::InitFromD3DResource() でラップし、
	 *   既存の Sprite の SpriteInitData::m_textures に渡せる。
	 */
	class VideoFrameTexture
	{
	public:
		VideoFrameTexture() = default;
		~VideoFrameTexture();

		/**
		 * @brief 初期化（GPU リソース確保 + 同期的な初回アップロード）
		 * @param width  テクスチャ幅
		 * @param height テクスチャ高さ
		 */
		void Init(int width, int height);
		/**
		 * @brief フレームデータを GPU テクスチャへ転送する
		 * @param rgbaPixels  RGBA 各 1 バイト、width * height * 4 バイトのバッファ
		 * @details
		 *   OnRender2D() 内（コマンドリスト記録中）から呼ぶこと。
		 *   内部で PIXEL_SHADER_RESOURCE → COPY_DEST → PIXEL_SHADER_RESOURCE のバリアを発行する。
		 */
		void UploadFrame(const uint8_t* rgbaPixels);
		/**
		 * @brief Sprite の SpriteInitData::m_textures に渡す k2EngineLow::Texture 参照
		 * @returns k2EngineLow::Texture への参照
		 */
		nsK2EngineLow::Texture& GetK2Texture() { return m_k2Texture; }
		/**
		 * @brief 初期化完了しているか
		 * @returns 初期化完了している場合は true
		 */
		inline bool IsInitialized() const { return m_isInitialized; }


	private:
		ID3D12Resource* m_gpuTexture = nullptr;					/** GPU 用テクスチャリソース（DEFAULT ヒープ） */
		ID3D12Resource* m_uploadBuffer = nullptr;				/** CPU→GPU 転送用バッファ（UPLOAD ヒープ） */
		nsK2EngineLow::Texture m_k2Texture;						/** k2EngineLow 側から参照するテクスチャラッパー */
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_footprint = {};	/** GetCopyableFootprints で取得したレイアウト情報 */
		int  m_width = 0;				/** テクスチャ幅（ピクセル） */
		int  m_height = 0;				/** テクスチャ高さ（ピクセル） */
		bool m_isInitialized = false;	/** 初期化完了フラグ */
	};
}
