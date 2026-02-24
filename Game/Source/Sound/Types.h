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

		/** 下にSEの設定 */
		enSoundKind_Damage = enSoundKind_SE,

		/** SEの設定はここまで */
		enSoundKind_SE_Max,

		enSoundKind_BGM = enSoundKind_SE_Max,
		/** 下にBGMの設定 */
		enSoundKind_Game = enSoundKind_BGM,

		/** BGMの設定はここまで */

		enSoundKind_Voice,
		/** 下にVoiceの設定 */
		enSoundKind_Penguin = enSoundKind_Voice,

		/** Voiceの設定はここまで */

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
	static SoundInformation soundInformation[enSoundKind_Max] =
	{
		//SE
		SoundInformation("Assets/sound/SE/damage.wav"),
		//BGM
		SoundInformation("Assets/sound/BGM/InGame.wav"),
		//Voice
		SoundInformation("Assets/sound/Voice/penguin.wav")
	};
}
