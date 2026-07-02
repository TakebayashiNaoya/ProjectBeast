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
		// =========================================================================
		// SE (効果音)
		// =========================================================================
		enSoundKind_SE = 0,
		enSoundKind_Damage = enSoundKind_SE,

		// --- 親ペンギン ---
		enSoundKind_DaddyPenguinShoutFollow,
		enSoundKind_DaddyPenguinShoutWait,
		enSoundKind_DaddyPenguinSystemFollow,
		enSoundKind_DaddyPenguinSystemWait,

		// --- 子ペンギン ---
		enSoundKind_ChildPenguinCRY,
		enSoundKind_NaughtyPoke,

		// --- ペンギン共通 ---
		enSoundKind_PenguinDash,
		enSoundKind_PenguinJump,
		enSoundKind_PenguinLanding,
		enSoundKind_PenguinSlide,
		enSoundKind_PenguinSneak,
		enSoundKind_PenguinSwimming,
		enSoundKind_PenguinWaterIn,
		enSoundKind_PenguinWaterOut,

		// --- 敵 ---
		enSoundKind_EnemyStep,
		enSoundKind_EnemyRoar,
		enSoundKind_EnemyGrowl,
		enSoundKind_EnemyAttack,

		// --- 子ペンギンリアクション ---
		enSoundKind_CPReactionHappy,
		enSoundKind_CPReactionTrouble,

		// --- ギミック・環境音 ---
		enSoundKind_IglooBreak,

		// -------------------------------------------------------------------------
		// UI / システム関連
		// -------------------------------------------------------------------------
		// --- UI: 汎用操作 ---
		enSoundKind_ButtonBack,
		enSoundKind_ButtonEnter,
		enSoundKind_CursorMove,

		// --- UI: ゲーム進行・演出 ---
		enSoundKind_Whistle,
		enSoundKind_CountDown,
		enSoundKind_GameStart,

		// --- UI: ゲージ・数値変動（救助数など） ---
		enSoundKind_RemainPlus,
		enSoundKind_RemainORTotalMinus,

		// --- UI: 実績通知 ---
		enSoundKind_NoticeAchievement,
		enSoundKind_FadeOutAchievement,

		// --- UI: リザルト画面 ---
		enSoundKind_Stamp,
		enSoundKind_DrumRoll,
		enSoundKind_Cymbals,

		enSoundKind_SE_Max,

		// =========================================================================
		// BGM
		// =========================================================================
		enSoundKind_BGM = enSoundKind_SE_Max,
		enSoundKind_InGame = enSoundKind_BGM,
		enSoundKind_Title,
		enSoundKind_Result,

		// =========================================================================
		// Voice (ボイス)
		// =========================================================================
		enSoundKind_Voice,
		enSoundKind_bootA = enSoundKind_Voice,

		// =========================================================================
		// 全体の最大数定義
		// =========================================================================
		enSoundKind_Max,
		enSoundKind_None = enSoundKind_Max,
	};


	/** サウンドの情報の構造体 */
	struct SoundInformation
	{
		std::string assetPath;

		SoundInformation(const std::string& path) : assetPath(path) {}
	};


	/** 情報を保持 */
	static SoundInformation soundInformation[enSoundKind_Max] =
	{
		// -------------------------------------------------------------------------
		// SE (効果音)
		// -------------------------------------------------------------------------
		SoundInformation("Assets/sound/SE/damage.wav"),

		// --- 親ペンギン ---
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/shoutFollow.wav"),
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/shoutWait.wav"),
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/systemFollow.wav"),
		SoundInformation("Assets/sound/SE/penguin/daddyPenguin/systemWait.wav"),

		// --- 子ペンギン ---
		SoundInformation("Assets/sound/Voice/ChildPenguinCryVoice.wav"),
		SoundInformation("Assets/sound/SE/penguin/childPenguin/NaughtyPenguinWakeBear.wav"),

		// --- ペンギン共通 ---
		SoundInformation("Assets/sound/SE/penguin/dash.wav"),
		SoundInformation("Assets/sound/SE/penguin/jump.wav"),
		SoundInformation("Assets/sound/SE/penguin/landing.wav"),
		SoundInformation("Assets/sound/SE/penguin/slide.wav"),
		SoundInformation("Assets/sound/SE/penguin/sneak.wav"),
		SoundInformation("Assets/sound/SE/penguin/swimming.wav"),
		SoundInformation("Assets/sound/SE/penguin/waterIn.wav"),
		SoundInformation("Assets/sound/SE/penguin/waterOut.wav"),

		// --- 敵 ---
		SoundInformation("Assets/sound/SE/enemy/step.wav"),
		SoundInformation("Assets/sound/SE/enemy/roar.wav"),
		SoundInformation("Assets/sound/SE/enemy/growl.wav"),
		SoundInformation("Assets/sound/SE/enemy/attack.wav"),

		// --- 子ペンギンリアクション ---
		SoundInformation("Assets/sound/SE/CPReaction/happy.wav"),
		SoundInformation("Assets/sound/SE/CPReaction/trouble.wav"),

		// --- ギミック・環境音 ---
		SoundInformation("Assets/sound/SE/igloo/iglooBreak.wav"),

		// -------------------------------------------------------------------------
		// UI / システム関連
		// -------------------------------------------------------------------------
		// --- UI: 汎用操作 ---
		SoundInformation("Assets/sound/SE/controller/buttonBack.wav"),
		SoundInformation("Assets/sound/SE/controller/buttonEnter.wav"),
		SoundInformation("Assets/sound/SE/controller/cursorMove.wav"),

		// --- UI: ゲーム進行・演出 ---
		SoundInformation("Assets/sound/SE/progress/whistle.wav"),
		SoundInformation("Assets/sound/SE/progress/countDown.wav"),
		SoundInformation("Assets/sound/SE/progress/gameStart.wav"),

		// --- UI: ゲージ・数値変動（救助数など） ---
		SoundInformation("Assets/sound/SE/UI/remaining/plus.wav"),
		SoundInformation("Assets/sound/SE/UI/remaining/minus.wav"),

		// --- UI: 実績通知 ---
		SoundInformation("Assets/sound/SE/achievement/noticeAchievement.wav"),
		SoundInformation("Assets/sound/SE/achievement/fadeOutAchievement.wav"),

		// --- UI: リザルト画面 ---
		SoundInformation("Assets/sound/SE/result/stamp.wav"),
		SoundInformation("Assets/sound/SE/result/drumRoll.wav"),
		SoundInformation("Assets/sound/SE/result/cymbals.wav"),

		// -------------------------------------------------------------------------
		// BGM
		// -------------------------------------------------------------------------
		SoundInformation("Assets/sound/BGM/inGame.wav"),
		SoundInformation("Assets/sound/BGM/title.wav"),
		SoundInformation("Assets/sound/BGM/result.wav"),

		// -------------------------------------------------------------------------
		// Voice (ボイス)
		// -------------------------------------------------------------------------
		SoundInformation("Assets/sound/Voice/bootA.wav")
	};
}