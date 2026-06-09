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
	class VideoClip
	{
	public:
		VideoClip() = default;
		~VideoClip();
		/**
		 * @brief 動画ファイルパス（例: "C:/video.mp4"）またはコマ撮りフォルダパス（例 : "C:/frames/"）を指定してロード
		 * @param path 動画ファイルパスまたはコマ撮りフォルダパス
		 */
		bool Load(const char* path);
		/**
		 * @brief 幅を取得
		 * @return 動画の幅（ピクセル）
		 */
		int GetWidth() const { return m_width; }
		/**
		 * @brief 高さを取得
		 * @return 動画の高さ（ピクセル）
		 */
		int GetHeight() const { return m_height; }
		/**
		 * @brief フレーム数を取得
		 * @return 動画のフレーム数
		 */
		int GetFrameCount() const { return m_frameCount; }
		/**
		 * @brief フレームレートを取得
		 * @return 動画のフレームレート（fps）
		 */
		float GetFPS() const { return m_fps; }
		/**
		 * @brief 動画が有効にロードされているか
		 * @return フレーム数が1以上ならtrue、それ以外はfalse
		 */
		bool IsValid() const { return m_frameCount > 0; }
		/**
		 * @brief フレームレートを設定（MP4の場合は無視される）
		 * @param fps フレームレート（fps）
		 */
		void SetFPS(float fps) { m_fps = fps; }
		/**
		 * @brief 指定したフレームのピクセルデータを取得
		 * @param frameIndex フレームインデックス（0からframeCount-1の範囲）
		 * @return RGBA32形式のピクセルデータへのポインタ。インデックスが範囲外の場合はnullptr。
		 */
		const uint8_t* GetFramePixels(int frameIndex) const;


	private:
		/**
		 * @brief 動画の種類
		 */
		enum class ClipType
		{
			None = 0,      /** 無効な状態 */
			FrameSequence, /** コマ撮りフレームシーケンス */
			MP4            /** MP4動画ファイル */
		};
		/**
		 * @brief コマ撮りフレームシーケンスをロード
		 * @param folderPath コマ撮りフレームが格納されたフォルダのパス
		 */
		bool LoadFrameSequence(const std::string& folderPath);
		/**
		 * @brief MP4動画ファイルをロード
		 * @param filePath MP4動画ファイルのパス
		 */
		bool LoadMP4(const std::string& filePath);
		/**
		 * @brief MP4関連のリソースを解放
		 */
		void CleanupMP4();
		/**
		 * @brief MP4動画から次のフレームをデコードしてピクセルデータを更新
		 * @return デコード成功ならtrue、動画の終端に達したかエラーが発生した場合はfalse
		 */
		bool ReadNextMP4Frame() const;
		/**
		 * @brief MP4動画の再生位置を先頭にリセット
		 * @return 成功ならtrue、失敗した場合はfalse
		 */
		bool SeekMP4ToBeginning() const;

		int       m_width = 0;                 /** 動画の幅（ピクセル） */
		int       m_height = 0;                /** 動画の高さ（ピクセル） */
		int       m_frameCount = 0;            /** 動画のフレーム数 */
		float     m_fps = 24.0f;               /** 動画のフレームレート（fps） */
		ClipType  m_clipType = ClipType::None; /** 動画の種類 */
		/** コマ撮り: 全フレームを事前ロード */
		std::vector<std::vector<uint8_t>> m_frames;
		/**
		 * MP4: Media Foundation によるストリーミングデコード
		 * IMFSourceReader* を格納（void* で宣言しているのは、ヘッダーファイルで Windows.h をインクルードせずに済ませるため）
		 */
		mutable void* m_mfReader = nullptr;
		mutable std::vector<uint8_t> m_mp4FrameBuffer;       /** MP4の現在フレームのピクセルデータを格納するバッファ */
		mutable int                  m_mp4CurrentFrame = -1; /** MP4の現在フレームインデックス（0からframeCount-1の範囲） */
		mutable bool                 m_mp4Eos = false;       /** MP4の終端に達したかどうか */
		mutable bool                 m_coInitialized = false; /** COMが初期化されているかどうか */
	};
}
