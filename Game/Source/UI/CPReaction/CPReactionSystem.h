/**
 * @file CPReactionSystem.h
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#pragma once
#include "CPReactionMenu.h"
#include "CPReactionTypes.h"

#include "Source/UI/Modules/System/SystemPacket.h"


namespace app
{
	namespace actor
	{
		class ChildPenguin;
		class DaddyPenguin;
	}


	namespace ui
	{
		/** 前方宣言 */
		class Layout;
		class CPReactionMenu;
		class CPReactionStatus;


		/**
		 * @brief 子ペンギンのリアクションシステムクラス
		 * @detail ターゲットの管理・座標変換をここで行う。
		 *         リアクションのタイプ（Trouble/Happy等）の判定は行わない。
		 *         型の確定は呼び出し側（BattleManager経由の通知元）の責務であり、
		 *         Systemは受け取ったtypeとMenuへの反映のみを担う。
		 *         ただし、同一ターゲットに対して同フレーム内で複数回通知された場合の
		 *         優先度による調停（EnCPReactionPriority）はここで行う。
		 */
		class CPReactionSystem : Noncopyable
		{
		private:
			/** リアクションの最大数 */
			static constexpr uint8_t REACTION_PACKET_NUM = 10;


		public:
			/**
			 * @brief リアクションの対象となる子ペンギンを設定
			 * @detail 同じ子ペンギンが既にスロットを使用中であれば、そのスロットを使い回す。
			 *         同じtypeが指定された場合はアニメーションを再スタートしない。
			 *         既に表示中のリアクションより優先度が低い通知は無視する
			 *         （NotifyCPReactionChangedの呼び出し順序に結果が依存しないようにするため）。
			 * @param childPenguin 対象の子ペンギン
			 * @param type リアクションのタイプ（呼び出し側で確定済みの値）
			 * @param priority 通知の優先度（省略時はNormal）
			 */
			void SetTarget(
				actor::ChildPenguin* childPenguin,
				const EnCPReactionType type,
				const EnCPReactionPriority priority = EnCPReactionPriority::Normal
			);


		public:
			CPReactionSystem();
			~CPReactionSystem();


		public:
			/** @brief 初期化処理 */
			void Initialize();
			/** @brief 更新処理 */
			void Update();
			/** @brief 描画処理 */
			void Render(RenderContext& rc);


		private:
			/**
			 * @brief 空いているリアクションスロットのインデックスを探索する
			 * @detail 見つからなければ先頭(0)を返す(上書き)
			 */
			uint8_t SearchTargettableIndex() const;

			/**
			 * @brief 指定した子ペンギンが既に使用しているスロットのインデックスを探索する
			 * @param childPenguin 検索対象の子ペンギン
			 * @return 見つかったスロットのインデックス。見つからなければREACTION_PACKET_NUM
			 */
			uint8_t SearchExistingIndex(const actor::ChildPenguin* childPenguin) const;

			/**
			 * @brief 各リアクションスロットの座標変換を行い、Menuへ反映する
			 */
			void UpdateReactionPositions();


		private:
			/** リアクションのSystemPacketの配列 */
			std::array<SystemPacket<CPReactionMenu>, REACTION_PACKET_NUM> m_reactionPackets;
			/** 各スロットに対応するターゲットの子ペンギン */
			std::array<actor::ChildPenguin*, REACTION_PACKET_NUM> m_targets;
			/** 各スロットで現在表示中のリアクションの優先度 */
			std::array<EnCPReactionPriority, REACTION_PACKET_NUM> m_priorities;
			/** リアクションの親パラメータ */
			std::unique_ptr<CPReactionStatus> m_reactionStatusParent;
		};
	}
}
