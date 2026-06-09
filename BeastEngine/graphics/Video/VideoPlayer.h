/**
 * @file VideoPlayer.h
 * @brief 映像再生制御クラス
 * @author 竹林
 */
#pragma once
#include <functional>


namespace nsBeastEngine
{
	class VideoClip;

	/**
	 * @brief 映像の再生状態を管理するクラス
	 * @details
	 *   フレームタイマーを進めて現在フレームを管理する。
	 *   実際の描画・テクスチャ更新は VideoRender が行う。
	 */
	class VideoPlayer
	{
	public:
		/**
		 * @brief 再生するクリップの設定
		 * @param clip VideoClip へのポインタ（nullptr でクリップなし）
		 */
		void SetClip(VideoClip* clip);
		/**
		 * @brief 再生開始
		 */
		void Play();
		/**
		 * @brief 再生一時停止
		 */
		void Pause();
		/**
		 * @brief 再生停止（先頭に戻る）
		 */
		void Stop();
		/**
		 * @brief 毎フレーム呼び出す更新処理
		 * @param deltaTime 経過時間（秒）
		 */
		void Update(float deltaTime);
		/**
		 * @brief ループ再生の設定
		 * @param loop ループ再生する場合は true
		 */
		void SetLoop(bool loop) { m_loop = loop; }
		/**
		 * @brief 再生速度の設定（1.0 = 等速）
		 * @param speed 再生速度倍率
		 */
		void SetPlaybackSpeed(float speed) { m_speed = speed; }
		/**
		 * @brief 現在のフレームインデックスを取得
		 * @return フレームインデックス（0 からフレーム数 - 1）
		 */
		int GetCurrentFrameIndex() const { return m_currentFrame; }
		/**
		 * @brief 現在の再生状態を取得
		 * @return 再生中なら true
		 */
		bool IsPlaying() const { return m_isPlaying; }
		/**
		 * @brief 再生終了状態を取得
		 * @return 再生終了しているなら true
		 */
		bool IsFinished() const { return m_isFinished; }

		/** @brief 再生終了時コールバック */
		std::function<void()> m_onFinished;


	private:
		VideoClip* m_clip = nullptr;     /** 再生するクリップへのポインタ（nullptr でクリップなし） */
		float      m_elapsed = 0.0f;     /** フレームタイマーの経過時間（秒） */
		int        m_currentFrame = 0;   /** 現在のフレームインデックス（0 からフレーム数 - 1） */
		bool       m_isPlaying = false;  /** 再生中フラグ */
		bool       m_isFinished = false; /** 再生終了フラグ */
		bool       m_loop = true;        /** ループ再生するかどうか */
		float      m_speed = 1.0f;       /** 再生速度倍率（1.0 = 等速） */
	};
}
