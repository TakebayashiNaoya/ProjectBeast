/**
 * @file GaussianBlur.h
 * @brief ガウシアンブラー・平均ブラーの独自実装
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/PostEffect/PostEffectTypes.h"


namespace nsBeastEngine
{
	/**
	 * @brief ブラー処理クラス
	 * @details EnBlurType により平均ブラーとガウシアンブラーを切り替える。
	 *          横方向・縦方向の2パスで縮小バッファに書き込む。
	 */
	class GaussianBlur
	{
	public:
		/**
		 * @brief ガウシアンブラーの重みテーブルの要素数
		 */
		static constexpr int NUM_WEIGHTS = 8;


	public:
		GaussianBlur() = default;
		~GaussianBlur() = default;


	public:
		/**
		 * @brief 初期化
		 * @param srcTexture ブラーをかける入力テクスチャ
		 * @param blurType   ブラーの種別
		 * @param blurWidth  横ブラー用RTの幅（入力の半分が基本）
		 * @param blurHeight 縦ブラー用RTの高さ（入力の半分が基本）
		 * @param sigma      ガウシアンブラーの強さ（平均ブラー時は無視）
		 */
		void Init(
			Texture* srcTexture,
			EnBlurType blurType,
			int blurWidth,
			int blurHeight,
			float sigma
		);

		/**
		 * @brief GPUでブラーを実行する
		 * @param rc レンダリングコンテキスト
		 */
		void ExecuteOnGPU(RenderContext& rc);

		/**
		 * @brief ボケテクスチャを取得する
		 * @return ブラー済みのテクスチャ
		 */
		Texture& GetBokeTexture()
		{
			return m_yBlurRenderTarget.GetRenderTargetTexture();
		}


	private:
		/**
		 * @brief ガウシアン関数で重みテーブルを計算する
		 * @param sigma 分散の強さ
		 */
		void CalcWeightsTableFromGaussian(float sigma);

		/**
		 * @brief 平均ブラーの重みテーブルを設定する
		 */
		void SetAverageWeightsTable();

		/**
		 * @brief 横ブラー用スプライトの初期化
		 * @param srcTexture 入力テクスチャ
		 */
		void InitXBlurSprite(Texture* srcTexture);

		/**
		 * @brief 縦ブラー用スプライトの初期化
		 */
		void InitYBlurSprite();


	private:
		/**
		 * @brief ブラー用の定数バッファ構造体
		 * @details HLSL側の cbuffer BlurCb と一致させること。
		 *          HLSLのcbuffer内のfloat配列は各要素が16バイト境界に
		 *          アライメントされるため、Vector4[2]で渡す必要がある。
		 *          weights[0].xyzw = weights[0]〜[3]
		 *          weights[1].xyzw = weights[4]〜[7]
		 */
		struct SBlurCb
		{
			Vector4 weights[2]; /** 重みテーブル（float4×2 = 8サンプル分） */
		};

		/** 横ブラー用レンダリングターゲット */
		RenderTarget m_xBlurRenderTarget;
		/** 縦ブラー用レンダリングターゲット */
		RenderTarget m_yBlurRenderTarget;
		/** 横ブラー用スプライト */
		Sprite m_xBlurSprite;
		/** 縦ブラー用スプライト */
		Sprite m_yBlurSprite;
		/** ブラー用定数バッファデータ */
		SBlurCb m_blurCb;
	};

} // namespace nsBeastEngine