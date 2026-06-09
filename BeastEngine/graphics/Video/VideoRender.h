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
		 * @brief VideoPlayer へのアクセス（再生制御や状態の取得に）
		 */
		void Play() { m_player.Play(); }
		void Pause() { m_player.Pause(); }
		void Stop() { m_player.Stop(); }
		void SetLoop(bool loop) { m_player.SetLoop(loop); }
		void SetPlaybackSpeed(float speed) { m_player.SetPlaybackSpeed(speed); }
		void SetOnFinished(std::function<void()> cb) { m_player.onFinished = cb; }
		bool IsPlaying()     const { return m_player.IsPlaying(); }
		bool IsFinished()    const { return m_player.IsFinished(); }
		bool IsInitialized() const { return m_isInitialized; }

		/**
		 * @brief 描画パラメータの設定
		 */
		void SetPosition(const Vector3& pos) { m_position = pos; }
		void SetScale(const Vector3& scale) { m_scale = scale; }
		void SetRotation(const Quaternion& rot) { m_rotation = rot; }
		void SetPivot(const Vector2& pivot) { m_pivot = pivot; }
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


	private:
		VideoClip         m_clip;		/** 再生する映像クリップ */
		VideoPlayer       m_player;		/** プレイヤー */
		VideoFrameTexture m_frameTex;	/** GPU テクスチャ管理 */
		Sprite            m_sprite;		/** 描画用スプライト */

		Vector3    m_position = Vector3::Zero;			/** 座標 */
		Vector3    m_scale = Vector3::One;				/** 拡大率 */
		Quaternion m_rotation = Quaternion::Identity;	/** 回転 */
		Vector2    m_pivot = Sprite::DEFAULT_PIVOT;		/** ピボット */
		Vector4    m_mulColor = Vector4::White;			/** 乗算カラー */

		/** 前回描画したフレーム番号（変化した時だけ GPU 転送） */
		int  m_lastFrameIdx = -1;		/** フレーム番号は 0 から始まるため、-1 で「まだ描画していない」状態を表す */
		bool m_isInitialized = false;	/** 初期化完了フラグ */
	};
}
