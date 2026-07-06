/**
 * @file FormationController.h
 * @brief 陣形の切り替えと座標計算を管理するコントローラー
 * @author 竹林
 */
#pragma once
#include <array>
#include <functional>
#include <memory>
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"
#include "Source/Actor/Character/Penguin/Formation/Ult/UltController.h"


namespace app
{
	namespace actor
	{
		class IFormation;
		struct UltContext;


		/**
		 * @brief 陣形コントローラー
		 * @details
		 *   IFormation の各実装を保持し、現在の陣形への切り替えと
		 *   座標計算の委譲を担う。レベル管理もここで行う。
		 *   ChildPenguinManager のメンバーとして所有される。
		 *
		 *   速度倍率 = passive->GetSpeedMultiplier(level) × (ウルト中なら ult->GetSpeedMultiplier(level))
		 *   渦潮耐性 = passive->HasWhirlpoolResistance() || (ウルト中 && ult->HasWhirlpoolResistance())
		 */
		class FormationController
		{
		public:
			FormationController();
			~FormationController();


			/**
			 * @brief 陣形座標を計算する（現在の陣形に委譲）
			 * @details レベルが上昇した場合は m_onLevelUp を呼び出す。
			 * @param center         親ペンギンの座標
			 * @param forward        親ペンギンの前方向（正規化済み）
			 * @param out            計算結果を書き込むベクター（事前にclearしておくこと）
			 * @param count          位置を生成するスロット数（m_outerRadius の算出に使用）
			 * @param countForLevel  レベル判定に使うフォロワー数。-1 の場合は count を使う
			 */
			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count,
				int countForLevel = -1
			);

			/**
			 * @brief 現在充填中のリングの全スロット座標を計算する（表示専用・状態を変化させない）
			 * @details m_outerRadius を元の値に復元するため、レベルアップ判定もしない。
			 * @param center   親ペンギンの座標
			 * @param forward  親ペンギンの前方向（正規化済み）
			 * @param out      計算結果を書き込むベクター（事前にclearしておくこと）
			 * @param occupied 現在のフォロワー数（空きスロットのフィルタリングに使用）
			 */
			void CalculateNextLevelPositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int occupied
			);

			/**
			 * @brief 陣形を切り替える
			 * @param type 切り替え先の陣形
			 */
			void SwitchFormation(EnFormationType type);

			/** 
			 * @brief 現在の陣形種別を取得する
			 * @return 現在の陣形種別
			 */
			EnFormationType GetCurrentType() const { return m_currentType; }

			/**
			 * @brief 移動速度倍率を取得する
			 * @details 陣形効果チェーンから直接取得する（パッシブ + ウルトを統合済み）。
			 */
			float GetSpeedMultiplier() const;

			/**
			 * @brief 現在の陣形が渦潮耐性を持つか（パッシブ + ウルト効果を統合済み）
			 * @return true で持つ、false で持たない
			 */
			bool HasWhirlpoolResistance() const;

			/**
			 * @brief 指定フォロワー数に対応する入隊判定半径を返す
			 * @param count フォロワー数
			 */
			float GetJoinRadius(int count) const;

			/** 
			 * @brief 最外半径を取得する（CalculatePositions後に有効）
			 * @return 最外半径
			 */
			float GetOuterRadius() const;

			/** 
			 * @brief 入隊判定半径を取得する（最外半径 + 入隊マージン）
			 * @return 入隊判定半径
			 */
			float GetJoinRadius() const;

			/** 
			 * @brief 陣形レベルを取得する（フォロワー数 / FOLLOWERS_PER_LEVEL）
			 * @return 陣形レベル
			 */
			int GetFormationLevel() const { return m_formationLevel; }

			/**
			 * @brief レベルアップ時のコールバックを設定する
			 * @param callback 引数: 新しいレベル
			 */
			void SetOnLevelUp(std::function<void(int)> callback) { m_onLevelUp = std::move(callback); }


			//============================================//
			// ウルト操作
			//============================================//

			/** 
			 * @brief ウルトを発動する
			 * @param ctx ウルト発動時のコンテキスト情報
			 */
			void ActivateUlt(const UltContext& ctx) { m_ultController.Activate(ctx); }

