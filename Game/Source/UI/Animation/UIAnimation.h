/**
 * @file UIAnimation.h
 * @brief UIAnimationの機能群
 * @author 忽那
 */
#pragma once
#include "Source/Util/Curve.h"


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class UIBase;


		/** 長いので省略 */
		template <typename T>
		using UIAnimationApplyFunc = std::function<void(const T&)>;


		/**
		 * @brief UIAnimationの機能
		 */
		class UIAnimationBase
		{
		public:
			UIAnimationBase() : m_ui(nullptr),m_timeSec(0.0f){}
			~UIAnimationBase(){}


			/** 純粋仮想関数 */
			virtual void Update() = 0;
			virtual void PlayAnimation() = 0;
			virtual void StopAnimation() = 0;
			virtual bool IsPlayAnimation() = 0;


		public:
			/**
			 * @brief UIの設定
			 * @param ui UIBaseのポインタ
			 */
			void SetUI(UIBase* ui) { m_ui = ui; }
			

		protected:
			/** UIBaseのポインタ */
			UIBase* m_ui;
			/** 秒数 */
			float m_timeSec;
			/** 等速 */
			app::util::EasingType m_type = app::util::EasingType::Linear;
			/** 片道 */
			app::util::LoopMode m_loopMode = app::util::LoopMode::Once;
		};



		/**
		 * @brief Floatのアニメーション
		 */
		class UIFloatAnimation : public UIAnimationBase
		{
		public:
			UIFloatAnimation() : m_start(0.0f),m_end(0.0f),m_timeSec(0.0f){}
			~UIFloatAnimation(){}

			
			/** 更新処理 */
			void Update()override
			{
				bool wasPlaying = m_curve.IsPlaying();
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中 OR 停止に変わった瞬間が終了のタイミング。
				if (wasPlaying || !m_curve.IsPlaying())
				{
					m_applyFunc(m_curve.GetCurrentValue());
				}
			}


			/** 再生 */
			void PlayAnimation() override
			{
				m_curve.Play();
			}


			/** 停止 */
			void StopAnimation() override
			{
				m_curve.Stop();
			}


			/** 再生中か */
			bool IsPlayAnimation() override
			{
				return m_curve.IsPlaying();
			}
			

			/**
			 * @brief UIアニメーションのパラメーターを設定
			 * @param start 最初
			 * @param end 最後
			 * @param timeSec 秒数
			 * @param type イージングタイプ
			 * @param loopMode ループモード
			 */
			void SetParameter(float start, float end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				m_start = start;
				m_end = end;
				m_timeSec = timeSec;
				m_type = type;
				m_loopMode = loopMode;
				m_curve.Initialize(m_start, m_end, m_timeSec, m_type, m_loopMode);
			}


			/** アニメーション中の現在の値を取得 */
			float GetCurrendValue()
			{
				return m_curve.GetCurrentValue();
			}


			/** アニメーション後の情報を適用する関数を設定 */
			void SetFunc(const UIAnimationApplyFunc<float>& func)
			{
				m_applyFunc = func;
			}


		protected:
			/** Floatカーブ */
			app::util::FloatCurve m_curve;
			
			/** カーブ用のパラメーター */
			float m_start;
			float m_end;
			float m_timeSec;

			/** Float型のカーブを持ってくる */
			UIAnimationApplyFunc<float> m_applyFunc;
		};



		/**
		 * @brief 2Dアニメーション
		 */
		class UIVector2Animation : public UIAnimationBase
		{
		public:
			UIVector2Animation() : m_start(Vector2::Zero),m_end(Vector2::Zero),m_timeSec(0.0f){}
			~UIVector2Animation(){}


			/** 更新処理 */
			void Update()override
			{
				bool wasPlaying = m_curve.IsPlaying();
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				// 再生中かつ停止に変わった瞬間が終了のタイミング。
				if (wasPlaying && !m_curve.IsPlaying())
				{
					m_applyFunc(m_curve.GetCurrentValue());
				}
			}


			/** 再生 */
			void PlayAnimation() override
			{
				m_curve.Play();
			}


			/** 停止 */
			void StopAnimation() override
			{
				m_curve.Stop();
			}


			/** 再生中か */
			bool IsPlayAnimation() override
			{
				return m_curve.IsPlaying();
			}


			/** UIアニメーションの情報を設定 */
			void SetParameter(Vector2& start, Vector2& end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				m_start = start;
				m_end = end;
				m_timeSec = timeSec;
				m_type = type;
				m_loopMode = loopMode;
				m_curve.Initialize(m_start, m_end, m_timeSec, m_type, m_loopMode);
			}


			/** アニメーション中の現在の値を取得 */
			Vector2 GetCurrendValue()
			{
				return m_curve.GetCurrentValue();
			}


			/** アニメーション後の情報を適用する関数を設定 */
			void SetFunc(const UIAnimationApplyFunc<Vector2>& func)
			{
				m_applyFunc = func;
			}


		protected:
			/** Vector2のカーブ */
			app::util::Vector2Curve m_curve;
			/** カーブ用のパラメーター */
			Vector2 m_start;
			Vector2 m_end;
			float m_timeSec;

			/** Vector2型のカーブを持ってくる */
			UIAnimationApplyFunc<Vector2> m_applyFunc;
		};



		/**
		 * @brief Vector3のアニメーション(拡縮、座標、回転など)
		 */
		class UIVector3Animation : public UIAnimationBase
		{
		public:
			UIVector3Animation() : m_start(Vector3::Zero), m_end(Vector3::Zero),m_timeSec(0.0f){}
			~UIVector3Animation(){}


			/** 更新処理 */
			void Update()override
			{
				bool wasPlaying = m_curve.IsPlaying();
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				if (wasPlaying && !m_curve.IsPlaying())
				{
					m_applyFunc(m_curve.GetCurrentValue());
				}
			}

			
			/** 再生 */
			void PlayAnimation() override
			{
				m_curve.Play();
			}


			/** 停止 */
			void StopAnimation() override
			{
				m_curve.Stop();
			}


			/** 再生中か */
			bool IsPlayAnimation() override
			{
				return m_curve.IsPlaying();
			}


			/** UIアニメーションの情報を設定 */
			void SetParameter(Vector3 start, Vector3 end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				m_start = start;
				m_end = end;
				m_timeSec = timeSec;
				m_type = type;
				m_loopMode = loopMode;
				m_curve.Initialize(m_start, m_end, m_timeSec, m_type, m_loopMode);
			}


			/** Vector3の値を取得 */
			Vector3 GetCurrendValue()
			{
				return m_curve.GetCurrentValue();
			}


			/** アニメーション後の情報を適用する関数 */
			void SetFunc(const UIAnimationApplyFunc<Vector3>& func)
			{
				m_applyFunc = func;
			}


		protected:
			/** Vector3型のカーブ */
			app::util::Vector3Curve m_curve;
			/** カーブ用のパラメーター */
			Vector3 m_start;
			Vector3 m_end;
			float m_timeSec;

			/** 等速 */
			app::util::EasingType m_type = app::util::EasingType::Linear;
			/** 片道 */
			app::util::LoopMode m_loopMode = app::util::LoopMode::Once;

			/** Vector3型の指定をして持ってくる */
			UIAnimationApplyFunc<Vector3> m_applyFunc;
		};



		/**
		 * @brief Vector4のアニメーション(色のアニメーション等)
		 */
		class UIVector4Animation : public UIAnimationBase
		{
		public:
			UIVector4Animation() : m_start(Vector4::White),m_end(Vector4::White),m_timeSec(0.0f){}
			~UIVector4Animation(){}


			/** 更新処理 */
			void Update() override
			{
				bool wasPlaying = m_curve.IsPlaying();
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				if (wasPlaying && !m_curve.IsPlaying())
				{
					m_applyFunc(m_curve.GetCurrentValue());
				}
			}


			/** 再生 */
			void PlayAnimation() override
			{
				m_curve.Play();
			}


			/** 停止 */
			void StopAnimation() override
			{
				m_curve.Stop();
			}


			/** 再生中か */
			bool IsPlayAnimation() override
			{
				return m_curve.IsPlaying();
			}


			/** UIアニメーションの情報を設定 */
			void SetParameter(Vector4 start, Vector4 end, float timeSec, app::util::EasingType type, app::util::LoopMode loopMode)
			{
				m_start = start;
				m_end = end;
				m_timeSec = timeSec;
				m_type = type;
				m_loopMode = loopMode;
				m_curve.Initialize(m_start, m_end, m_timeSec, m_type, m_loopMode);
			}


			/** Vector4を取得 */
			Vector4 GetCurrendValue()
			{
				return m_curve.GetCurrentValue();
			}


			/** アニメーション後の情報を適用する関数  */
			void SetFunc(const UIAnimationApplyFunc<Vector4>& func)
			{
				m_applyFunc = func;
			}


		protected:
			/** Vector4のカーブ */
			app::util::Vector4Curve m_curve;

			/** カーブ用のパラメーター */
			Vector4 m_start;
			Vector4 m_end;
			float m_timeSec;
			
			/** 等速 */
			app::util::EasingType m_type = app::util::EasingType::Linear;
			/** 片道 */
			app::util::LoopMode m_loopMode = app::util::LoopMode::Once;

			UIAnimationApplyFunc<Vector4> m_applyFunc;
		};



		/**
		 * @brief カラーアニメーション
		 */
		class UIColorAnimation : public UIVector4Animation
		{
		public:
			UIColorAnimation();
			~UIColorAnimation(){}


			/** 更新処理 */
			void Update() override
			{
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				m_applyFunc(m_curve.GetCurrentValue());
			}
		};



		/**
		 * @brief 拡縮アニメーション
		 */
		class UIScaleAnimation : public UIVector3Animation
		{
		public:
			UIScaleAnimation();
			~UIScaleAnimation(){}


			/** 更新処理 */
			void Update() override
			{
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				m_applyFunc(m_curve.GetCurrentValue());
			}
		};



		/**
		 * @brief 座標アニメーション
		 */
		class UITranslateAnimation : public UIVector3Animation
		{
		public:
			UITranslateAnimation();
			~UITranslateAnimation(){}


			/** 更新処理 */
			void Update() override
			{
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				m_applyFunc(m_curve.GetCurrentValue());
			}
		};



		/**
		 * @brief 差分アニメーション
		 */
		class UITranslateOffsetAnimation : public UIVector3Animation
		{
		public:
			UITranslateOffsetAnimation();
			~UITranslateOffsetAnimation(){}


			/** 更新処理 */
			void Update() override
			{
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				m_applyFunc(m_curve.GetCurrentValue());
			}
		};



		/**
		 * @brief 回転アニメーション
		 */
		class UIRotationAnimation : public UIFloatAnimation
		{
		public:
			UIRotationAnimation();
			~UIRotationAnimation(){}


			/** 更新処理 */
			void Update() override
			{
				m_curve.Update(g_gameTime->GetFrameDeltaTime());
				m_applyFunc(m_curve.GetCurrentValue());
			}
		};



		/***********************************************************/


		/**
		 * @brief シーケンス内の1ステップ(構造体)
		 * @param animationKey 再生するアニメーションのキー
		 * @param delayBefore このステップ開始前の待機時間(秒)
		 * @param m_onStart 開始時コールバック
		 * @param m_onComplete 終了時コールバック
		 * @detail シーケンスアニメーションで使用する構造体。
		 */
		struct UIAnimationStep
		{
			uint32_t animationKey;
			float delayBefore = 0.0f;
			std::function<void()>onStart;
			std::function<void()>onComplete;
		};


		class UIAnimationSequence
		{
		private:
			/** アニメーションを再生する順番を管理するためのステップのリスト */
			std::vector<UIAnimationStep> m_steps;
			/** 現在再生しているステップのインデックス */
			int m_currentIndex;
			/** シーケンスが再生中かどうか */
			bool m_isPlaying;
			/** ステップ開始前の待機時間(秒) */
			float m_delayTimer;
			/** ステップ開始前の待機時間が経過したかどうか */
			bool m_waitingDelay;

			/** UIBaseのポインタ */
			UIBase* m_target;
			/** シーケンス全体の完了時のコールバック */
			std::function<void()>m_onSequenceComplete;


		public:
			UIAnimationSequence() : m_currentIndex(-1),m_isPlaying(false),m_delayTimer(0.0f),m_waitingDelay(false),m_target(nullptr){}
			~UIAnimationSequence(){}

			/**
			 * @brief ステップを追加
			 * @param animKey 再生するアニメーションのキー
			 * @param delayBefore このステップ開始前の待機時間(秒)
			 */
			UIAnimationSequence& Add(uint32_t animKey, float delayBefore = 0.0f)
			{
				UIAnimationStep step;
				step.animationKey = animKey;
				step.delayBefore = delayBefore;
				m_steps.push_back(step);
				return *this;
			}


			/**
			 * @brief コールバック付きのステップを追加
			 * @param animKey 再生するアニメーションのキー
			 * @param delayBefore このステップ開始前
			 * @param onStart ステップ開始時のコールバック
			 * @param onComplete ステップ終了時のコールバック
			 */
			UIAnimationSequence& Add(
				uint32_t animKey
				, float delayBefore
				, std::function<void()>onStart
				, std::function<void()>onComplete = nullptr
			)
			{
				UIAnimationStep step;
				step.animationKey = animKey;
				step.delayBefore = delayBefore;
				step.onStart = std::move(onStart);
				step.onComplete = std::move(onComplete);
				m_steps.push_back(step);
				return *this;
			}


			/**
			 * @brief シーケンス全体の完了コールバック
			 * @param callback シーケンス全体の完了時のコールバック
			 * @return UIAnimationSequence& 自身の参照
			 */
			UIAnimationSequence& OnComplete(std::function<void()> callback)
			{
				m_onSequenceComplete = std::move(callback);
				return *this;
			}


			/**
			 * @brief 再生開始
			 * @param target シーケンスを再生するUIBaseのポインタ
			 * @detail targetに対して、Addで追加した順番でアニメーション
			 */
			void Play(UIBase* target)
			{
				if (m_steps.empty())return;
				m_target = target;
				m_currentIndex = -1;
				m_isPlaying = true;
				AdvanceToNext();
			}


			/**
			 * @brief 停止処理
			 * @detail シーケンスを停止し、状態をリセットします。
			 */
			void Stop()
			{
				m_isPlaying = false;
				m_currentIndex = -1;
				m_waitingDelay = false;
			}


			/**
			 * @brief 毎フレーム更新処理
			 * @param deltaTime 前のフレームからの経過時間(秒)
			 */
			void Update(float deltaTime);

			/**
			 * @brief シーケンスが再生中かどうか
			 * @return bool シーケンスが再生中か
			 */
			bool IsPlaying()const { return m_isPlaying; }

			/**
			 * @brief ステップをクリア
			 * @detail シーケンス内の全てのステップをクリアし、シーケンスを停止します。
			 */
			void Clear()
			{
				m_steps.clear();
				Stop();
			}


		private:
			/**
			 * @brief 次のステップに進む
			 */
			void AdvanceToNext()
			{
				m_currentIndex++;
				if (m_currentIndex >= static_cast<int>(m_steps.size()))
				{
					// 全ステップ完了。
					m_isPlaying = false;
					if (m_onSequenceComplete)m_onSequenceComplete();
					return;
				}


				const auto& step = m_steps[m_currentIndex];
				if (step.delayBefore > 0.0f)
				{
					m_delayTimer = step.delayBefore;
					m_waitingDelay = true;
				}
				else
				{
					StartCurrentStep();
				}
			}


			void StartCurrentStep();
		};
	}
}