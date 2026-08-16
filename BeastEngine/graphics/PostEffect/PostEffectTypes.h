/**
 * @file PostEffectTypes.h
 * @brief ポストエフェクトの種別を定義するEnum
 * @author 竹林
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief ブルームの種別
	 * @details PostEffectManager::Init()の引数に渡すことで切り替える
	 */
	enum class EnBloomType
	{
		enNone,    /** ブルームなし（オフ）         */
		enNormal,  /** 通常ブルーム（平均合成）     */
		enKawase,  /** 川瀬式ブルーム（縮小多段合成）*/
	};


	/**
	 * @brief ブラーの種別
	 * @details PostEffectManager::Init()の引数に渡すことで切り替える
	 */
	enum class EnBlurType
	{
		enAverage,   /** 平均ブラー     */
		enGaussian,  /** ガウシアンブラー*/
	};


	/**
	 * @brief トーンマップの種別
	 * @details PostEffectManager::Init()の引数に渡すことで切り替える。
	 *          方式を追加する場合は、ここへの追加に加えて
	 *          toneMap.fx のピクセルシェーダーと
	 *          ToneMap.cpp の GetPixelShaderEntryPoint() にも追加すること。
	 */
	enum class EnToneMapType
	{
		enNone,              /** トーンマップなし（1.0を超えた値は白に潰れる）   */
		enExposure,          /** 露出のみ適用してクランプ（比較用の基準）        */
		enReinhard,          /** Reinhard。素直に白飛びを抑える                  */
		enReinhardExtended,  /** ホワイトポイント付きReinhard。最大輝度を指定可  */
		enACES,              /** ACESフィルミック近似。コントラストが強い        */
		enUncharted2,        /** Uncharted2フィルミック。暗部が締まる            */
		enNum,               /** 種別の数（配列サイズ用。方式としては使わない）  */
	};

} // namespace nsBeastEngine
