/**
 * @file Layout.h
 * @brief UIのレイアウト管理
 * @author 忽那
 */
#pragma once
#include "Menu.h"
#include "Json/json.hpp"


#ifdef APP_DEBUG
	#define APP_ENABLE_LAYOUT_HOTRELOAD
#endif


namespace app
{
	namespace ui
	{
		/**
		 * @brief UIのレイアウト管理クラス
		 */
		class Layout : public Noncopyable
		{
		public:
			Layout(){}
			~Layout(){}


		public:
			template<typename T>
			void Initialize(const std::string& path)
			{
				m_filePath = path;
				m_menu = std::make_unique<T>();
				Reload();
			}


			/** 更新処理 */
			void Update();
			/** 描画機能処理 */
			void Render(RenderContext& rc);
			/**
			 * @brief Jsonを読み込み直して、UICanvasやその要素を作り直すまたは更新処理
			 */
			void Reload();

			/**
			 * @brief MenuBaseを取得
			 * @return MenuBaseのポインタを取得
			 * @datil
			 * Menuの内容を使う時毎回dynamic_castするのは面倒なので、GetMenu<具象クラス>()で直接具象クラスのポインタを取得できるようにする
			 */
			template<typename T>
			T* GetMenu() { return dynamic_cast<T*>(m_menu.get()); }
			

		private:
#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			time_t m_lastUpdateTime = 0;
#endif //APP_ENABLE_LAYOUT_HOTRELOAD

			std::string m_filePath = "";
			std::unique_ptr<MenuBase>m_menu = nullptr;


		private:
			static UIBase* CreateUI(
					UICanvas* canvas
				,	const std::string& type
				,	const uint32_t key
				,	nlohmann::json& item
			);
		};
	}
}