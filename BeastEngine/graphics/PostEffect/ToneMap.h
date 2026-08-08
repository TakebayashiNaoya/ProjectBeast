/**
 * @file ToneMap.h
 * @brief トーンマップポストエフェクトクラス
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/PostEffect/PostEffectTypes.h"


namespace nsBeastEngine
{
	/**
	 * @brief トーンマップポストエフェクト
	 * @details HDRのメインレンダリングターゲットの色をLDRの範囲へ圧縮する。
	 *          同一のレンダリングターゲットを読み書きできないため、
	 *          「専用RTへトーンマップ」→「メインRTへ書き戻し」の2パス構成にしている。
	 * @details 方式ごとのスプライトを初期化時にまとめて作っておき、描画時に選ぶことで
	 *          実行中でも方式を切り替えられるようにしている。
	 *          ただしenNoneで初期化した場合はリソースを一切作らないため切り替えできない。
	 *          見比べたい場合はenNone以外で初期化してから、実行中にenNoneへ切り替えること。
	 * @details 露出などのパラメーターはSpriteが描画のたびに定数バッファを再アップロード
	 *          するため、実行中に変更すると即座に反映される。
	 */
	class ToneMap
	{
	public:
		ToneMap() = default;
		~ToneMap() = default;


	public:
		/**
		 * @brief 初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 * @param toneMapType      トーンマップの種別
		 */
		void Init(RenderTarget& mainRenderTarget, EnToneMapType toneMapType);

		/**
		 * @brief トーンマップを実行する
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void Render(RenderContext& rc, RenderTarget& mainRenderTarget);

		/**
		 * @brief トーンマップの種別に対応する表示名を取得する
		 * @param toneMapType トーンマップの種別
		 * @return 表示名
		 */
		static const char* GetTypeName(const EnToneMapType toneMapType);


		//============================================//
		// パラメーターへのアクセス（実行中の調整用）
		//============================================//

	public:
		/**
		 * @brief トーンマップの種別を設定する
		 * @details 方式ごとに適正な露出が異なるため、既定の露出も併せて適用する。
		 *          露出だけを変えたい場合はSetExposure()を後から呼ぶ。
		 * @param toneMapType トーンマップの種別
		 */
		void SetToneMapType(const EnToneMapType toneMapType);

		/**
		 * @brief トーンマップの種別を取得
		 * @return トーンマップの種別
		 */
		__forceinline EnToneMapType GetToneMapType() const { return m_toneMapType; }

		/**
		 * @brief 実行中に種別を切り替えられるか
		 * @return 切り替え可能かどうか
		 */
		__forceinline bool IsSwitchable() const { return m_isInitialized; }

		/**
		 * @brief 露出倍率を設定
		 * @param exposure 露出倍率
		 */
		__forceinline void SetExposure(const float exposure) { m_toneMapCb.exposure = exposure; }

		/**
		 * @brief 露出倍率を取得
		 * @return 露出倍率
		 */
		__forceinline float GetExposure() const { return m_toneMapCb.exposure; }

		/**
		 * @brief 白とみなす輝度を設定
		 * @param whitePoint 白とみなす輝度
		 */
		__forceinline void SetWhitePoint(const float whitePoint) { m_toneMapCb.whitePoint = whitePoint; }

		/**
		 * @brief 白とみなす輝度を取得
		 * @return 白とみなす輝度
		 */
		__forceinline float GetWhitePoint() const { return m_toneMapCb.whitePoint; }

		/**
		 * @brief sRGBエンコードを行うかを設定
		 * @param isApplyGamma sRGBエンコードを行うかどうか
		 */
		__forceinline void SetApplyGamma(const bool isApplyGamma) { m_toneMapCb.applyGamma = isApplyGamma ? 1.0f : 0.0f; }

		/**
		 * @brief sRGBエンコードを行うかを取得
		 * @return sRGBエンコードを行うかどうか
		 */
		__forceinline bool IsApplyGamma() const { return m_toneMapCb.applyGamma > 0.5f; }

		/**
		 * @brief 輝度ベースで適用するかを設定
		 * @details RGBベースは各チャンネルを独立に圧縮するため明部の色が白へ抜ける。
		 *          輝度ベースは色比を保つので彩度が残る。
		 * @param isLuminanceBased 輝度ベースで適用するかどうか
		 */
		__forceinline void SetLuminanceBased(const bool isLuminanceBased) { m_toneMapCb.isLuminanceBased = isLuminanceBased ? 1.0f : 0.0f; }

		/**
		 * @brief 輝度ベースで適用するかを取得
		 * @return 輝度ベースで適用するかどうか
		 */
		__forceinline bool IsLuminanceBased() const { return m_toneMapCb.isLuminanceBased > 0.5f; }


	private:
		/**
		 * @brief トーンマップ結果を書き込むレンダリングターゲットの初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitToneMapRenderTarget(RenderTarget& mainRenderTarget);

		/**
		 * @brief 全方式のトーンマップ用スプライトを初期化する
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitToneMapSprites(RenderTarget& mainRenderTarget);

		/**
		 * @brief メインRTへ書き戻すためのスプライトの初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void InitCopyBackSprite(RenderTarget& mainRenderTarget);

		/**
		 * @brief トーンマップパスを実行する
		 * @param rc レンダリングコンテキスト
		 */
		void RenderToneMap(RenderContext& rc);

		/**
		 * @brief メインRTへの書き戻しパスを実行する
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void RenderCopyBack(RenderContext& rc, RenderTarget& mainRenderTarget);


	private:
		/**
		 * @brief トーンマップ用の定数バッファ構造体
		 * @details HLSL側の cbuffer ToneMapCb と一致させること
		 */
		struct SToneMapCb
		{
			float exposure         = 1.0f;   /** 露出倍率                                     */
			float whitePoint       = 4.0f;   /** 白とみなす輝度（enReinhardExtendedのみ）     */
			float applyGamma       = 0.0f;   /** sRGBエンコードを行うか（0:しない 1:する）    */
			float isLuminanceBased = 1.0f;   /** 輝度ベースで適用するか（0:RGB 1:輝度）       */
		};

		/** トーンマップの種別 */
		EnToneMapType m_toneMapType = EnToneMapType::enNone;
		/** リソースを初期化済みか（enNoneで初期化した場合はfalseのまま） */
		bool m_isInitialized = false;

		/** トーンマップ結果の書き込み先レンダリングターゲット */
		RenderTarget m_toneMapRenderTarget;
		/** 方式ごとのトーンマップ用スプライト（EnToneMapTypeで添字を引く） */
		std::array<Sprite, static_cast<size_t>(EnToneMapType::enNum)> m_toneMapSprites;
		/** メインRTへの書き戻し用スプライト */
		Sprite m_copyBackSprite;
		/** トーンマップ用定数バッファデータ */
		SToneMapCb m_toneMapCb;
	};

} // namespace nsBeastEngine
