/**
 * @file Types.h
 * @brief サウンド用の定数やファイルパスなどを定義するクラス
 * @author 立山
 */
#pragma once
#include <string>


namespace app
{
	enum enSoundKind
	{
		enSoundKind_SE = 0,
		enSoundKind_SE_Max,
		enSoundKind_BGM = enSoundKind_SE_Max,
		enSoundKind_Voice,
		enSoundKind_Max,
		enSoundKind_None = enSoundKind_Max,
	};


	/** サウンドの情報の構造体 */
	struct SoundInformation
	{
		std::string assetPath;

		SoundInformation(const std::string& path) :assetPath(path) {}
	};


	/** 情報を保持 */
	//static SoundInformation soundInformation[enSoundKind_Max] =
	//{
	//	//SE

	//};
}
