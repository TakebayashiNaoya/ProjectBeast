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
		class FormationEffectChain;


		/**
		 * @brief ウルトコントローラー
		 * @details
		 *   FormationEffectChain（非所有ポインタ）を保持し、発動・更新・終了を管理する。
		 *   チェーンの所有権は IFormation が持つ。
		 *   陣形切り替え時は FormationController が SetUlt() を呼び直す。
		 *
		 *   ウルト発動時に Enter()、毎フレーム Update()、終了時に Exit() を転送する。
		 *   速度倍率・渦潮耐性の取得は FormationController が直接チェーンに問い合わせる。
		 */
		class UltController
		{
		public:
			/**
			 * @brief ウルトチェーンをセットする（陣形切り替え時に呼ぶ）
			 * @param ult      陣形のウルトエフェクトチェーン（IFormation が所有する）
			 * @param duration 持続時間（秒）
			 * @param cooldown クールダウン（秒）
			 */
			void SetUlt(FormationEffectChain* ult, float duration, float cooldown);

			/** @brief 発動可能か（未発動 かつ クールダウン終了） */
			bool CanActivate() const;

			/**
			 * @brief ウルトを発動する（チェーンの Enter を呼ぶ）
			 * @param ctx ウルト発動時のコンテキスト情報
			 */
			void Activate(const UltContext& ctx);

			/**
			 * @brief 毎フレーム更新（タイマー管理・Exit呼び出し）
			 * @param dt  前フレームからの経過時間（秒）
			 * @param ctx ウルト更新時のコンテキスト情報
			 */
			void Update(float dt, const UltContext& ctx);

			/** @brief ウルト発動中か */
			bool IsActive() const { return m_isActive; }

			/** @brief クールダウン中か */
			bool IsOnCooldown() const { return m_cooldownTimer > 0.0f; }

			/**
			 * @brief クールダウン残量を 0.0〜1.0 で返す（UI表示用）
			 * @return 0.0 で完了、1.0 で開始直後
			 */
			float GetCooldownRate() const;


		private:
			FormationEffectChain* m_ult          = nullptr;  /** 非所有ポインタ。IFormation が所有する */
			float                 m_duration      = 0.0f;
			float                 m_timer         = 0.0f;
			float                 m_cooldown      = 0.0f;
			float                 m_cooldownTimer = 0.0f;
			bool                  m_isActive      = false;
		};
	}
}
