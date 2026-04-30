/**
 * @file SearchMenu.h
 * @brief シロクマがプレイヤーを見つけるか見つけないかの動的処理クラス
 * @author 忽那
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
			using SearchClass = MenuBase;
		public:
			SearchMenu();
			~SearchMenu();
			void Update()override;
			void InitializeLogic()override;
			
			/**
			 * @brief 見つけているか見つけていないかの内部的処理をまとめる用
			 */
			void Searching();
			
			/**
			 * @brief アイコンとフレームの描画のオンオフをまとめる用
			 * @param isDraw アイコンとフレームの描画の設定
			 */
			void SetAllIconActive(bool isDraw);

			/**
			 * @brief アクティブの取得
			 * @return m_isActive アクティブの取得
			 */
			inline bool IsActive()const { return m_isActive; }
			/**
			 * @brief アクティブの設定
			 * @param isActive アクティブに応じての設定
			 */
			inline void SetIsActive(bool isActive) { m_isActive = isActive; }
			/**
			 * @brief 敵の情報の設定
			 * @param enemy 敵の情報の設定
			 */
			void SetEnemy(actor::Enemy* enemy) { m_enemy = enemy; }
			/**
			 * @brief 見つけるか見つけないかのフラグの設定
			 * @param canFind 見つけるか見つけないかのフラグの設定
			 */
			inline void SetCanFind(bool canFind) { m_canFind = canFind; }
			/**
			 * @brief 見つけるか見つけないかのフラグの取得
			 * @return m_canFind 見つけるか見つけないかのフラグの取得
			 */
			inline bool CanFind()const { return m_canFind; }


		private:
			/** シロクマの追跡・索敵のステータスの生ポインタ */
			std::unique_ptr<SearchStatus> m_searchStatus;
			actor::Enemy* m_enemy;
			bool m_isActive;
			bool m_canFind;
		};
	}
}
