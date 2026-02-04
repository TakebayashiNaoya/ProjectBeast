/**
 * @file UIBase.h
 * @brief UIの基本的なクラス
 * @author 忽那
 */

#pragma once
#include "Source/Core/HierarchicalTransform.h"


namespace app 
{
	namespace ui
	{
		/**
		 * @brief UIBaseの基底クラス
		 */
		class UIBase : public IGameObject
		{
		public:
			/** トランスフォーム */
			core::HierarchicalTransform m_transform;


		public:
			UIBase();
			virtual ~UIBase();


			/** 初期化処理 */
			virtual bool Start() = 0;
			/** 更新処理 */
			virtual void Update() = 0;
			/** 描画処理 */
			virtual void Render(RenderContext& rc) = 0;


		private:
			/** α値 */
			Vector4 m_color;
		};



		/**
		 * @brief UIの画像を扱うUIBaseの派生クラス
		 */
		class UIImage : public UIBase
		{
			/** friendclassの宣言 */
			friend class UICanvas;


		public:
			/**
			 * @brief スプライトレンダーの取得
			 * @return スプライトレンダーのポインタ
			 */
			SpriteRender* GetSpriteRender() { return &m_spriteRender; }


		protected:
			/** スプライトレンダー */
			SpriteRender m_spriteRender;


		protected:
			UIImage();
			~UIImage();


			/** 初期化処理 */
			virtual bool Start()override;
			/** 更新処理 */
			virtual void Update()override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override;

			/**
			 * @brief 初期化
			 * @param assetName アセット名
			 * @param wide 横幅
			 * @param height 縦幅
			 * @param position 座標
			 * @param scale 拡大縮小
			 * @param rotation 回転
			 */
			void Initialize(
					const char* assetName
				,	const float wide
				,	const float height
				,	const Vector3& position
				,	const Vector3& scale
				,	const Quaternion& rotation
			);
		};



		/**
		 * @brief 数字のスプライトのみを扱うUIBaseの派生クラス
		 */
		class UIDigit : public UIBase
		{
		public:
			UIDigit();
			~UIDigit();


		public:
			/** 初期化処理 */
			virtual bool Start()override;
			/** 更新処理 */
			virtual void Update()override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override;


			/**
			 * @brief 数字のスプライトの初期化
			 * @param assetName アセット名
			 * @param digit 何桁かの情報(数)
			 * @param number 表示する数
			 * @param wide 横幅
			 * @param height 縦幅
			 * @param position 位置
			 * @param scale 大きさ
			 * @param rotation 回転
			 */
			void Initialize(
				const char* assetName,
				const int digit,
				const int number, 
				const float wide, 
				const float height, 
				Vector3& position,
				Vector3& scale, 
				Quaternion& rotation
			);


			/**
			 * @brief 数字の設定
			 * @param number 数
			 */
			void SetNumber(const int number) { m_requestNumber = number; }
			/** 
			 * @brief スプライトレンダー配列の取得
			 * @return スプライトレンダー配列の参照
			 */
			std::vector<SpriteRender*>& GetSpriteRenderList() { return m_renderList; }
			/**
			 * @brief スプライトの配列のラムダ式
			 * @param func 全てのスプライトレンダーに対して何を行うか
			 */
			void ForEach(const std::function<void(SpriteRender*)>& func)
			{
				for (auto* render : m_renderList)
				{
					func(render);
				}
			}
			 

		private:
			/** 画像表示用の配列 */
			std::vector<SpriteRender*> m_renderList;


			/** 表示用の数字 */
			int m_number;
			int m_requestNumber;
			int m_digit;


			/** 数字用に必要な画像の名前 */
			std::string m_assetsPath;


			/** 横幅 */
			float m_wide;
			/** 縦幅 */
			float m_height;


			/**
			 * @brief 桁数の更新処理
			 * @param targetDigit 対象の桁
			 * @param number 数
			 */
			void UpdateNumber(const int targetDigit, const int number);
			/**
			 * @brief 桁の更新処理
			 * @param index 要素数
			 */
			void UpdatePosition(const int index);


			/** 
			 * @brief 対象の桁
			 * @param digit 桁
			 */
			int GetDigit(int digit);
		};



		/**
		 * @brief UI作成時、UICanvasを追加する
		 */
		class UICanvas : public UIBase
		{
			/** friendclassの宣言 */
			friend class UIBase;
			friend class UIImage;
			friend class UIDigit;


		public:
			UICanvas();
			~UICanvas();


			/** 初期化処理 */
			bool Start();
			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


			/**
			 * @brief UIの生成
			 * @tparam T 派生クラス
			 * @return 生成されたインスタンスを返す
			 */
			template<typename T>
			T* CreateUI()
			{
				T* ui = new T();
				return ui;
			}


		private:
			/** 描画するUIBaseのリスト */
			std::vector<UIBase*> m_uiList;
		};
	}
}
