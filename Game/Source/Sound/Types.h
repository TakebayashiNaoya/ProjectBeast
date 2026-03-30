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
		enSoundKind_EnemyStep,
		enSoundKind_EnemyRoar,
		enSoundKind_EnemyGrowl,
		enSoundKind_EnemyAttack,

		enSoundKind_ButtonPush,

		/** SEの設定はここまで */
		enSoundKind_SE_Max,

		enSoundKind_BGM = enSoundKind_SE_Max,
		/** 下にBGMの設定 */
		enSoundKind_Game = enSoundKind_BGM,
		enSoundKind_Title,

		/** BGMの設定はここまで */

		enSoundKind_Voice,
		/** 下にVoiceの設定 */
		enSoundKind_bootA = enSoundKind_Voice,

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
		SoundInformation("Assets/sound/SE/step.wav"),
		SoundInformation("Assets/sound/SE/roar.wav"),
		SoundInformation("Assets/sound/SE/growl.wav"),
		SoundInformation("Assets/sound/SE/attack.wav"),
		SoundInformation("Assets/sound/SE/AS_139690_キャンセルや決定ボタンの選択音（ピコン）.wav"),

		//BGM
		SoundInformation("Assets/sound/BGM/AS_1419280_氷の世界＿雪＿かわいいフィールドBGM.wav"),
		SoundInformation("Assets/sound/BGM/AS_1157435_輝く銀世界彩るR_Bポップ＿+.wav"),

		//Voice
		SoundInformation("Assets/sound/Voice/bootA.wav")
	};
}
