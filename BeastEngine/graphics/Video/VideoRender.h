/**
 * @file VideoRender.h
 * @brief 映像を IRenderer として 2D パスで描画するクラス
 * @author 竹林
 */
#pragma once
#include "VideoClip.h"
#include "VideoPlayer.h"
#include "VideoFrameTexture.h"


namespace nsBeastEngine
{
	/**
	 * @brief 映像クリップを 2D スプライトとして描画する IRenderer 実装
	 * @details
	 *   UIVideo から保持され、Layout / Menu から位置・スケール・色を制御できる。
	 *   フレームが変化した時のみ VideoFrameTexture::UploadFrame() を呼ぶため、
	 *   不要な GPU 転送が発生しない。
	 */
	class VideoRender : public IRenderer
	{
	public:
		/**
		 * @brief 初期化
		 * @param clipPath  フォルダパス（"Assets/video/foo/"）
		 * @param dispWidth  スプライトの表示幅（ピクセル）
		 * @param dispHeight スプライトの表示高さ（ピクセル）
		 * @param fps        コマ撮りの FPS（MP4 では自動取得）
		 */
		void Init(const char* clipPath, float dispWidth, float dispHeight, float fps = 24.0f);

		/**
		 * @brief 再生開始
		 */
		void Play() { m_player.Play(); }
		/**
		 * @brief 再生一時停止
		 */
		void Pause() { m_player.Pause(); }
		/**
		 * @brief 再生停止（先頭に戻る）
		 */
		void Stop() { m_player.Stop(); }
		/**
		 * @brief ループ再生の設定
		 * @param loop ループ再生する場合は true
		 */
		void SetLoop(bool loop) { m_player.SetLoop(loop); }
		/**
		 * @brief 再生速度の設定（1.0 = 等速）
		 * @param speed 再生速度倍率
		 */
		void SetPlaybackSpeed(float speed) { m_player.SetPlaybackSpeed(speed); }
		/**
		 * @brief 再生終了コールバックの設定
		 * @param cb 再生終了時に呼び出すコールバック
		 */
		void SetOnFinished(std::function<void()> cb) { m_player.m_onFinished = cb; }
		/**
		 * @brief 再生中かどうかを取得
		 * @return 再生中なら true
		 */
		bool IsPlaying()     const { return m_player.IsPlaying(); }
		/**
		 * @brief 再生終了しているかどうかを取得
		 * @return 再生終了しているなら true
		 */
		bool IsFinished()    const { return m_player.IsFinished(); }
		/**
		 * @brief 初期化完了しているかどうかを取得
		 * @return 初期化完了している場合は true
		 */
		bool IsInitialized() const { return m_isInitialized; }

		/**
		 * @brief 座標の設定
		 * @param pos 座標
		 */
		void SetPosition(const Vector3& pos) { m_position = pos; }
		/**
		 * @brief 拡大率の設定
		 * @param scale 拡大率
		 */
		void SetScale(const Vector3& scale) { m_scale = scale; }
		/**
		 * @brief 回転の設定
		 * @param rot 回転
		 */
		void SetRotation(const Quaternion& rot) { m_rotation = rot; }
		/**
		 * @brief ピボットの設定
		 * @param pivot ピボット
		 */
		void SetPivot(const Vector2& pivot) { m_pivot = pivot; }
		/**
		 * @brief 乗算カラーの設定
		 * @param color 乗算カラー
		 */
		void SetMulColor(const Vector4& color) { m_mulColor = color; }

		/**
		 * @brief 毎フレーム呼ぶ更新処理（UIVideo::Update() から呼ぶ）
		 */
		void Update();
		/**
		 * @brief RenderingEngine に登録する（UIVideo::Render() から呼ぶ）
		 */
		void Draw(RenderContext& rc);


	private:
		void OnRender2D(RenderContext& rc) override;

		VideoClip         m_clip;     /** 再生する映像クリップ */
		VideoPlayer       m_player;   /** プレイヤー */
		VideoFrameTexture m_frameTex; /** GPU テクスチャ管理 */
		Sprite            m_sprite;   /** 描画用スプライト */

		Vector3    m_position = Vector3::Zero;        /** 座標 */
		Vector3    m_scale = Vector3::One;             /** 拡大率 */
		Quaternion m_rotation = Quaternion::Identity; /** 回転 */
		Vector2    m_pivot = Sprite::DEFAULT_PIVOT;   /** ピボット */
		Vector4    m_mulColor = Vector4::White;        /** 乗算カラー */

		/** 前回描画したフレーム番号。-1 は「まだ描画していない」状態を表す。フレームが変化した時だけ GPU 転送する */
		int  m_lastFrameIdx = -1;
		bool m_isInitialized = false; /** 初期化完了フラグ */
	};
}
