/**
 * @file Types.h
 * @brief エフェクト用の定数など必要な情報を定義するファイル
 * @author 藤谷
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
	PenguinJump,
	Max,
	None = Max,
};


/** エフェクトの情報の構造体 */
struct EffectInformation
{
	const char16_t* assetPath;
	//
	EffectInformation(const char16_t* path) : assetPath(path) {}
};


/** 情報を保持 */
static EffectInformation effectInformation[static_cast<uint8_t>(EnEffectKind::Max)] =
{
	EffectInformation(u"Assets/effect/DaddyPenguinCommand.efk"),
	EffectInformation(u"Assets/effect/enemy/EnemyAttack.efk"),
	EffectInformation(u"Assets/effect/whirlpool/Whirlpool.efk"),
	EffectInformation(u"Assets/effect/stage/igloo/IglooBreak.efk"),
	EffectInformation(u"Assets/effect/swim/swimSplash.efk"),
	EffectInformation(u"Assets/effect/penguin/jump/jumpFrost.efk"),
};

