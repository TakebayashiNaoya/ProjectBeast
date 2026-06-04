/**
 * @file CPReactionMenu.h
 * @brief 子ペンギンのリアクションUIクラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
	}


	namespace ui
	{
		/** 前方宣言 */
		class CPReactionStatus;
		class CPReactionAnimStatus;


		/**
		 * @brief リアクションのタイプ
		 */
		enum class EnReactionType : uint8_t
		{
			Trouble,
			Happy,
			None
		};


		/**
		 * @brief リアクションの描画フェーズ
		 */
		enum class EnDrawPhase : uint8_t
		{
			Drawing,
			ToClear,
			None
		};


		/**
		 * @brief 子ペンギンのリアクションUIクラス
		 */
		class CPReactionMenu : public MenuBase
		{
			using CPReactionClass = MenuBase;


		public:
			/**
			 * @brief ステータスを設定
			 * @param status ステータス
			 */
			inline void SetStatus(CPReactionStatus* status)
			{
				m_status = status;
			}
			/**
			 * @brief ターゲットの子ペンギンを設定
			 * @param target ターゲットの子ペンギン
			 */
			inline void SetTarget(actor::ChildPenguin* target)
			{
				m_target = target;
			}
			/**
			 * @brief UIAnimationを再生する
			 */
			void PlayUIAnimation(const EnReactionType type);


		public:
			/**
			 * @brief ターゲットの子ペンギンを取得
			 * @return ターゲットの子ペンギン
			 */
			inline actor::ChildPenguin* GetTarget() const
			{
				return m_target;
			}
			/**
			 * @brief リアクションのタイプを取得
			 * @return リアクションのタイプ
			 */
			inline EnReactionType GetReactionType() const
			{
				return m_type;
			}


		public:
			CPReactionMenu();
			~CPReactionMenu();

			void Update()override;
			void InitializeLogic()override;


		private:
			/**
			 * @brief ターゲットの子ペンギンの位置にUIを移動させる
			 */
			void PositionUpdate();
			/**
			 * @brief リアクションの描画フラグを更新する
			 */
			void DrawFlagUpdate();
			/**
			 * @brief アイコンをリセットする
			 */
			void ResetIcon();
			/**
			 * @brief アニメーションをセットする
			 * @param icon アニメーションをセットするアイコン
			 */
			void SetAnimation();
			/**
			 * @brief アニメーションを更新する
			 */
			void UpdateAnimation();


		private:
			/** ステータス */
			CPReactionStatus* m_status;
			/** アニメーションステータス */
			std::unique_ptr<CPReactionAnimStatus> m_animStatus;

			/** ターゲット*/
			actor::ChildPenguin* m_target;

			/** 吹き出し */
			UIIcon* m_speechBubble;
			/** 困り */
			UIIcon* m_troubleReaction;
			/** 喜び */
			UIIcon* m_happyReaction;

			/** リアクションのタイプ */
			EnReactionType m_type;

			/** タイマー */
			float m_timer;
			/** アニメーション再生中フラグ */
			bool m_isPlayAnimation;
		};
	}
}