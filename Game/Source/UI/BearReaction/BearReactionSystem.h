/**
 * @file BearReactionSystem.h
 * @brief クマのリアクションシステムクラス
 */
#pragma once
#include "BearReactionTypes.h"
#include "Source/UI/Modules/FrontChecker/FrontChecker.h"
#include "Source/UI/Modules/System/SystemPacket.h"


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class BearReactionMenu;


		/**
		 * @brief クマのリアクションシステムクラス
		 */
		class BearReactionSystem
		{
		public:
			/** @brief リアクションの数を設定 */
			void SetReactionNum(const uint8_t num)
			{
				K2_ASSERT(m_reactionNum == 0, "設定済み");
				K2_ASSERT(num > 0, "無効値");
				m_reactionNum = num;
			}


			/**
			 * @brief リアクションを設定
			 * @param index リアクションのインデックス
			 * @param position リアクションの座標
			 * @param daddyTRS 親ペンギンのTransform
			 * @param type リアクションのタイプ
			 */
			void SetReaction(
				const uint8_t index,
				const Vector3& position,
				const core::Transform& daddyTRS,
				const EnBearReactionType type
			);


		public:
			/** 初期化 */
			void Initialize();


			/** 更新 */
			void Update();


			/** 描画 */
			void Render(RenderContext& rc);


		public:
			BearReactionSystem();
			~BearReactionSystem();


		private:
			/** リアクション情報 */
			struct ReactionInfo
			{
				UIPacket<BearReactionMenu> packet;
				bool isActive;


				ReactionInfo();
				~ReactionInfo();
			};


			/** リアクション情報の配列 */
			std::vector<std::unique_ptr<ReactionInfo>> m_reactions;
			/** リアクションの数 */
			uint8_t m_reactionNum;
		};
	}
}