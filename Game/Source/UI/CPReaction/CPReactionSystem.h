/**
 * @file CPReactionSystem.h
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#pragma once
#include "CPReactionMenu.h"

#include "Source/UI/System/SystemPacket.h"


namespace app
{
	namespace actor
	{
		class ChildPenguin;
	}


	namespace ui
	{
		/** 前方宣言 */
		class Layout;
		class CPReactionMenu;
		class CPReactionStatus;



		/**
		 * @brief 子ペンギンのリアクションシステムクラス
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
			/** @brief リアクションメニューの配列を取得 */
			const std::array<SystemPacket<CPReactionMenu>, REACTION_PACKET_NUM>& GetReactionMenus() const { return m_reactionPackets; }


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
			 * @brief ターゲットの子ペンギンに対応するリアクションMenuを探索して返す
			 * @return 対象の子ペンギンに対応するリアクションMenuのポインタ。
			 */
			CPReactionMenu* SearchTargettableMenu();


		private:
			/** リアクションのSystemPacketの配列 */
			std::array<SystemPacket<CPReactionMenu>, REACTION_PACKET_NUM> m_reactionPackets;
			/** リアクションの親パラメータ */
			std::unique_ptr<CPReactionStatus> m_reactionStatusParent;
		};
	}
}