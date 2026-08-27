/**
 * @file UltController.h
 * @brief ウルトの発動・タイマー・クールダウンを管理するコントローラー
 */
#pragma once
#include "Source/Sound/SoundHandle.h"
#include "UltContext.h"


namespace app
{
	namespace actor
	{
		class FormationEffectChain;
		class IUltEffect;


		/**
		 * @brief ウルトコントローラー
		 * @details
		 *   FormationEffectChain（効果・非所有ポインタ）と IUltEffect（演出・非所有ポインタ）を保持し、
		 *   発動・更新・終了を管理する。チェーンと演出の所有権はどちらも IFormation が持つ。
		 *   陣形切り替え時は FormationController が SetUlt() を呼び直す。
		 *
		 *   ウルト発動時に Enter()、毎フレーム Update()、終了時に Exit() を
		 *   効果チェーンと演出の両方に転送する。
		 *   速度倍率・渦潮耐性の取得は FormationController が直接チェーンに問い合わせる。
		 */
		class UltController
		{
		public:
			UltController() = default;
			/**
			 * @brief デストラクタ
			 * @details 再生中のループSE（チャージ・ディスチャージ）を止めてから破棄する。
			 *          通常はシーン破棄時の StopAllSE で止まるが、それを経由しない破棄経路でも
			 *          鳴りっぱなしにならないよう保険として停止する。
			 */
			~UltController();

			/**
			 * @brief ウルトチェーンと演出をセットする（陣形切り替え時に呼ぶ）
			 * @param ult      陣形のウルトエフェクトチェーン（IFormation が所有する）
			 * @param visual   陣形のウルト演出（IFormation が所有する）。演出なしは nullptr 可
			 * @param duration 持続時間（秒）
			 * @param cooldown クールダウン（秒）
			 * @param formationName 陣形名（プレイログにどの陣形のウルトかを残すために使う）
			 */
			void SetUlt(FormationEffectChain* ult, IUltEffect* visual, float duration, float cooldown,
				const char* formationName);

			/** @brief 発動可能か（未発動 かつ クールダウン終了） */
			bool CanActivate() const;

			/**
			 * @brief ウルトを発動する（チェーンと演出の Enter を呼ぶ）
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

			/**
			 * @brief 発動中の残り時間割合を 0.0〜1.0 で返す（UI表示用）
			 * @return 発動直後は1.0、終了間際は0.0。発動中でなければ0.0
			 */
			float GetActiveRemainingRate() const;

			/**
			 * @brief クールダウンを最大値にリセットする
			 * @details ゲーム開始直後など、ウルトを「何も貯まっていない状態」から開始させたい場合に呼ぶ
			 */
			/**
			 * @brief クールダウンを張り直す
			 * @param rate クールダウン時間に掛ける倍率（省略時1.0）。
			 *             ゲーム開始時に0.5を渡すと、最初のウルトへ半分の待ち時間で到達できる
			 */
			void ResetCooldown(const float rate = 1.0f) { m_cooldownTimer = m_cooldown * rate; }


		private:
			/**
			 * @brief チャージSEの再生状態をゲージ蓄積状態に同期する
			 * @details ゲージ蓄積中（クールダウン中）はループ再生し、満タン・発動中は停止する。
			 *          ゲーム開始直後の初回チャージとウルト使用後のチャージの両方を拾う。
			 */
			void UpdateChargeSe();

			/**
			 * @brief ディスチャージSEの再生状態をウルト発動状態に同期する
			 * @details 発動中（効果持続中）はループ再生し、発動が終わったら停止する。
			 */
			void UpdateDischargeSe();


		private:
			FormationEffectChain* m_ult       = nullptr;  /** 効果チェーン。非所有ポインタ。IFormation が所有する */
			IUltEffect*           m_ultVisual = nullptr;  /** ウルト演出。非所有ポインタ。IFormation が所有する */
			float                 m_duration = 0.0f;
			float                 m_timer = 0.0f;
			float                 m_cooldown = 0.0f;
			float                 m_cooldownTimer = 0.0f;
			bool                  m_isActive = false;

			/** チャージ（ゲージ蓄積）中に再生しているループSEのハンドル */
			SEHandle              m_chargeSeHandle = INVALID_SE_HANDLE;
			/** ディスチャージ（発動中）に再生しているループSEのハンドル */
			SEHandle              m_dischargeSeHandle = INVALID_SE_HANDLE;

			/** 現在セットされている陣形の名前（プレイログ用。文字列リテラルを指すため所有しない） */
			const char*           m_formationName = "Unknown";
		};
	}
}
