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
			 * @brief スライドのスタミナ状態を設定する（毎フレーム外部から渡される）
			 * @param ratio 0.0(空)〜1.0(満タン)の割合
			 * @param isLocked クールダウン中かどうか
			 */
			inline void SetSlideStaminaInfo(float ratio, bool isLocked)
			{
				m_slideStaminaRatio = ratio;
				m_isSlideStaminaLocked = isLocked;
			}


		private:
			/** ボタンのアイコンの更新処理 */
			void ButtonIconUpdate();

			/** スニーク関連アイコンのカラーをグレー / 通常に切り替える */
			void UpdateSneakIconColor();

			/** ジャンプ・スライドのスタミナサークルゲージの更新処理（追従イージング＋色演出） */
			void UpdateStaminaGauge();

			/** 命令標識（表:GO / 裏:WAIT）の更新処理（命令の切り替わり検出＋回転演出） */
			void UpdateCommandSign();

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

			/** スライドスタミナの割合(0〜1、実値) */
			float m_slideStaminaRatio = 1.0f;
			/** スライドスタミナがクールダウン中かどうか */
			bool m_isSlideStaminaLocked = false;
			/** スライドスタミナの表示用割合(0〜1、なめらかに追従する値) */
			float m_slideDisplayRatio = 1.0f;
			/** 直前フレームでスライドスタミナがロック中だったか（エッジ検出用） */
			bool m_wasSlideStaminaLocked = false;

			/** 命令標識のJsonで指定された等倍時のスケール（回転演出の基準値） */
			Vector3 m_signBaseScale = Vector3::One;
			/** 命令標識が回り始めてからの経過時間(秒) */
			float m_signFlipTimer = 0.0f;
			/** 命令標識が回転中かどうか */
			bool m_isSignFlipping = false;
			/** 直前フレームの命令が待機命令だったか（切り替わりのエッジ検出用） */
			bool m_wasWaitCommand = false;
		};

	}
}