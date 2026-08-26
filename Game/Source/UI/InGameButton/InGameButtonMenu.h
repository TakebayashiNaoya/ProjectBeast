/**
 * @file InGameButtonMenu.h
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/InGameButton/InGameButtonGaugeAnimStatus.h"
#include "Source/UI/InGameButton/InGameButtonGaugeStatus.h"
#include "Source/UI/Modules/InGameStartingAnimLogic/InGameStartingAnimLogic.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲーム中にボタンメニューを表示するMenuクラス
		 * @details ゲーム中にボタンの操作方法を表示するUIを管理するクラスです。
		 */
		class InGameButtonMenu : public MenuBase
		{
		public:
			InGameButtonMenu();
			~InGameButtonMenu() override = default;
			void Update() override;
			void InitializeLogic() override;

			/**
			 * @brief ジャンプのスタミナ状態を設定する（毎フレーム外部から渡される）
			 * @param ratio 0.0(空)〜1.0(満タン)の割合
			 * @param isLocked クールダウン中かどうか
			 */
			inline void SetJumpStaminaInfo(float ratio, bool isLocked)
			{
				m_jumpStaminaRatio = ratio;
				m_isJumpStaminaLocked = isLocked;
			}

			/**
			 * @brief Y（再集合の呼びかけ）のクールダウン状態を設定する（毎フレーム外部から渡される）
			 * @param ratio 0.0(使用直後)〜1.0(使用可能)の割合
			 * @param isLocked クールダウン中かどうか
			 */
			inline void SetRegroupCooldownInfo(float ratio, bool isLocked)
			{
				m_regroupCooldownRatio = ratio;
				m_isRegroupCooldownLocked = isLocked;
			}


		private:
			/** ボタンのアイコンの更新処理 */
			void ButtonIconUpdate();

			/** スニーク関連アイコンのカラーをグレー / 通常に切り替える */
			void UpdateSneakIconColor();

			/** ジャンプスタミナ・Yクールダウンのサークルゲージの更新処理（追従イージング＋色演出） */
			void UpdateStaminaGauge();

			/**
			 * @brief クマに襲われている間のYボタン強調表示の更新
			 * @details クマが子を追っていて再集合が使えるときだけ、
			 *          Yボタンとメガホンアイコンを黄色く脈動させる。
			 */
			void UpdateYButtonEmphasis();

			bool IsInputAButton() const;
			bool IsInputBButton() const;
			bool IsInputXButton() const;
			bool IsInputYButton() const;

			InGameStartingAnimLogic m_startingAnimLogic;

			/** スタミナゲージ専用のステータス（追従速度などの数値設定） */
			std::unique_ptr<InGameButtonGaugeStatus> m_gaugeStatus;
			/** スタミナゲージ専用のアニメーションステータス（色演出） */
			std::unique_ptr<InGameButtonGaugeAnimStatus> m_gaugeAnimStatus;

			/** ジャンプスタミナの割合(0〜1、実値) */
			float m_jumpStaminaRatio = 1.0f;
			/** ジャンプスタミナがクールダウン中かどうか */
			bool m_isJumpStaminaLocked = false;
			/** ジャンプスタミナの表示用割合(0〜1、なめらかに追従する値) */
			float m_jumpDisplayRatio = 1.0f;
			/** 直前フレームでジャンプスタミナがロック中だったか（エッジ検出用） */
			bool m_wasJumpStaminaLocked = false;

			/** Yクールダウンの回復割合(0〜1、実値。1.0で使用可能) */
			float m_regroupCooldownRatio = 1.0f;
			/** Yがクールダウン中かどうか */
			bool m_isRegroupCooldownLocked = false;
			/** Yクールダウンの表示用割合(0〜1、なめらかに追従する値) */
			float m_regroupDisplayRatio = 1.0f;
			/** 直前フレームでYがクールダウン中だったか（エッジ検出用） */
			bool m_wasRegroupCooldownLocked = false;

			/** Yボタン強調の脈動用タイマー（常時加算） */
			float m_yEmphasisTimer = 0.0f;
			/** Y強調対象アイコンのJSON基準スケール（初回に取得。上書きすると基準0.8のアイコンが常に拡大されるため） */
			std::array<Vector3, 4> m_yEmphasisBaseScales =
				{ Vector3::One, Vector3::One, Vector3::One, Vector3::One };
			/** m_yEmphasisBaseScales を取得済みか（アイコンごと） */
			std::array<bool, 4> m_isYEmphasisBaseCaptured = {};
		};

	}
}