/**
 * @file PostEffectManager.h
 * @brief ポストエフェクトマネージャー
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/PostEffect/PostEffectTypes.h"
#include "Graphics/PostEffect/Bloom.h"
#include "Graphics/PostEffect/RadialBlur.h"
#include "Graphics/PostEffect/ToneMap.h"


namespace nsBeastEngine
{
	/**
	 * @brief ポストエフェクトマネージャー
	 * @details RenderingEngine が保持し、Execute() 内で Render() を呼び出す。
	 *          エフェクトを含む3D描画完了後・UI描画前に実行することで、
	 *          エフェクトもポストエフェクトの対象にしつつUIへの影響を防ぐ。
	 *          各ポストエフェクトをここで一括管理する。
	 *          将来の被写界深度・モーションブラー等の追加はここに行う。
	 */
	class PostEffectManager
	{
	public:
		PostEffectManager() = default;
		~PostEffectManager() = default;


	public:
		/**
		 * @brief 初期化
		 * @param mainRenderTarget メインレンダリングターゲット
		 * @param bloomType        ブルームの種別
		 * @param blurType         ブラーの種別
		 * @param toneMapType      トーンマップの種別
		 */
		void Init(
			RenderTarget& mainRenderTarget,
			EnBloomType bloomType,
			EnBlurType blurType,
			EnToneMapType toneMapType
		);

		/**
		 * @brief 全ポストエフェクトを実行する
		 * @details HDRのまま行うブルームを先に実行し、
		 *          最後にトーンマップでLDRへ変換する順序で呼び出す。
		 * @param rc               レンダリングコンテキスト
		 * @param mainRenderTarget メインレンダリングターゲット
		 */
		void Render(RenderContext& rc, RenderTarget& mainRenderTarget);

		/**
		 * @brief トーンマップを取得する
		 * @details デバッグUIから実行中にパラメーターを調整するために使う
		 * @return トーンマップの参照
		 */
		ToneMap& GetToneMap() { return m_toneMap; }

		/**
		 * @brief ラジアルブラーを取得する
		 * @details シロクマの咆哮などの衝撃演出でゲーム側が Start() を呼ぶために使う
		 * @return ラジアルブラーの参照
		 */
		RadialBlur& GetRadialBlur() { return m_radialBlur; }


	private:
		/** ブルームエフェクト */
		Bloom m_bloom;
		/** ラジアルブラー */
		RadialBlur m_radialBlur;
		/** トーンマップ */
		ToneMap m_toneMap;
	};

} // namespace nsBeastEngine