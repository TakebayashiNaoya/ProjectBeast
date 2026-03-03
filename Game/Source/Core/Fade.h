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


		private:
			Fade();
			~Fade();


		private:
			/** フェードの処理 */
			void FadeProcess();


		private:
			static Fade* m_instance;


		private:
			SpriteRender m_fadeRender;


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
			float m_timer = 0.0f;

			/** フェードが完了するまでの時間 */
			float m_duration = 0.0f;
		};
	}
}
