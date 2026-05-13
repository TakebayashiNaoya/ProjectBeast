/**
 * @file CPReactionSystem.h
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#pragma once
#include "CPReactionMenu.h"
#include "Source/UI/Model/CPReactionStatus.h"


namespace app
{
	namespace actor
	{
		class ChildPenguin;
	}


	namespace ui
	{
		namespace
		{
			/** リアクションの最大数 */
			constexpr uint8_t MAX_REACTIONS_NUM = 10;
		}


		/** 前方宣言 */
		class Layout;
		class CPReactionMenu;



		/**
		 * @brief 子ペンギンのリアクションシステムクラス
		 */
		class CPReactionSystem : Noncopyable
		{
		public:
			/**
			 * @brief リアクションの対象となる子ペンギンを設定
			 * @param childPenguin 対象の子ペンギン
			 * @param type リアクションのタイプ
			 */
			void SetTarget(actor::ChildPenguin* childPenguin, const EnReactionType type);


		public:
			/** @brief リアクションメニューの配列を取得 */
			std::array<CPReactionMenu*, MAX_REACTIONS_NUM>& GetReactionMenus() { return m_reactionMenus; }


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
			/** リアクションレイアウトの配列 */
			std::array<Layout*, MAX_REACTIONS_NUM> m_reactionLayouts;
			/** リアクションメニューの配列 */
			std::array<CPReactionMenu*, MAX_REACTIONS_NUM> m_reactionMenus;
			/** リアクションの親パラメータ */
			std::unique_ptr<CPReactionStatus> m_reactionStatusParent;
		};
	}
}