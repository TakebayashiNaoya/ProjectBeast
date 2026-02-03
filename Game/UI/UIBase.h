#pragma once
#include "Source/Core/HierarchicalTransform.h"


namespace app {
	namespace ui {
		
		
		/**
		 * UI関連のベース
		 */
		class UIBase : public IGameObject
		{
		public:
			/** トランスフォーム */
			core::HierarchicalTransform m_transform;


		private:
			/** α値(白色) */
			Vector4 m_color = Vector4::White;


			bool m_isStart = false; //アニメーション開始フラグ
			bool m_isStop = true;   //アニメーション中断フラグ
			bool m_isUpdate = true; //画像更新フラグ
			bool m_isDraw = false;  //画像描画フラグ


		public:
			UIBase();
			virtual ~UIBase();


			virtual bool Start() = 0;
			virtual void Update() = 0;
			virtual void Render(RenderContext& rc) = 0;
		};



		/**
		 * UIで画像を扱う場所
		 */
		class UIImage : public UIBase
		{
			friend class UICanvas;


		protected:
			SpriteRender m_spriteRender;


		protected:
			UIImage();
			~UIImage();


			virtual bool Start()override;
			virtual void Update()override;
			virtual void Render(RenderContext& rc)override;


		public:
			/** ゲッター */
			SpriteRender* GetSpriteRender() { return &m_spriteRender; }


			/**
			 * 初期化
			 * ・アセット名
			 * ・横幅
			 * ・縦幅
			 * ・座標
			 * ・拡大縮小
			 * ・回転
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
		 * 数字の桁数表示用のUI関連
		 * スコア表示などで使用
		 */
		class UIDigit : public UIBase
		{
		private:
			/** 画像表示の可変長配列 */
			std::vector<SpriteRender*> m_renderList;

			/** 表示用の数字 */
			int m_number;
			int m_requestNumber;
			int m_digit;


			/** 数字表示に必要な画像の名前 */
			std::string m_assetsPath;

			/** 横幅×縦幅 */
			float m_wide;
			float m_height;


		public:
			UIDigit();
			~UIDigit();


			/**
			 * アセット名
			 * 何桁かの情報(数)
			 * 表示する数
			 * 横幅
			 * 縦幅
			 * 位置
			 * 大きさ
			 * 回転
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

			/** 数字の設定 */
			void SetNumber(const int number) { m_requestNumber = number; }

			/** Spriteのゲッター */
			std::vector<SpriteRender*>& GetSpriteRender() { return m_renderList; }

			/** Spriteのラムダ式 */
			void ForEach(const std::function<void(SpriteRender*)>& func)
			{
				for (auto* render : m_renderList)
				{
					func(render);
				}
			}


		public:
			virtual bool Start()override;
			virtual void Update()override;
			virtual void Render(RenderContext& rc)override;


		private:
			/**  */
			void UpdateNumber(const int targetDigit, const int number);
			void UpdatePosition(const int index);

			/** 対象の桁 */
			int GetDigit(int digit);
		};



		/**
		 * キャンバス
		 * UIを作る時はこのクラス追加して
		 */
		class UICanvas : public UIBase
		{
			friend class UIBase;
			friend class UIImage;
			friend class UIDigit;

		private:
			/** 描画するUIのリスト */
			std::vector<UIBase*> m_uiList;


		public:
			UICanvas();
			~UICanvas();


			bool Start();
			void Update();
			void Render(RenderContext& rc);


		public:
			/** UIの生成 */
			template<typename T>
			T* CreateUI()
			{
				T* ui = new T();
				return ui;
			}
		};

	}
}
