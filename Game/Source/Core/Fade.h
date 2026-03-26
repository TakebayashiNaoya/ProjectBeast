/**
 * @file Fade.h
 * @brief ロード画面を表示するクラス
 * @author 立山
 */
#pragma once


namespace app
{
	namespace core
	{
		class Fade
		{
		public:
			void Update();
			void Render(RenderContext& rc);


		public:
			/**
			 * @brief フェードアウトを開始する
			 * @param duration フェードアウトが完了するまでの時間
			 */
			void FadeOut(float duration);
			/**
			 * @brief フェードインを開始する
			 * @param duration フェードインが完了するまでの時間
			 */
			void FadeIn(float duration);


		public:
			/** 現在フェードをしているか */
			bool IsFading() const
			{
				return m_state != FadeState::None;
			}
			/** フェードアウトが完了したか（画面が完全に暗い状態） */
			bool IsFadeOutComplete() const
			{
				return m_state == FadeState::FadeOut && m_timer >= m_duration;
			}
			/** ローディングサークルを表示する */
			void ShowLoadingCircle();
			/** ローディングサークルを非表示にする */
			void HideLoadingCircle();


		public:
			static void Create()
			{
				if (m_instance == nullptr)
				{
					m_instance = new Fade();
				}
			}
			static Fade& Get()
			{
				return *m_instance;
			}
			static void Delete()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}

			/**
			 * @brief フェードイン中かどうか
			 * @return フェードイン中ならtrue、そうでなければfalse
			 */
			inline bool IsFadeIn()const { return m_isFadeIn; }
			/**
			 * @brief フェードアウト中かどうか
			 * @return フェードアウト中ならtrue、そうでなければfalse
			 */
			inline bool IsFadeOut()const { return m_isFadeOut; }


		private:
			Fade();
			~Fade();


		private:
			/** フェードの処理 */
			void FadeProcess();


		private:
			static Fade* m_instance;


		private:
			/** 背景スプライト */
			SpriteRender m_fadeRender;
			/** ローディングサークル（遅延初期化） */
			std::unique_ptr<SpriteRender> m_circleRender;
			/** サークル初期化済みフラグ */
			bool m_circleInited = false;
			/** サークル表示フラグ */
			bool m_showCircle = false;
			/** サークルの回転角度（ラジアン） */
			float m_circleAngle = 0.0f;

		private:
			/**
			 * @enum class FadeState
			 * @brief フェードの状態
			 */
			enum class FadeState
			{
				/** 何もしていない状態 */
				None,
				/** フェードイン */
				FadeIn,
				/** フェードアウト */
				FadeOut,
			};


		private:
			FadeState m_state = FadeState::None;


			/** フェードの経過時間 */
			float m_timer;

			/** フェードが完了するまでの時間 */
			float m_duration;

			/** フェードイン中かどうか */
			bool m_isFadeIn;
			/** フェードアウト中かどうか */
			bool m_isFadeOut;
		};
	}
}
