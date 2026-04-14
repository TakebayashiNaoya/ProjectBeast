/**
 * EnemyTypes.h
 * Enemyが使用する定数の定義を行う
 */
#pragma once


 /**
  * プレイヤーのアニメーションの種類
  */
enum class EnEnemyAnimationType : uint8_t
{
	Idle,
	Idle_UnderWater,
	Walk,
	Attack,
	Attack_UnderWater,
	BackWalk,
	Run,
	Swim,
	Buff,
	Damage,
	Eat,
	Stun,
	Sleep,

	Num,
};

