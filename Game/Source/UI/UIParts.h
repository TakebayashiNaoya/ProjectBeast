/**
 * @file UIParts.h
 * @brief UIのパーツ群
 * @author 忽那
 */
#pragma once
#include "Source/UI/UIAnimation.h"
#include "Source/Core/HierarchicalTransform.h"
#include <unordered_map>


namespace app
{
	namespace ui
	{
		/**
		 * @brief UIの基底クラス
		 */
		class UIBase : public Noncopyable
		{
		public:
			/** トランスフォーム */
			app::core::HierarchicalTransform m_transform;
			/** カラー */
			Vector4 m_color;
			/** 基底軸 */
			Vector2 m_pivot;
			/** 描画するかのフラグ */
			bool m_isDraw;


		public:
			UIBase()
				: m_color(Vector4::White)
				, m_pivot(0.5f,0.5f)
				, m_isDraw(false)
			{
				m_uiAnimationMap.clear();
			}
			virtual ~UIBase()
			{
				/** 明示的に消している */
				m_uiAnimationMap.clear();
			}


			/** 初期化処理 */
			virtual void Start() = 0;
			/** 更新処理 */
			virtual void Update() = 0;
			/** 描画処理 */
			virtual void Render(RenderContext& rc) = 0;


		public:
			/** 
			 * @brief 全てのUIアニメーションの更新処理
			 * @param animation UIAnimationBase
			 */
			void UpdateAnimation()
			{
				ForEachAnimation([](UIAnimationBase* animation)
					{
						animation->Update();
					});
			}


			/** 
			 * @brief UIアニメーションを流す処理
			 * @param animation UIAnimationBase
			 */
			void PlayAnimation()
			{
				ForEachAnimation([](UIAnimationBase* animation)
					{
						animation->PlayAnimation();
					});
			}


			/**
			 * @brief UIアニメーションを再生するかどうか
			 * @param 
			 */
			bool IsPlayAnimation()
			{
				/** 全て再生済みか */
				auto it = std::find_if(m_uiAnimationMap.begin(), m_uiAnimationMap.end(),
					[&](const auto& animationPair)
					{
						auto* animation = animationPair.second.get();
						if (animation->IsPlayAnimation())
						{
							return true;
						}
						return false;
					});
				return it != m_uiAnimationMap.end();
			}


			/** 
			 * @brief UIアニメーションの追加
			 * @param key キー
			 * @param animation 値(UIAnimationBase)
			 */
			void AddAnimation(const uint32_t key,std::unique_ptr<UIAnimationBase> animation)
			{
				animation->SetUI(this);
				m_uiAnimationMap.emplace(key, std::move(animation));
			}


			/**
			 * @brief UIAnimatoinを取り除く(消去する)
			 * @param key キー(UIAnimationBase)
			 */
			void RemoveAnimation(const uint32_t key)
			{
				m_uiAnimationMap.erase(key);
			}


			/**
			 * @brief UIAnimationBaseのラムダ
			 * param func UIAnimationに対して何を行うか
			 */
			void ForEachAnimation(const std::function<void(UIAnimationBase*)>& func)
			{
				for (auto& animation : m_uiAnimationMap)
				{
					func(animation.second.get());
				}
			}


			/**
			 * @brief UIAnimationのsecondの中身を取得
			 * @param key キーを探す
			 */
			UIAnimationBase* FindAnimation(const uint32_t key)
			{
				auto it = m_uiAnimationMap.find(key);
				if (it != m_uiAnimationMap.end())
				{
					return it->second.get();
				}
				return nullptr;
			}
			

		protected:
			/** キーと値(UIAnimationBase)の登録する */
			std::unordered_map<uint32_t, std::unique_ptr<UIAnimationBase>>m_uiAnimationMap;
		};



		/**
		 * @brief UIの画像を扱うUIBaseの派生クラス
		 */
		class UIImage : public UIBase
		{
		public:
			UIImage();
			~UIImage();


			/** 更新処理 */
			virtual void Update()override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override;


		public:
			/** 
			 * @brief 基底軸の設定
			 * @param pivot 基底軸
			 */
			void SetPivot(const Vector2& pivot)
			{
				this->m_pivot = pivot;
				m_spriteRender.SetPivot(pivot);
			}


		protected:
			/** スプライトレンダー */
			SpriteRender m_spriteRender;
		};



		/**
		 * @brief UIアイコン
		 */
		class UIIcon : public UIImage
		{
		public:
			UIIcon();
			~UIIcon();


		public:
			/** 更新処理 */
			virtual void Update()override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override;


		public:
			/** 
			 * @brief 初期化
			 * @param assetName 画像の名前
			 * @param width 横
			 * @param height 縦
			 */
			void Initialize(const char* assetName, const float width, const float height);
		};



		/**
		 * @brief ボタンを使うUI
		 */
		class UIButton : public UIImage
		{
		private:
			/** 
			 * @brief ボタンが押された時の処理(外部から委譲される)
			 */
			std::function<void()>m_delegete;


		public:
			UIButton();
			~UIButton();


		public:
			/** 更新処理 */
			virtual void Update()override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override;
		};



		/**
		 * @brief ゲージUI
		 */
		class UIGauge : public UIImage
		{
		public:
			UIGauge();
			~UIGauge();


		public:
			virtual void Update()override;
			virtual void Render(RenderContext& rc)override;


		public:
			/**
			 * @brief 初期化
			 * @param assetName 画像の名前
			 * @param width 横
			 * @param height 縦
			 * @param position 座標
			 * @param scale スケール
			 * @param rotation 回転
			 */
			void Initialize(
					const char* assetName
				,	const float width
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
			friend class UIButton;


		public:
			UICanvas();
			~UICanvas();


			/** 更新処理 */
			void Update();
			/** 描画処理 */
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief UIの生成
			 * @tparam T 派生クラス
			 * @return 生成されたインスタンスを返す
			 */
			template<typename T>
			void CreateUI(const uint32_t key)
			{
				auto ui = std::make_unique<T>();
				ui->m_transform.SetParent(&m_transform);
				m_uiMap.emplace(key, std::move(ui));
			}

			
			/**
			 * @brief キーを消去
			 * @param key UIBaseのキーを消去
			 */
			void RemoveUI(const uint32_t key)
			{
				m_uiMap.erase(key);
			}


			/**
			 * @brief キーを探す(セカンドの中身を取得)
			 * @param key UIBaseのキー
			 */
			template<typename T>
			T* FindUI(const uint32_t key)
			{
				auto it = m_uiMap.find(key);
				if (it != m_uiMap.end())
				{
					return dynamic_cast<T*>(it->second.get());
				}
				return nullptr;
			}


		private:
			/** 描画するUIBaseのリスト */
			std::vector<UIBase*> m_uiList;
			/**
			 * @brief キーと値を保持する
			 * @brief 各UI自体に親子関係持たせたいけど使わない可能性があるので、
			 * 一旦ここだけにしてみる
			 */
			std::unordered_map<uint32_t, std::unique_ptr<UIBase>>m_uiMap;
		};
	}
}
