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

} // namespace nsBeastEngine
