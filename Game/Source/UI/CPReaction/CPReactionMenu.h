/**
 * @file CPReactionMenu.h
 * @brief 子ペンギンのリアクションUIクラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"


namespace app
{
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
		 * @brief 子ペンギンのリアクションUIクラス
		 * @detail ターゲットの管理・前方判定・座標変換はCPReactionSystemが行う。
		 *         Menuは渡された座標・フラグを自身のアイコンへ反映するのみ。
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
			 * @brief アイコンのスクリーン座標を設定する
			 * @param screenPosition スクリーン座標(x, y)。zは未使用。
			 */
			void SetTargetPosition(const Vector3& screenPosition);

			/**
			 * @brief 描画するかどうかを設定する
			 * @detail m_typeがNoneの場合は常に非表示になる
			 * @param isDraw 描画するかどうか(前方判定などの結果をSystemから渡す)
			 */
			void SetIsDraw(const bool isDraw);

			/**
			 * @brief リアクションを開始し、UIAnimationを再生する
			 * @param type リアクションのタイプ
			 * @param cpType 対象の子ペンギンのタイプ(吹き出し色の決定に使用)
			 */
			void PlayUIAnimation(const EnReactionType type, const actor::EnChildPenguinType cpType);


		public:
			/**
			 * @brief リアクションのタイプを取得
			 * @detail Noneであれば、このMenuは空き状態(ターゲット未設定)であることを示す
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
			 * @brief タイマーの更新とアニメーションの再生・自動終了処理
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
			/** 描画するかどうか(Systemから渡される前方判定の結果) */
			bool m_isDraw;
		};
	}
}
