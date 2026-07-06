/**
 * @file CPReactionSystem.h
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#pragma once
#include "CPReactionMenu.h"

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
		 * @detail ターゲットの管理・前方判定・座標変換をここで行い、
		 *         Menuには最終的な座標・描画フラグのみを渡す。
		 */
		class CPReactionSystem : Noncopyable
		{
		private:
			/** リアクションの最大数 */
			static constexpr uint8_t REACTION_PACKET_NUM = 10;


		public:
			/**
			 * @brief リアクションの対象となる子ペンギンを設定
			 * @param childPenguin 対象の子ペンギン
			 * @param type リアクションのタイプ
			 */
			void SetTarget(actor::ChildPenguin* childPenguin, const EnReactionType type);


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
			 * @brief 各リアクションスロットの前方判定・座標変換を行い、Menuへ反映する
			 */
			void UpdateReactionPositions();


		private:
			/** リアクションのSystemPacketの配列 */
			std::array<SystemPacket<CPReactionMenu>, REACTION_PACKET_NUM> m_reactionPackets;
			/** 各スロットに対応するターゲットの子ペンギン */
			std::array<actor::ChildPenguin*, REACTION_PACKET_NUM> m_targets;
			/** リアクションの親パラメータ */
			std::unique_ptr<CPReactionStatus> m_reactionStatusParent;
		};
	}
}
