/**
 * @file Application.h
 * @brief アプリケーション全体を管理するクラス
 * @author 立山
 */
#pragma once


namespace app
{
	/**
	 * @brief アプリケーション全体を管理するクラス
	 */
	class Application : public Noncopyable
	{
	public:
		Application();
		~Application();

		void Update();
		void Render(RenderContext& rc);
	};
}
