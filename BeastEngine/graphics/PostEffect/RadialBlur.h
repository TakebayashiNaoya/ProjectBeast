/**
 * @file RadialBlur.h
 * @brief ラジアルブラーポストエフェクトクラス
 * @author 竹林
 */
#pragma once
#include "Graphics/PostEffect/PostEffectTypes.h"


namespace nsBeastEngine
{
	/**
	 * @brief ラジアルブラーポストエフェクト
	 * @details シロクマの咆哮など「衝撃の瞬間」に、画面中心へ向かう放射状のブラーを
	 *          短時間かける。Start() で開始し、以降は Render() が毎フレーム強度を
	 *          減衰させながら描画する。非発動中は一切描画しない（コストゼロ）。
	 * @details 同一のレンダリングターゲットを読み書きできないため、
	 *          「専用RTへブラー」→「メインRTへ書き戻し」の2パス構成にしている
	 *          （ToneMap と同じ方式）。
	 */
	class RadialBlur
	{
	public:
		RadialBlur() = default;
		~RadialBlur() = default;


	public:
		/**
		 * @brief 初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void Init(RenderTarget& mainRenderTarget);

		/**
		 * @brief ラジアルブラーを実行する
		 * @details 発動中でなければ何もしない。強度は
		 *          「立ち上がり（加速的に強くなる）→ 線形にゆっくり減衰」の
		 *          エンベロープで変化する。咆哮の溜め→叫びに同期させるための形。
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void Render(RenderContext& rc, RenderTarget& mainRenderTarget);

		/**
		 * @brief ラジアルブラーを開始する
		 * @details 発動中に呼ばれた場合は強い方を採用してやり直す。
		 * @param strength   ピーク強度（0.0〜1.0。1.0でUV空間の最大ずらし幅）
		 * @param attackTime 立ち上がりにかける時間（秒）。2乗カーブで加速的に強くなる
		 * @param duration   全体の継続時間（秒）。attackTime以降は線形に減衰する
		 */
		void Start(const float strength, const float attackTime, const float duration);


	private:
		/**
		 * @brief ラジアルブラー用の定数バッファ構造体
		 * @details HLSL側の cbuffer RadialBlurCb と一致させること
		 */
		struct SRadialBlurCb
		{
			float strength   = 0.0f;            /** UV空間での最大サンプリング距離 */
			float padding[3] = { 0.0f };
		};

		/** ブラー結果の書き込み先レンダリングターゲット */
		RenderTarget m_blurRenderTarget;
		/** ブラー用スプライト */
		Sprite m_blurSprite;
		/** メインRTへの書き戻し用スプライト */
		Sprite m_copyBackSprite;
		/** ラジアルブラー用定数バッファデータ */
		SRadialBlurCb m_cb;

		float m_elapsedTime    = 0.0f;   /** 発動からの経過時間（秒）           */
		float m_attackTime     = 0.0f;   /** 立ち上がりにかける時間（秒）       */
		float m_duration       = 0.0f;   /** 全体の継続時間（秒）。0なら非発動  */
		float m_peakStrength   = 0.0f;   /** ピーク強度（0.0〜1.0）             */
		bool  m_isInitialized  = false;  /** リソースを初期化済みか             */
	};

} // namespace nsBeastEngine
