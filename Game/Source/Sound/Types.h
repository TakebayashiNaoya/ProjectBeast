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

		/** 親ペンギンのSE */
		enSoundKind_DaddyPenguinShoutFollow,
		enSoundKind_DaddyPenguinShoutWait,
		enSoundKind_DaddyPenguinSystemFollow,
		enSoundKind_DaddyPenguinSystemWait,

		/** ペンギン共通のSE */
		enSoundKind_PenguinDash,
		enSoundKind_PenguinJump,
		enSoundKind_PenguinLanding,
		enSoundKind_PenguinSlide,
		enSoundKind_PenguinSneak,
		enSoundKind_PenguinSwimming,
		enSoundKind_PenguinWaterIn,
		enSoundKind_PenguinWaterOut,

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
		enSoundKind_Result,

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
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/shoutFollow.wav"),
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/shoutWait.wav"),
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/systemFollow.wav"),
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/systemWait.wav"),
		SoundInformation("Assets/sound/SE/penguin/dash.wav"),
		SoundInformation("Assets/sound/SE/penguin/jump.wav"),
		SoundInformation("Assets/sound/SE/penguin/landing.wav"),
		SoundInformation("Assets/sound/SE/penguin/slide.wav"),
		SoundInformation("Assets/sound/SE/penguin/sneak.wav"),
		SoundInformation("Assets/sound/SE/penguin/swimming.wav"),
		SoundInformation("Assets/sound/SE/penguin/waterIn.wav"),
		SoundInformation("Assets/sound/SE/penguin/waterOut.wav"),
		SoundInformation("Assets/sound/SE/step.wav"),
		SoundInformation("Assets/sound/SE/roar.wav"),
		SoundInformation("Assets/sound/SE/growl.wav"),
		SoundInformation("Assets/sound/SE/attack.wav"),
		SoundInformation("Assets/sound/SE/AS_139690_キャンセルや決定ボタンの選択音（ピコン）.wav"),

		//BGM
		SoundInformation("Assets/sound/BGM/AS_1419280_氷の世界＿雪＿かわいいフィールドBGM.wav"),
		SoundInformation("Assets/sound/BGM/AS_1157435_輝く銀世界彩るR_Bポップ＿+.wav"),
		SoundInformation("Assets/sound/BGM/AS_1408083_かわいい軽やかクリスマスBGM。CM広告.wav"),

		//Voice
		SoundInformation("Assets/sound/Voice/bootA.wav")
	};
}
