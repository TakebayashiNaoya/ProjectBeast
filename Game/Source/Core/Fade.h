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
			void IsEnable() { m_IsEnable = true; }
			void IsDisable() { m_IsEnable = false; }


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
			static Fade* m_instance;


		private:
			SpriteRender m_fadeRender;
			bool m_IsEnable = false;
		};
	}
}
