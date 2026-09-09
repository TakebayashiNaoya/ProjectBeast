/**
 * @file PVScene.h
 * @brief PV（アトラクト映像）シーン
 */
#pragma once
#include "IScene.h"
#include "Source/UI/Layout.h"


namespace app
{
	/**
	 * @brief タイトルの放置中に流すPVシーン
	 * @details
	 *   全画面の映像を1本再生するだけのシーン。再生し切るか、
	 *   何かボタンを押されたらタイトルへ戻る。
	 *   暗転はSceneManagerのフェードに任せ、フェードが明けてから再生を始める。
	 */
	class PVScene : public IScene
	{
		appScene(PVScene);


	public:
		PVScene();
		~PVScene();

		bool Start() override;
		void Update() override;
		void PauseUpdate() override;
		void Render(RenderContext& rc) override;

		bool RequesutScene(uint32_t& id, float& waitTime) override;


	private:
		/**
		 * @brief 映像と音声の再生を開始する
		 */
		void BeginPlayback();


	private:
		/** レイアウト（UIVideoを1つだけ持つ） */
		ui::Layout    m_layout;
		/** 再生する映像 */
		ui::UIVideo*  m_video = nullptr;
		/** 再生を開始済みか（暗転が明けるまでは開始しない） */
		bool          m_isPlaybackStarted = false;
		/** タイトルへ戻る要求が出ているか */
		bool          m_isReturnRequested = false;
	};
}
