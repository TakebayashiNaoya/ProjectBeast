/**
 * @file SearchMenu.h
 * @brief シロクマがプレイヤーを見つけるか見つけないかの動的処理クラス
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/Actor/Character/Enemy/Enemy.h"


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class SearchStatus;

		class SearchMenu : public MenuBase
		{
		public:
			SearchMenu();
			~SearchMenu();
			void Update() override;
			void InitializeLogic() override;
			
			/**
			 * @brief 見つけているか見つけていないかの内部的処理をまとめる用
			 */
			void Searching();
			
			/**
			 * @brief アクティブの取得
			 * @return m_isActive アクティブの取得
			 */
			bool IsActive()const { return m_isActive; }
			/**
			 * @brief アクティブの設定
			 * @param isActive アクティブに応じての設定
			 */
			void SetIsActive(bool isActive) { m_isActive = isActive; }
			/**
			 * @brief 敵の情報の設定
			 * @param enemy 敵の情報の設定
			 */
			void SetEnemy(actor::Enemy* enemy) { m_enemy = enemy; }

			/**
			 * @brief 描画の設定
			 * @param isDraw 描画フラグ
			 */
			void SetDraw(const bool isDraw) { m_isDraw = isDraw; }


		private:
			/** シロクマの索敵・追跡のステータスをunique_ptrで所有する */
			std::unique_ptr<SearchStatus> m_searchStatus;
			actor::Enemy* m_enemy = nullptr;
			bool m_isDraw = false;
			bool m_isActive = false;
		};
	}
}
