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
		enum class EnSearchType : uint8_t
		{
			CanFind,
			CanNotFind,
			//CanFindFrame,
			//CanNotFindFrame,
			Max
		};


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
			 * @brief タイプの取得
			 * @return m_currentType タイプの取得
			 */
			EnSearchType GetType()const { return m_currentType; }
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
			bool m_isActive;
			bool m_canFind;

			actor::Enemy* m_enemy;
			EnSearchType m_currentType;

			//using Icon = std::unique_ptr<SearchIcon>;
			//using Key = uint32_t;
			//std::unordered_map<Key, Icon>m_searchIconMap;
		};
	}
}
