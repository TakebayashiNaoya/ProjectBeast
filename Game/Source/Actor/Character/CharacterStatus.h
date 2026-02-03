#pragma once
#include "Source/Actor/ActorStatus.h"



namespace app
{
	namespace actor
	{
		class CharacterStatus : public ActorStatus
		{
		protected:
			/** 最大体力 */
			int m_maxHp;
			/** 体力 */
			int m_hp;
			/** 移動速度(歩き) */
			float m_walkSpeed;
			/** 移動速度(走り) */
			float m_runSpeed;
			/** 半径 */
			float m_radius;
			/** 高さ */
			float m_height;


		public:
			CharacterStatus() = default;
			~CharacterStatus() = default;


		public:
			/*
			 * セットアップ
			 * ステータスの持ち主が必ず呼び出すこと
			 */
			virtual void Setup() override;


		public:
			/**
			 * @brief 最大体力を取得
			 * @return 最大体力
			 */
			inline int GetMaxHp() const { return m_maxHp; }
			/**
			 * @brief 体力を取得
			 * @return 体力
			 */
			inline int GetHp() const { return m_hp; }
			/**
			 * @brief 歩き速度を取得
			 * @return 歩き速度
			 */
			inline float GetWalkSpeed() const { return m_walkSpeed; }
			/**
			 * @brief 走り速度を取得
			 * @return 走り速度
			 */
			inline float GetRunSpeed() const { return m_runSpeed; }
			/**
			 * @brief 半径を取得
			 * @return 半径
			 */
			inline float GetRadius() const { return m_radius; }
			/**
			 * @brief 高さを取得
			 * @return 高さ
			 */
			inline float GetHeight() const { return m_height; }
		};
	}
}

