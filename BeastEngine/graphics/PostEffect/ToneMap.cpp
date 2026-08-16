/**
 * @file ToneMap.cpp
 * @brief トーンマップポストエフェクトクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/PostEffect/ToneMap.h"


namespace nsBeastEngine
{
	namespace
	{
		//============================================//
		// トーンマップパラメーター
		// 調整する場合はここの値を変更する
		//============================================//

		/** 種別の数（配列サイズ用） */
		constexpr size_t TONE_MAP_TYPE_NUM = static_cast<size_t>(EnToneMapType::enNum);

		/**
		 * @brief 方式ごとの既定の露出倍率
		 * @details 圧縮カーブが方式ごとに違うため、適正な露出も方式ごとに変わる。
		 *          トーンマップ無しの平均輝度を基準に実測して求めた値。
		 *          SetToneMapType()で方式を切り替えたときにこの値が適用される。
		 */
		constexpr float DEFAULT_EXPOSURES[TONE_MAP_TYPE_NUM] = {
			1.0f,  /** enNone             未使用                                    */
			1.0f,  /** enExposure         カーブが無いため上げるとすぐ飽和する      */
			2.6f,  /** enReinhard         全域を圧縮するぶん高めが必要              */
			2.2f,  /** enReinhardExtended トーンマップ無しとほぼ同じ明るさになる    */
			1.0f,  /** enACES             中間調を大きく持ち上げるため低め          */
			2.5f,  /** enUncharted2       Reinhardよりやや低めで足りる              */
		};

		/**
		 * @brief 白とみなす輝度の初期値
		 * @details enReinhardExtended でのみ使用する。
		 *          この値のHDR輝度がちょうど純白になる。下げるほど早く白へ到達する。
		 */
		constexpr float INITIAL_WHITE_POINT = 4.0f;

		/**
		 * @brief sRGBエンコード（ガンマ補正）を行うかどうかの初期値
		 * @details 現状バックバッファは R8G8B8A8_UNORM（非sRGB）で、
		 *          リニア色をエンコードせずそのまま出力している。
		 *          true にすると物理的に正しくなるが画面全体が大きく明るくなり、
		 *          ライト強度とマテリアルの再調整が必要になるため既定では false。
		 */
		constexpr bool INITIAL_IS_APPLY_GAMMA = false;

		/**
		 * @brief 輝度ベースで適用するかどうかの初期値
		 * @details RGBベースは各チャンネルを独立に圧縮するため、明るいチャンネルほど
		 *          強く潰れて3チャンネルが互いに近づき、明部の色が白へ抜ける
		 *          （実測でReinhardの明部の彩度が0.001まで低下した）。
		 *          輝度ベースは色比を保ったまま輝度だけを圧縮するので彩度が残るため、
		 *          既定はこちらにしている。
		 */
		constexpr bool INITIAL_IS_LUMINANCE_BASED = true;

		/** トーンマップシェーダーのファイルパス */
		constexpr const char* TONE_MAP_FX_PATH = "Assets/shader/PostEffect/toneMap.fx";

		/** メインRTへの書き戻しに使うシェーダーのファイルパス */
		constexpr const char* COPY_BACK_FX_PATH = "Assets/shader/sprite.fx";

		/**
		 * @brief 方式ごとのピクセルシェーダーのエントリーポイント名
		 * @details 方式を追加するときは EnToneMapType・toneMap.fx・
		 *          DEFAULT_EXPOSURES・TYPE_NAMES にも追加すること。
		 */
		constexpr const char* PS_ENTRY_POINTS[TONE_MAP_TYPE_NUM] = {
			"",                             /** enNone（スプライトを作らない） */
			"PSToneMapExposure",            /** enExposure                     */
			"PSToneMapReinhard",            /** enReinhard                     */
			"PSToneMapReinhardExtended",    /** enReinhardExtended             */
			"PSToneMapACES",                /** enACES                         */
			"PSToneMapUncharted2",          /** enUncharted2                   */
		};

		/**
		 * @brief 方式ごとの表示名（デバッグUI用）
		 * @details ImGuiはUTF-8を要求するため、u8接頭辞を必ず付けること。
		 *          付け忘れると実行文字セット（CP932）で埋め込まれて文字化けする。
		 */
		constexpr const char* TYPE_NAMES[TONE_MAP_TYPE_NUM] = {
			u8"enNone（トーンマップなし）",
			u8"enExposure（露出のみ）",
			u8"enReinhard（基本）",
			u8"enReinhardExtended（白を出せる）",
			u8"enACES（映画的）",
			u8"enUncharted2（暗部が締まる）",
		};
	}


	const char* ToneMap::GetTypeName(const EnToneMapType toneMapType)
	{
		const size_t index = static_cast<size_t>(toneMapType);
		if (index >= TONE_MAP_TYPE_NUM)
		{
			return "unknown";
		}
		return TYPE_NAMES[index];
	}


	void ToneMap::Init(RenderTarget& mainRenderTarget, EnToneMapType toneMapType)
	{
		m_toneMapType = toneMapType;

		if (m_toneMapType == EnToneMapType::enNone)
		{
			// トーンマップが無効の場合はリソースを作らない
			// この場合は実行中に他の方式へ切り替えることもできない
			return;
		}

		// 定数バッファデータを設定する
		m_toneMapCb.exposure = DEFAULT_EXPOSURES[static_cast<size_t>(m_toneMapType)];
		m_toneMapCb.whitePoint = INITIAL_WHITE_POINT;
		m_toneMapCb.applyGamma = INITIAL_IS_APPLY_GAMMA ? 1.0f : 0.0f;
		m_toneMapCb.isLuminanceBased = INITIAL_IS_LUMINANCE_BASED ? 1.0f : 0.0f;

		// 各リソースを初期化する
		InitToneMapRenderTarget(mainRenderTarget);
		InitToneMapSprites(mainRenderTarget);
		InitCopyBackSprite(mainRenderTarget);

		m_isInitialized = true;
	}


	void ToneMap::SetToneMapType(const EnToneMapType toneMapType)
	{
		if (!m_isInitialized)
		{
			// enNoneで初期化した場合はスプライトが無いので切り替えられない
			return;
		}

		m_toneMapType = toneMapType;

		// 方式ごとに適正な露出が異なるため、既定値を適用する
		if (toneMapType != EnToneMapType::enNone)
		{
			m_toneMapCb.exposure = DEFAULT_EXPOSURES[static_cast<size_t>(toneMapType)];
		}
	}


	void ToneMap::Render(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		if (!m_isInitialized || m_toneMapType == EnToneMapType::enNone)
		{
			return;
		}

		// メインRTをトーンマップして専用RTへ書き込む
		RenderToneMap(rc);

		// トーンマップ結果をメインRTへ書き戻す
		RenderCopyBack(rc, mainRenderTarget);
	}


	void ToneMap::InitToneMapRenderTarget(RenderTarget& mainRenderTarget)
	{
		// 書き戻し時のフォーマット変換を避けるため、メインRTと同じ設定で作成する
		m_toneMapRenderTarget.Create(
			mainRenderTarget.GetWidth(),
			mainRenderTarget.GetHeight(),
			1,
			1,
			DXGI_FORMAT_R32G32B32A32_FLOAT,
			DXGI_FORMAT_UNKNOWN
		);
	}


	void ToneMap::InitToneMapSprites(RenderTarget& mainRenderTarget)
	{
		// 実行中に方式を切り替えられるよう、全方式のスプライトをここで作っておく
		// ピクセルシェーダーはPSOに焼き込まれるため、描画時に選べるのは
		// あらかじめ作ってあるスプライトだけになる
		for (size_t i = static_cast<size_t>(EnToneMapType::enNone) + 1; i < TONE_MAP_TYPE_NUM; i++)
		{
			SpriteInitData initData;
			initData.m_fxFilePath = TONE_MAP_FX_PATH;
			initData.m_vsEntryPointFunc = "VSMain";
			initData.m_psEntryPoinFunc = PS_ENTRY_POINTS[i];
			initData.m_width = static_cast<UINT>(m_toneMapRenderTarget.GetWidth());
			initData.m_height = static_cast<UINT>(m_toneMapRenderTarget.GetHeight());
			initData.m_textures[0] = &mainRenderTarget.GetRenderTargetTexture();
			initData.m_expandConstantBuffer = &m_toneMapCb;
			initData.m_expandConstantBufferSize = sizeof(m_toneMapCb);
			initData.m_alphaBlendMode = AlphaBlendMode_None;
			initData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;

			m_toneMapSprites[i].Init(initData);
		}
	}


	void ToneMap::InitCopyBackSprite(RenderTarget& mainRenderTarget)
	{
		// 書き戻しは単純なコピーなので、既存のスプライトシェーダーを流用する
		SpriteInitData initData;
		initData.m_fxFilePath = COPY_BACK_FX_PATH;
		initData.m_vsEntryPointFunc = "VSMain";
		initData.m_psEntryPoinFunc = "PSMain";
		initData.m_width = static_cast<UINT>(mainRenderTarget.GetWidth());
		initData.m_height = static_cast<UINT>(mainRenderTarget.GetHeight());
		initData.m_textures[0] = &m_toneMapRenderTarget.GetRenderTargetTexture();
		initData.m_alphaBlendMode = AlphaBlendMode_None;
		initData.m_colorBufferFormat[0] = mainRenderTarget.GetColorBufferFormat();

		m_copyBackSprite.Init(initData);
	}


	void ToneMap::RenderToneMap(RenderContext& rc)
	{
		rc.WaitUntilToPossibleSetRenderTarget(m_toneMapRenderTarget);
		rc.SetRenderTargetAndViewport(m_toneMapRenderTarget);
		m_toneMapSprites[static_cast<size_t>(m_toneMapType)].Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(m_toneMapRenderTarget);
	}


	void ToneMap::RenderCopyBack(RenderContext& rc, RenderTarget& mainRenderTarget)
	{
		rc.WaitUntilToPossibleSetRenderTarget(mainRenderTarget);
		rc.SetRenderTargetAndViewport(mainRenderTarget);
		m_copyBackSprite.Draw(rc);
		rc.WaitUntilFinishDrawingToRenderTarget(mainRenderTarget);
	}

} // namespace nsBeastEngine