			/** 
			 * @brief ウルトを毎フレーム更新する（ChildPenguinManager::Update から呼ぶ）
			 * @param dt 前フレームとの時間差（秒）
			 * @param ctx ウルト更新時のコンテキスト情報
			 */
			void UpdateUlt(float dt, const UltContext& ctx) { m_ultController.Update(dt, ctx); }

			/** 
			 * @brief ウルト発動中か
			 * @return true で発動中、false で非発動
			 */
			bool IsUltActive() const { return m_ultController.IsActive(); }

			/** 
			 * @brief ウルトが発動可能か
			 * @return true で発動可能、false で非発動
			 */
			bool CanActivateUlt() const { return m_ultController.CanActivate(); }

			/** 
			 * @brief クールダウン残量を 0.0〜1.0 で返す（UI表示用）
			 * @return クールダウン率（0.0 〜 1.0）
			 */
			float GetUltCooldownRate() const { return m_ultController.GetCooldownRate(); }


			//============================================//
			// 陣形切り替え演出（スライドUI用の入力ロック）
			//============================================//

			/**
			 * @brief 陣形切り替え演出（スライド）を開始する
			 * @details 演出中は IsSwitchingFormation() が true を返し、ChildPenguinManager 側で
			 *          連続切り替え入力を無視するために使う。
			 */
			void StartSwitchTransition() { m_switchLockTimer = m_switchLockDuration; }

			/**
			 * @brief 陣形切り替え演出中か
			 * @return true の間は再度の切り替え入力を無視すべき
			 */
			bool IsSwitchingFormation() const { return m_switchLockTimer > 0.0f; }

			/**
			 * @brief 切り替え演出の進行度を 0.0(開始)〜1.0(完了) で返す（UI表示用）
			 */
			float GetSwitchProgress() const { return 1.0f - (m_switchLockTimer / m_switchLockDuration); }

			/**
			 * @brief 切り替えロックのタイマーを更新する（切り替え時間のホットリロード監視も行う）
			 * @param dt 前フレームとの時間差（秒）
			 */
			void UpdateSwitchLock(float dt)
			{
#if defined(APP_DEBUG)
				m_tuningReloadTimer += dt;
				if (m_tuningReloadTimer >= kTuningReloadInterval)
				{
					m_tuningReloadTimer = 0.0f;
					ReloadSwitchTuningIfChanged();
				}
#endif
				m_switchLockTimer -= dt;
				if (m_switchLockTimer < 0.0f) m_switchLockTimer = 0.0f;
			}


		private:
			/** 陣形切り替え演出（スライド）時間のホットリロード対象JSONを再読み込みする（変更があれば） */
			void ReloadSwitchTuningIfChanged();


		private:
			static constexpr int FOLLOWERS_PER_LEVEL = 9;  /** レベルアップに必要なフォロワー数 */
#if defined(APP_DEBUG)
			static constexpr float kTuningReloadInterval = 1.0f;  /** チューニングJSONの変更チェック間隔（秒） */
#endif

			std::array<
				std::unique_ptr<IFormation>,
				static_cast<size_t>(EnFormationType::Num)> m_formations;	/** 陣形の種類ごとのインスタンスを保持する配列 */

			IFormation*     m_currentFormation = nullptr;					/** 現在の陣形インスタンスへのポインタ */
			EnFormationType m_currentType      = EnFormationType::Circle;	/** 現在の陣形種別 */
			int             m_formationLevel   = 0;							/** 現在の陣形レベル */

			std::function<void(int)> m_onLevelUp;  /** レベルアップ時のコールバック */

			UltController m_ultController;  /** ウルト発動・タイマー管理 */

			float m_switchLockTimer    = 0.0f;  /** 陣形切り替え演出の残り時間（秒）。0より大きい間は演出中 */
			float m_switchLockDuration = 0.2f;  /** 陣形切り替え演出（スライド）の時間（秒）。FormationSwitchTuning.jsonからホットリロード可能 */
#if defined(APP_DEBUG)
			time_t m_tuningLastWriteTime = 0;   /** チューニングJSONの最終更新日時（ホットリロード監視用） */
			float  m_tuningReloadTimer   = 0.0f;/** チューニングJSONの変更チェック用タイマー */
#endif
		};
	}
}
