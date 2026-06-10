/**
 * @file TitleScene.h
 * @brief タイトルシーン
 * @author 立山
 */
#pragma once
#include "IScene.h"

#include "Source/UI/System/SystemPacket.h"


namespace app
{
	namespace ui
	{
		class SoundOptionMenu;
		class TitleEventMenu;
		class TutorialMenu;
		class StageSelectMenu;
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
		/** @brief タイトル画面の更新 */
		void TitleUpdate();
		/** @brief ステージ選択画面の更新 */
		void StageSelectUpdate();
		/** @brief サウンドオプション画面の更新 */
		void SoundOptionUpdate();
		/** @brief チュートリアル画面の更新 */
		void TutorialUpdate();


	private:
		enum class TitleState
		{
			Title,
			StageSelect,
			SoundOption,
			Tutorial
		};

		TitleState m_state;

	private:
		bool     m_nextScene   = false;
		uint32_t m_nextSceneId = 0;


	private:
		ui::UIPacket<ui::TitleEventMenu> m_titleEventPacket;
		ui::UIPacket<ui::SoundOptionMenu> m_soundOptionPacket;
		ui::UIPacket<ui::TutorialMenu> m_tutorialPacket;
		ui::UIPacket<ui::StageSelectMenu> m_stageSelectPacket;
	};
}