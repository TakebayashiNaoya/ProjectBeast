/**
 * @file Menu.h
 * @brief UIパーツ周りから動的な処理を行う基底クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/UIParts.h"


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class UIBase;
		class UICanvas;

		/**
		 * @brief UIパーツ周りから動的な処理を行う基底クラス
		 */
		class MenuBase : public Noncopyable
		{
		public:
			MenuBase(){}
			virtual ~MenuBase() { Clear(); }


			/** 更新処理 */
			void Update()
			{
				if (m_canvas)
				{
					m_canvas->Update();
				}
			}


			/** 描画機能処理 */
			void Render(RenderContext& rc)
			{
				if (m_canvas)
				{
					m_canvas->Render(rc);
				}
			}


		public:
			/**
			 * @brief キャンバスを設定する
			 * @param canvas　新しいキャンバスを設定
			 */
			void SetCanvas(UICanvas* canvas) { m_canvas.reset(canvas); }
			
			
			/**
			 * @brief キャンバスを取得
			 * @return キャンバスのポインタを取得
			 */
			UICanvas* GetCanvas() { return m_canvas.get(); }


			/** 
			 * @brief UIを登録
			 * @param key キー
			 * @param ui UI
			 */
			void RegisterUI(const uint32_t key, UIBase* ui)
			{
				m_uiMap.emplace(key, ui);
			}


			/**
			 * @brief UIの登録解除
			 * @param key キー
			 */
			void UnregisterUI(const uint32_t key)
			{
				m_uiMap.erase(key);
			}


			/**
			 * @brief UIの取得
			 * @brief コード側から特定のボタンやテキストを取得する用
			 */
			template<typename T>
			T* GetUI(const uint32_t& key)
			{
				if (m_uiMap.count(key))
				{
					return dynamic_cast<T*>(m_uiMap[key]);
				}
				return nullptr;
			}


			/** UIが存在するかどうか */
			bool HasUI(const uint32_t& key)const
			{
				return m_uiMap.count(key) > 0;
			}


			/** UIの消去(クリア) */
			void Clear()
			{
				m_uiMap.clear();
			}


			/** 
			 * @brief UIのロジック初期化処理
			 * @brief ここに「ボタンが押された時の処理」等を書く
			 */
			virtual void IntializeLogic()
			{
			}


		private:
			/** UICanvasを生成 */
			std::unique_ptr<UICanvas>m_canvas = nullptr;
			/** キーと値の保持 */
			std::unordered_map<uint32_t, UIBase*>m_uiMap;
		};
	}
}


