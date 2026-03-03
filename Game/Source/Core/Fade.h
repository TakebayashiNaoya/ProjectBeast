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
			void Enable() { m_isEnable = true; }
			void Disable() { m_isEnable = false; }


		public:
			void FadeOut(float duration);
			void FadeIn(float duration);


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
			void FadeProcess();


		private:
			static Fade* m_instance;


		private:
			SpriteRender m_fadeRender;


		private:
			float m_timer = 0.0f;
			float m_duration = 1.0f;
			bool m_isFadeOut = false;
			bool m_isFadeIn = false;

			bool m_isEnable = false;
		};
	}
}
