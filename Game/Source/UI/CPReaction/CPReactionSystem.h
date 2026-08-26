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
		class Actor;
		class ChildPenguin;
		class DaddyPenguin;
		class Enemy;
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
			/** リアクション（吹き出し）の最大数 */
			static constexpr uint8_t REACTION_PACKET_NUM = 10;
			/**
			 * 頭上マーク（？/！）の最大数。
			 * 吹き出しと別枠にしてあるのは、マークが吹き出しのスロットを占有すると
			 * 「！」の直後に来る入隊の吹き出し（Happy）が優先度調停で棄却されてしまうため
			 */
			static constexpr uint8_t MARK_PACKET_NUM = 16;


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

			/**
			 * @brief リアクションの対象となるシロクマを設定
			 * @detail 吹き出しは固定色（グレー）で、SEは鳴らさない。
			 *         シロクマは背が高いので、アイコンの基準位置を頭上へ持ち上げる。
			 * @param enemy 対象のシロクマ
			 * @param type リアクションのタイプ（Question / Exclamation を想定）
			 */
			void SetEnemyTarget(actor::Enemy* enemy, const EnCPReactionType type);


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
			 * @brief 指定したアクターが既に使用しているスロットのインデックスを探索する
			 * @param target 検索対象のアクター
			 * @return 見つかったスロットのインデックス。見つからなければREACTION_PACKET_NUM
			 */
			uint8_t SearchExistingIndex(const actor::Actor* target) const;

			/**
			 * @brief 吹き出しスロットを確保する（使用中なら使い回し、優先度で調停する）
			 * @param target 対象のアクター
			 * @param priority 通知の優先度
			 * @return 確保したスロットのインデックス。優先度で棄却された場合はREACTION_PACKET_NUM
			 */
			uint8_t AcquireSlot(actor::Actor* target, const EnCPReactionPriority priority);

			/**
			 * @brief 頭上マーク（？/！）を設定する
			 * @details 吹き出しとは独立したスロットを使うため、同じアクターに
			 *          マークと吹き出しを同時に出せる。マークは揺れず、SEも鳴らさない。
			 * @param target 対象のアクター
			 * @param type リアクションのタイプ（Question / Exclamation）
			 * @param worldOffsetY アイコン基準位置へ足すワールドYオフセット
			 */
			void SetMarkTarget(actor::Actor* target, const EnCPReactionType type, const float worldOffsetY);

			/**
			 * @brief 指定アクターの頭上マークを即座に消す
			 * @details 「？」を出している子が親を見つけた（＝入隊した）瞬間に呼ぶ。
			 *          該当マークが無ければ何もしない。
			 * @param target 対象のアクター
			 */
			void ClearMark(actor::Actor* target);

			/**
			 * @brief 各リアクションスロットの座標変換を行い、Menuへ反映する
			 */
			void UpdateReactionPositions();


		private:
			/** リアクション（吹き出し）のSystemPacketの配列 */
			std::array<SystemPacket<CPReactionMenu>, REACTION_PACKET_NUM> m_reactionPackets;
			/** 各スロットに対応するターゲット（子ペンギンまたはシロクマ） */
			std::array<actor::Actor*, REACTION_PACKET_NUM> m_targets;
			/** 各スロットのアイコン基準位置へ足すワールドYオフセット（シロクマの背丈ぶん等） */
			std::array<float, REACTION_PACKET_NUM> m_targetWorldOffsetsY;

			/** 頭上マーク（？/！）のSystemPacketの配列（吹き出しとは独立） */
			std::array<SystemPacket<CPReactionMenu>, MARK_PACKET_NUM> m_markPackets;
			/** マーク各スロットに対応するターゲット */
			std::array<actor::Actor*, MARK_PACKET_NUM> m_markTargets;
			/** マーク各スロットのワールドYオフセット */
			std::array<float, MARK_PACKET_NUM> m_markWorldOffsetsY;
			/** 各スロットで現在表示中のリアクションの優先度 */
			std::array<EnCPReactionPriority, REACTION_PACKET_NUM> m_priorities;
			/** リアクションの親パラメータ */
			std::unique_ptr<CPReactionStatus> m_reactionStatusParent;
		};
	}
}
