/**
 * @file TutorialInGameScene.cpp
 * @brief チュートリアルステージ
 * @author 竹林
 */
#include "stdafx.h"
#include "TutorialInGameScene.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Actor/Stage/TerrainObject.h"


namespace app
{
	void TutorialInGameScene::OnLoadComplete()
	{
		actor::TerrainObject::TerrainConfig terrainConfig;
		terrainConfig.totalWidth  = 5000.0f;  // 地形の横幅（ワールド単位）
		terrainConfig.totalDepth  = 5000.0f;  // 地形の奥行き（ワールド単位）
		terrainConfig.heightScale = 500.0f;   // 最大高さ（ワールド単位）
		terrainConfig.subsample   = 8;        // 頂点間引き倍率（1=フル解像度 / 大=低ポリゴン）
		terrainConfig.uvTile      = 0.05f;    // テクスチャタイリング（1/0.05=20頂点ごとに繰り返し）
		terrainConfig.albedoScale = 0.8f;     // アルベド明度スケール（1.0=そのまま、小さいほど暗い）
		terrainConfig.yOffset     = -50.0f;   // 地形全体を下げる（海面に合わせて調整）
		terrainConfig.minHeight   = 5.0f;     // この高さ未満のクワッドはメッシュを生成しない

		actor::StageSystem::GetInstance()->InitTerrain(terrainConfig);
		m_tutorialController.Initialize(m_daddyPenguin);
	}


	void TutorialInGameScene::OnUpdatePlaying()
	{
		m_tutorialController.Update();
	}


	void TutorialInGameScene::OnRenderPlaying(RenderContext& rc)
	{
		m_tutorialController.Render(rc);
	}


	bool TutorialInGameScene::OnPauseUpdate()
	{
		return m_tutorialController.PauseUpdate();
	}


	bool TutorialInGameScene::OnPauseRender(RenderContext& rc)
	{
		return m_tutorialController.PauseRender(rc);
	}
}
