/**
 * @file SystemPacket.h
 * @brief Layoutを複数保持するUIのためのカプセル化クラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Layout.h"
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief Layoutを複数保持するUIのためのカプセル化
		 * @tparam TMenu Menuの具象クラス
		 */
		template <typename TMenu>
		class SystemPacket : public Noncopyable
		{
		public:
			SystemPacket()
				: m_layout(nullptr)
				, m_menu(nullptr)
			{}
			~SystemPacket()
			{
				// MenuはLayoutがユニークポインタで所有しているため、削除の必要なし
			}


		public:
			/**
			 * @brief 初期化
			 * @param jsonFilePath Layoutのjsonファイルのパス
			 * @details Layoutを生成してMenuを取得する
			 */
			void Initialize(const char* jsonFilePath)
			{
				m_layout.reset();
				m_layout = nullptr;
				m_layout = std::make_unique<Layout>();
				m_layout->Initialize<TMenu>(jsonFilePath);
				m_menu = m_layout->GetMenu<TMenu>();
			}


			/** @brief 更新 */
			void Update()
			{
				if (m_layout) m_layout->Update();
			}


			/** @brief 描画 */
			void Render(RenderContext& rc)
			{
				if (m_layout) m_layout->Render(rc);
			}


		public:
			/** @brief Layoutを取得する */
			inline Layout* GetLayout() const { return m_layout.get(); }
			/** @brief Menuを取得する */
			inline TMenu* GetMenu() const { return m_menu; }


		private:
			/** レイアウト */
			std::unique_ptr<Layout> m_layout;
			/** メニュー */
			TMenu* m_menu;
		};


		/**
		 * @brief SystemPacketのユニークポインタ
		 * @details SystemPacketをstd::unique_ptrで管理したいため、using宣言で省略形を定義
		 */
		template <typename TMenu>
		using UIPacket = std::unique_ptr<SystemPacket<TMenu>>;


		/**
		 * @brief SystemPacketの初期化関数
		 * @param packet 初期化するSystemPacketのユニークポインタ
		 * @param path Layoutのjsonファイルのパス
		 * @details SystemPacketを生成して初期化する
		 */
		template <typename TMenu>
		inline void InitUIPacket(UIPacket<TMenu>& packet, const char* path)
		{
			packet = std::make_unique<SystemPacket<TMenu>>();
			packet->Initialize(path);
		}
	}
}


