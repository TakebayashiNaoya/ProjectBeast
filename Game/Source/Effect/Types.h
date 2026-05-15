/**
 * @file Types.h
 * @brief エフェクト用の定数など必要な情報を定義するファイル
 * @author 藤谷、竹林
 */
#pragma once
#include <string>


 /** エフェクトの種類 */
enum class EnEffectKind : uint8_t
{
	Kind = 0,
	DaddyPenguinCommand = Kind,
	EnemyAttack,
	Whirlpool,
	IglooBreak,
	SwimSplash,
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
	EffectInformation(u"Assets/effect/whirlpool/Whirlpool.efk",  200.0f),
	EffectInformation(u"Assets/effect/stage/igloo/IglooBreak.efk", 100.0f),
	EffectInformation(u"Assets/effect/swim/swimSplash.efk",      60.0f),
};