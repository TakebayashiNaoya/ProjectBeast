/**
 * @file UIParts.h
 * @brief UIのパーツ群
 */
#pragma once
#include "Source/Core/HierarchicalTransform.h"
#include "Source/UI/Animation/UIAnimation.h"
#include "Graphics/Video/VideoRender.h"
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

			uint32_t key_;


		public:
			UIBase()
				: m_color(Vector4::White)
				, m_pivot(0.5f, 0.5f)
				, m_isDraw(true)
			{
				m_uiAnimationMap.clear();
			}
			virtual ~UIBase()
			{
				/** 明示的に消している */
				m_uiAnimationMap.clear();
			}


			/** 更新処理 */
			virtual void Update() = 0;
			/** 描画処理 */
			virtual void Render(RenderContext& rc) = 0;

			/**
			 * @brief 描画するかどうか
			 * @return 描画するかどうか
			 */
			inline bool IsDraw()const { return m_isDraw; }

			/**
			 * @brief 描画するかどうかの設定
			 * @param isDraw 描画するかどうか
			 */
			inline void SetIsDraw(const bool isDraw) { m_isDraw = isDraw; }


		public:
			void SetKey(const uint32_t key)
			{
				key_ = key;
			}
			uint32_t GetKey() const { return key_; }


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
			 * @brief UIアニメーションを止める処理
			 * @param animation UIAnimationBase
			 */
			void StopAnimation()
			{
				ForEachAnimation([](UIAnimationBase* animation)
					{
						animation->StopAnimation();
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
			void AddAnimation(const uint32_t key, std::unique_ptr<UIAnimationBase> animation)
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
		 * @brief 円形のゲージUI
		 */
		class UICircleGauge : public UIBase
		{
		public:
			UICircleGauge();
			~UICircleGauge();

			virtual void Update()override;
			virtual void Render(RenderContext& rc)override;


			/**
			 * @brief 初期化
			 * @param assetName アセット名
			 * @param fxName シェーダー名
			 * @param width 横
			 * @param height 縦
			 * @param position 座標
			 * @param scale 大きさ
			 * @param rotation 回転
			 * @param pivot 基底軸
			 * @param gaugeColor ゲージ部分の色
			 * @param bgColor リング部分の色
			 * @param innerRadius ゲージの内径
			 * @param outerRadius ゲージの外径
			 */
			void Initialize(
				const char* assetName
				, const char* fxName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector2& pivot
				, const Vector4& gaugeColor
				, const Vector4& bgColor
				, const float innerRadius
				, const float outerRadius
			);


			// ------------------------------------------
			// GaugeRender固有のパラメーター
			// -----------------------------------------


			/**
			 * @brief 0.0f～1.0fでゲージを設定
			 * @param progress ゲージの進行度
			 */
			void SetProgress(float progress) { m_gaugeRender.SetProgress(progress); }
			/**
			 * @brief ゲージの開始と終了の割合を設定
			 * @param startProgress ゲージの開始の割合
			 * @param endProgress ゲージの終了の割合
			 */
			void SetProgressRange(float startProgress, float endProgress) { m_gaugeRender.SetProgressRange(startProgress, endProgress); }
			/**
			 * @brief ゲージの内径と外径を設定
			 * @param innerRadius 内径
			 * @param outerRadius 外径
			 */
			void SetRadius(float innerRadius, float outerRadius) { m_gaugeRender.SetRadius(innerRadius, outerRadius); }
			/**
			 * @brief ゲージの太さを設定
			 * @param radius ゲージの半径
			 * @param thickness ゲージの太さ
			 */
			void SetThickness(float radius, float thickness) { m_gaugeRender.SetThickness(radius, thickness); }
			/**
			 * @brief ゲージの回転角度(単位:ラジアン)を設定
			 * @param rotationAngle 回転角度(単位:ラジアン)
			 */
			void SetRotationAngle(float rotationAngle) { m_gaugeRender.SetRotationAngle(rotationAngle); }
			/**
			 * @brief ゲージの色を設定
			 * @param color 色
			 */
			void SetGaugeColor(const Vector4& color) { m_gaugeRender.SetMulColor(color); }

			//void SetGaugeColor(const Vector4& color) { m_gaugeRender.SetGaugeColor(color); }
			/**
			 * @brief リング部分の色を設定
			 * @param color 色
			 */
			void SetBgColor(const Vector4& color) { m_gaugeRender.SetBgColor(color); }


		private:
			/** 円形ゲージ専用のスプライト */
			nsBeastEngine::GaugeRender m_gaugeRender;
		};



		/**
		 * @brief 縦方向の塗り分けゲージUI
		 * @details
		 *   テクスチャのアルファ形状(アイコンの見た目)を保ったまま、
		 *   下から上へ baseColor → fillColor に塗り分ける。スケールで
		 *   引き伸ばさないため、アイコン自体は歪まない(シェーダーで実現)。
		 */
		class UILinearFillGauge : public UIBase
		{
		public:
			UILinearFillGauge();
			~UILinearFillGauge();

			virtual void Update()override;
			virtual void Render(RenderContext& rc)override;


			/**
			 * @brief 初期化
			 * @param assetName アセット名
			 * @param fxName シェーダー名
			 * @param width 横
			 * @param height 縦
			 * @param position 座標
			 * @param scale 大きさ
			 * @param rotation 回転
			 * @param pivot 基底軸
			 * @param baseColor 未充填部分の色
			 * @param fillColor 充填部分の色
			 */
			void Initialize(
				const char* assetName
				, const char* fxName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector2& pivot
				, const Vector4& baseColor
				, const Vector4& fillColor
			);


			// ------------------------------------------
			// LinearFillGaugeRender固有のパラメーター
			// -----------------------------------------


			/**
			 * @brief 塗りつぶし割合を設定する(0.0f ~ 1.0f)。下から上へ塗られる
			 * @param fillAmount 割合
			 */
			void SetFillAmount(float fillAmount) { m_fillGaugeRender.SetFillAmount(fillAmount); }
			/**
			 * @brief 塗りつぶし割合を取得
			 * @return 割合(0.0f ~ 1.0f)
			 */
			float GetFillAmount()const { return m_fillGaugeRender.GetFillAmount(); }
			/**
			 * @brief 未充填部分の色を設定
			 * @param color 色
			 */
			void SetBaseColor(const Vector4& color) { m_fillGaugeRender.SetBaseColor(color); }
			/**
			 * @brief 充填部分の色を設定
			 * @param color 色
			 */
			void SetFillColor(const Vector4& color) { m_fillGaugeRender.SetFillColor(color); }


		private:
			/** 直線ゲージ専用のスプライト */
			nsBeastEngine::LinearFillGaugeRender m_fillGaugeRender;
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
			void Initialize(
				const char* assetName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
			);
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


			/**
			 * @brief 初期化
			 * @param assetName アセット名
			 * @param width 横幅
			 * @param height 縦幅
			 * @param position 位置
			 * @param scale スケール
			 * @param rotation 回転
			 * @param color カラー
			 */
			void Initialize(
				const char* assetName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
			);
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
			 * @param pivot 基底軸
			 */
			void Initialize(
				const char* assetName
				, const float width
				, const float height
				, const Vector3& position
				, const Vector3& scale
				, const Quaternion& rotation
				, const Vector4& color
				, const Vector2& pivot
			);
		};


		// ============================================
		// UI桁表示(スコア表示などで使用)
		// ============================================
		class UIDigit : public UIBase
		{
		private:
			/** 画像表示機能の可変長配列 */
			std::vector<SpriteRender*> renderList_;
			/** 表示される数字 */
			int number_ = 0;
			int requestNumber_ = 0;
			int digit_ = 0;
			/** 数字表示に必要な画像が入った */
			std::string assetPath_;

			int w_ = 0;
			int h_ = 0;



		public:
			UIDigit();
			~UIDigit();


		public:
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		public:
			/**
			 * ・アセットの名前
			 * ・何桁かの情報（数）
			 * ・表示する数
			 * ・横
			 * ・高さ
			 * ・位置
			 * ・大きさ
			 * ・回転
			 */
			void Initialize(const char* assetPath, const int digit, const int number, const float widht, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation);

			/** 数字を設定 */
			void SetNumber(const int number) { requestNumber_ = number; }

			std::vector<SpriteRender*>& GetSpriteRenderList() { return renderList_; }

			void ForEach(const std::function<void(SpriteRender*)>& func)
			{
				for (auto* render : renderList_) {
					func(render);
				}
			}


		private:
			void UpdateNumber(const int targetDigit, const int number);
			void UpdatePosition(const int index);

			int ComputeDigit();

			/** 対象の桁 */
			int GetDigit(int digit);
		};


		/**
		 * @brief テキストを表示するUI
		 */
		class UIText : public UIBase
		{
		public:
			UIText();
			~UIText();

			/** 更新処理 */
			void Update() override;
			/** 描画処理 */
			void Render(RenderContext& rc) override;

			/** テキストを設定 */
			void SetText(const std::string& text);

			/** スケールの設定 */
			void SetScale(const float& scale) { m_scale = { scale, scale }; }
			void SetScale(const Vector2& scale) { m_scale = scale; }

			/** スケールの取得 */
			const Vector2& GetScale() const { return m_scale; }

			/** 影の設定 */
			void SetShadowParam(bool enable, float offset, const Vector4& color)
			{
				m_fontRender.SetShadowParam(enable, offset, color);
			}

			/** 水平アライメントの設定 */
			void SetTextAlign(nsBeastEngine::TextAlign align)
			{
				m_fontRender.SetTextAlign(align);
			}

			/** 行間倍率の設定 (1.0=隙間なし、1.2=デフォルト) */
			void SetLineSpacing(float lineSpacing)
			{
				m_fontRender.SetLineSpacing(lineSpacing);
			}


		private:
			nsBeastEngine::FontRender m_fontRender;
			Vector2 m_scale;
		};



		/** @brief UIVideo の初期化データ */
		struct UIVideoInitData
		{
			std::string clipPath;           /** フォルダパス（末尾 /）または動画ファイルパス */
			float       width = 1920.0f; /** 表示幅（ピクセル） */
			float       height = 1080.0f; /** 表示高さ（ピクセル） */
			float       fps = 24.0f;   /** コマ撮りの FPS */
			bool        loop = false;   /** ループ再生 */
			bool        autoPlay = true;    /** 初期化後すぐに再生開始 */
		};


		/**
		 * @brief 映像を再生できる UI パーツ
		 * @details
		 *   JSON では type: "UIVideo" として配置する。
		 *   clipPath にフォルダパス（末尾 /）を指定するとコマ撮り（PNG/JPG）を再生する。
		 */
		class UIVideo : public UIBase
		{
		public:
			UIVideo();
			~UIVideo();

			/** 更新処理 */
			void Update() override;
			/** 描画処理 */
			void Render(RenderContext& rc) override;

			/**
			 * @brief 初期化
			 * @param data 初期化データ
			 */
			void Initialize(const UIVideoInitData& data);

			/**
			 * @brief 再生開始
			 */
			void Play() { m_videoRender.Play(); }
			/**
			 * @brief 再生一時停止
			 */
			void Pause() { m_videoRender.Pause(); }
			/**
			 * @brief 再生停止（先頭に戻る）
			 */
			void Stop() { m_videoRender.Stop(); }
			/**
			 * @brief 再生クリップを差し替えて先頭から再生する（ファイル I/O あり）
			 * @param clipPath 新しいクリップのパス
			 */
			void ChangeClip(const char* clipPath) { m_videoRender.ChangeClip(clipPath); }
			/**
			 * @brief クリップを事前にメモリへロードしてスロットに登録する
			 * @param path ファイルパス
			 * @param fps  コマ撮り FPS（MP4 では自動取得）
			 * @return ロード成功なら true
			 */
			bool PreloadClip(const char* path, float fps = 24.0f);
			/**
			 * @brief 事前ロード済みのスロットに即座に切り替える（ファイル I/O なし）
			 * @param slotIndex PreloadClip の呼び出し順（0 始まり）
			 */
			void SwitchToPreloadedClip(int slotIndex);
			/**
			 * @brief 全プリロードスロットを破棄する
			 */
			void ClearPreloadedClips() { m_preloadedClips.clear(); }
			/**
			 * @brief ループ再生の設定
			 * @param loop ループ再生する場合は true
			 */
			void SetLoop(bool loop) { m_videoRender.SetLoop(loop); }
			/**
			 * @brief 再生速度の設定（1.0 = 等速）
			 * @param speed 再生速度倍率
			 */
			void SetPlaybackSpeed(float speed) { m_videoRender.SetPlaybackSpeed(speed); }
			/**
			 * @brief 再生終了コールバックの設定
			 * @param cb 再生終了時に呼び出すコールバック
			 */
			void SetOnFinished(std::function<void()> cb) { m_videoRender.SetOnFinished(cb); }
			/**
			 * @brief 再生中かどうかを取得
			 * @return 再生中なら true
			 */
			bool IsPlaying()  const { return m_videoRender.IsPlaying(); }
			/**
			 * @brief 再生終了しているかどうかを取得
			 * @return 再生終了しているなら true
			 */
			bool IsFinished() const { return m_videoRender.IsFinished(); }


		private:
			nsBeastEngine::VideoRender m_videoRender;
			/** PreloadClip で登録したクリップのプール */
			std::vector<std::unique_ptr<nsBeastEngine::VideoClip>> m_preloadedClips;
		};



		/**
		 * @brief UIのダミー(何も描画しない)
		 */
		class UIDummy : public UIBase
		{
		public:
			UIDummy();
			~UIDummy();

			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;
		};



		/**
		 * @brief UI作成時、UICanvasを追加する
		 */
		class UICanvas : public UIBase
		{
			friend class UIBase;
			friend class UIImage;
			friend class UIGauge;
			friend class UIIcon;
			friend class UIButton;
			friend class UICircleGauge;


		public:
			UICanvas();
			~UICanvas();


			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			template <typename T>
			void CreateUI(const uint32_t key)
			{
				auto ui = std::make_unique<T>();
				ui->SetKey(key);
				ui->m_transform.SetParent(&m_transform);
				uiList_.push_back(std::move(ui));
			}


			void RemoveUI(const uint32_t key)
			{
				// TODO:本当はstd::find使いたい
				for (auto it = uiList_.begin(); it != uiList_.end(); it++) {
					if ((*it)->GetKey() == key) {
						uiList_.erase(it);
						break;
					}
				}
			}


			template <typename T>
			T* FindUI(const uint32_t key)
			{
				// TODO:本当はstd::find使いたい
				for (auto it = uiList_.begin(); it != uiList_.end(); it++) {
					if ((*it)->GetKey() == key) {
						return dynamic_cast<T*>(it->get());
					}
				}
				return nullptr;
			}
		private:
			std::vector<std::unique_ptr<UIBase>> uiList_;
		};
	}
}
