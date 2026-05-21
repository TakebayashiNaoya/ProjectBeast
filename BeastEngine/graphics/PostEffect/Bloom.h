/**
 * @file Bloom.h
 * @brief ブルームポストエフェクトクラス
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/PostEffect/PostEffectTypes.h"
#include "Graphics/PostEffect/GaussianBlur.h"


namespace nsBeastEngine
{
	/**
	 * @brief ブルームポストエフェクト
	 * @details 輝度抽出 → ブラー → メインRTへの加算合成 を行う。
	 *          3D描画完了後・UI描画前に実行することで、UIへの影響を防ぐ。
	 *          EnBloomType により通常ブルームと川瀬式ブルームを切り替える。
	 *          EnBlurType  により平均ブラーとガウシアンブラーを切り替える。
	 *          パラメーター調整は Bloom.cpp 内の constexpr 定数で行う。
	 */
	class Bloom
	{
	public:
		/**
		 * @brief 川瀬式ブルームの縮小バッファの最大数
		 */
		static constexpr int MAX_KAWASE_BUFFERS = 4;


	public:
		Bloom() = default;
		~Bloom() = default;


	public:
		/**
		 * @brief 初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 * @param bloomType        ブルームの種別
		 * @param blurType         ブラーの種別
		 */
		void Init(
			RenderTarget& mainRenderTarget,
			EnBloomType bloomType,
			EnBlurType blurType
		);

		/**
		 * @brief ブルームを実行する
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void Render(RenderContext& rc, RenderTarget& mainRenderTarget);


	private:
		/**
		 * @brief 輝度抽出用レンダリングターゲットの初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitLuminanceRenderTarget(RenderTarget& mainRenderTarget);

		/**
		 * @brief 輝度抽出用スプライトの初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitLuminanceSprite(RenderTarget& mainRenderTarget);

		/**
		 * @brief 通常ブルーム用の合成スプライトの初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitNormalBloomFinalSprite(RenderTarget& mainRenderTarget);

		/**
		 * @brief 川瀬式ブルーム用の合成スプライトの初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitKawaseBloomFinalSprite(RenderTarget& mainRenderTarget);

		/**
		 * @brief 輝度抽出パスを実行する
		 * @param rc レンダリングコンテキスト
		 */
		void RenderLuminance(RenderContext& rc);

		/**
		 * @brief ブラーパスを実行する
		 * @param rc レンダリングコンテキスト
		 */
		void RenderBlur(RenderContext& rc);

		/**
		 * @brief メインRTへの加算合成パスを実行する
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void RenderFinal(RenderContext& rc, RenderTarget& mainRenderTarget);


	private:
		/**
		 * @brief 輝度抽出用の定数バッファ構造体
		 * @details HLSL側の cbuffer LuminanceCb と一致させること
		 */
		struct SLuminanceCb
		{
			float luminanceThreshold = 1.0f; /** 輝度抽出のしきい値 */
			float pad[3] = {};   /** パディング         */
		};

		/**
		 * @brief ブルーム合成用の定数バッファ構造体
		 * @details HLSL側の cbuffer BloomFinalCb と一致させること
		 */
		struct SBloomFinalCb
		{
			float bloomIntensity = 1.0f; /** ブルームの強度 */
			float pad[3] = {};   /** パディング     */
		};

		/** ブルームの種別 */
		EnBloomType m_bloomType = EnBloomType::enNone;
		/** ブラーの種別 */
		EnBlurType m_blurType = EnBlurType::enGaussian;

		/** 輝度抽出用レンダリングターゲット */
		RenderTarget m_luminanceRenderTarget;
		/** 輝度抽出用スプライト */
		Sprite m_luminanceSprite;
		/** 輝度抽出用定数バッファデータ */
		SLuminanceCb m_luminanceCb;

		/** ブルーム合成用定数バッファデータ */
		SBloomFinalCb m_bloomFinalCb;

		/**
		 * @brief 川瀬式ブルームの縮小バッファ数
		 * @details 1〜MAX_KAWASE_BUFFERS の範囲で設定する。
		 *          Bloom.cpp の KAWASE_NUM_BUFFERS で変更する。
		 */
		int m_numKawaseBuffers = MAX_KAWASE_BUFFERS;

		/**
		 * @brief ガウシアンブラーの配列
		 * @details 通常ブルームは[0]のみ使用。
		 *          川瀬式は[0]〜[m_numKawaseBuffers-1]を使用する。
		 */
		std::array<GaussianBlur, MAX_KAWASE_BUFFERS> m_gaussianBlurs;

		/** ブルーム加算合成用スプライト */
		Sprite m_finalSprite;
	};

} // namespace nsBeastEngine