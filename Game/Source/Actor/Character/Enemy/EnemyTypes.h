/**
 * EnemyTypes.h
 * Enemyが使用する定数の定義を行う
 */
#pragma once


 /**
  * プレイヤーのアニメーションの種類
  */
enum class EnemyAnimationType : uint8_t
{
	Idle,
	Walk,
	Attack,
	Num,
};

