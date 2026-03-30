/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"


namespace app
{
	namespace ui
	{
		class Layout;
		class SoundOptionMenu;
	}

	class TitleScene :public IScene
	{
		appScene(TitleScene);


	public:
		TitleScene();
		~TitleScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate()override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;

	private:
		enum class TitleState
		{
			Title,
			SoundOption
		};

		TitleState m_state;

	private:
		bool m_nextScene = false;


	private:
		SpriteRender m_titleRender;
		SpriteRender m_AButton;
		SpriteRender m_XButton;
		SpriteRender m_PenTakt;


	private:
		ui::Layout* m_titleLayout;
		ui::Layout* m_soundOptionLayout;
		ui::SoundOptionMenu* m_soundOption;
	};
}