/**
 * @file Types.h
 * @brief エフェクト用の定数など必要な情報を定義するファイル
 */
#pragma once
#include <string>


namespace app
{
	/** エフェクト用のハンドル名 */
	using EffectHandle = uint32_t;

	/** エフェクトの種類 */
	enum class EnEffectKind : uint8_t
	{
		Kind = 0,
		DaddyPenguinCommand = Kind,
		EnemyAttack,
		Whirlpool,
		IglooBreak,
		SwimSplash,
		PenguinLanding,
		PenguinSlideFrost,
		PenguinSlideLine,
		ChildPenguinCry,
		ClingyPenguinHart,
		CaringPenguinSweat,
		NaughtyPenguinLively,
		ToEnemyDebuff,
		GhostPenguinBlurFireBall,
		ClusterUltBegin,
		ClusterUltEnd,
		TriangleUltBegin,
		TriangleUltEnd,
		ScatterUlt,
		CircleUltBegin,
		CircleUltEnd,
		Max,
		None = Max,
	};

	/** エフェクトの情報の構造体 */
	struct EffectInformation
	{
		const char16_t* assetPath;
		/** バウンディング球の基準半径（スケール1.0時の見かけ上の最大半径） */
		float baseRadius;

		EffectInformation(const char16_t* path, const float radius)
			: assetPath(path)
			, baseRadius(radius)
		{}
	};

	/** 情報を保持 */
	static EffectInformation effectInformation[static_cast<uint8_t>(EnEffectKind::Max)] =
	{
		EffectInformation(u"Assets/effect/DaddyPenguinCommand.efk",  50.0f),
		EffectInformation(u"Assets/effect/enemy/EnemyAttack.efk",    80.0f),
		EffectInformation(u"Assets/effect/whirlpool/Whirlpool2.efk",  200.0f),
		EffectInformation(u"Assets/effect/stage/igloo/IglooBreak.efk", 100.0f),
		EffectInformation(u"Assets/effect/swim/swimSplash.efk",      60.0f),
		EffectInformation(u"Assets/effect/penguin/jump/LandingFrost.efk", 60.0f),
		EffectInformation(u"Assets/effect/penguin/slide/SlideFrost.efk", 50.0f),
		EffectInformation(u"Assets/effect/penguin/slide/SlideLine.efk", 100.0f),
		EffectInformation(u"Assets/effect/penguin/childPenguin/penguinCRY.efk", 150.0f),
		EffectInformation(u"Assets/effect/childPenguin/childPenguinClingyHart.efk", 200.0f),
		EffectInformation(u"Assets/effect/childPenguin/childPenguinCaringSweat.efk", 100.0f),
		EffectInformation(u"Assets/effect/childPenguin/childPenguinNaugtyLively.efk", 100.0f),
		EffectInformation(u"Assets/effect/enemy/Gloomy.efk", 100.0f),
		EffectInformation(u"Assets/effect/childPenguin/ghostPenguinBlurFireBall.efk", 100.0f),
		EffectInformation(u"Assets/effect/ult/clusterBegin.efk", 150.0f),
		EffectInformation(u"Assets/effect/ult/clusterEnd.efk", 150.0f),
		EffectInformation(u"Assets/effect/ult/triangleBegin.efk", 150.0f),
		EffectInformation(u"Assets/effect/ult/triangleEnd.efk", 150.0f),
		EffectInformation(u"Assets/effect/ult/scatter.efk", 150.0f),
		EffectInformation(u"Assets/effect/ult/circleBegin.efk", 150.0f),
		EffectInformation(u"Assets/effect/ult/circleEnd.efk", 150.0f),
	};
}