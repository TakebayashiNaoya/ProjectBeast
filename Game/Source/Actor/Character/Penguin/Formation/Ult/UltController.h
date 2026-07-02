/**
 * @file UltController.h
 * @brief ウルトの発動・タイマー・クールダウンを管理するコントローラー
 * @author 竹林
 */
#pragma once
#include "UltContext.h"


namespace app
{
	namespace actor
	{
		class IUltEffect;


		/**
		 * @brief ウルトコントローラー
		 * @details
		 *   IUltEffect（非所有ポインタ）を保持し、発動・更新・終了を管理する。
		 *   IUltEffect の所有権は IFormation が持つ。
		 *   陣形切り替え時は FormationController が SetUlt() を呼び直す。
		 */
		class UltController
		{
		public:
			/**
			 * @brief ウルトをセットする（陣形切り替え時に呼ぶ）
			 * @param ult      新しいウルト効果（IFormation が所有する）
			 * @param duration 持続時間（秒）
			 * @param cooldown クールダウン（秒）
			 */
			void SetUlt(IUltEffect* ult, float duration, float cooldown);

			/** 
			 * @brief 発動可能か（未発動 かつ クールダウン終了）
			 */
			bool CanActivate() const;

			/**
			 * @brief ウルトを発動する
			 * @param ctx ウルト発動時のコンテキスト情報
			 */
			void Activate(const UltContext& ctx);

			/** 
			 * @brief 毎フレーム更新（タイマー管理・Deactivate呼び出し）
			 * @param dt  前フレームからの経過時間（秒）
			 * @param ctx ウルト更新時のコンテキスト情報
			 */
			void Update(float dt, const UltContext& ctx);

			/** 
			 * @brief ウルト発動中か
			 * @return true で発動中、false で非発動中
			 */
			bool IsActive() const { return m_isActive; }

			/** 
			 * @brief クールダウン中か
			 * @return true でクールダウン中、false で発動可能
			 */
			bool IsOnCooldown() const { return m_cooldownTimer > 0.0f; }

			/** 
			 * @brief クールダウン残量を 0.0〜1.0 で返す（UI表示用）
			 * @return クールダウンの進行状況（0.0 で完了、1.0 で開始）
			 */
			float GetCooldownRate() const;

			/** 
			 * @brief 現在の速度倍率ボーナス（非発動中は 1.0f）
			 * @return 速度倍率ボーナス
			 */
			float GetSpeedMultiplierBonus() const;

			/** 
			 * @brief 現在、渦潮免疫が有効か
			 * @return true で有効、false で無効
			 */
			bool IsWhirlpoolImmune() const;


		private:
			IUltEffect* m_ult			= nullptr;	/** 非所有ポインタ。IFormation が所有する */
			float       m_duration		= 0.0f;		/** ウルト持続時間（秒） */
			float       m_timer			= 0.0f;		/** ウルト発動からの経過時間（秒） */
			float       m_cooldown		= 0.0f;		/** ウルトクールダウン（秒） */
			float       m_cooldownTimer = 0.0f;		/** クールダウン残り時間（秒） */
			bool        m_isActive		= false;	/** ウルト発動中か */
		};
	}
}
