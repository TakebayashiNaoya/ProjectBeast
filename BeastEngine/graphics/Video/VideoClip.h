/**
 * @file VideoClip.h
 * @brief コマ撮り／映像フレームデータ管理クラス
 * @author 竹林
 */
#pragma once
#include <vector>
#include <string>
#include <cstdint>


namespace nsBeastEngine
{
	/**
	 * @brief 映像クリップのフレームデータを管理するクラス
	 * @details
	 *   フォルダパス（末尾 / or \）→ コマ撮り（PNG/JPG/BMP/TGA）
	 *   .mp4 拡張子             → 将来対応
	 */
	class VideoClip
	{
	public:
		/**
		 * @brief クリップを読み込む
		 * @param path フォルダパス（"Assets/video/tutorial/"）または動画ファイルパス
		 * @return 読み込み成功なら true
		 */
		bool Load(const char* path);
		/**
		 * @brief 幅を取得
		 * @return 幅（ピクセル）
		 */
		inline int GetWidth() const { return m_width; }
		/**
		 * @brief 高さを取得
		 * @return 高さ（ピクセル）
		 */
		inline int GetHeight() const { return m_height; }
		/**
		 * @brief フレーム数を取得
		 * @return フレーム数
		 */
		inline int GetFrameCount() const { return static_cast<int>(m_frames.size()); }
		/**
		 * @brief FPS を取得
		 * @return FPS
		 */
		inline float GetFPS() const { return m_fps; }
		/**
		 * @brief クリップが有効か
		 * @return フレームが1枚以上あれば true
		 */
		inline bool IsValid() const { return !m_frames.empty(); }
		/**
		 * @brief FPS を上書き（コマ撮り用）
		 * @param fps FPS
		 */
		inline void SetFPS(float fps) { m_fps = fps; }
		/**
		 * @brief 指定フレームの RGBA ピクセルデータを返す
		 * @param frameIndex フレーム番号
		 * @return RGBA バイト列の先頭ポインタ、無効なら nullptr
		 */
		const uint8_t* GetFramePixels(int frameIndex) const;


	private:
		/**
		 * @brief コマ撮りフレームを読み込む
		 * @param folderPath フォルダパス（末尾 / or \）
		 */
		bool LoadFrameSequence(const std::string& folderPath);


	private:
		int   m_width = 0;		/** フレームの幅（ピクセル） */
		int   m_height = 0;		/** フレームの高さ（ピクセル） */
		float m_fps = 24.0f;	/** コマ撮りの FPS */
		std::vector<std::vector<uint8_t>> m_frames;	/** RGBA バイト列のフレームデータ */
	};
}
