/**
 * @file DebugScene.h
 * @brief デバッグシーン
 * @author 立山
 */
#include "stdafx.h"
#include "DebugScene.h"


namespace app
{
	DebugScene::DebugScene()
	{
	}


	DebugScene::~DebugScene()
	{
	}


	bool DebugScene::Start()
	{
		NewGO<Ocean>(0);
		g_camera3D->SetPosition({ 0.0f, 100.0f, -200.0f }); /** 手前・上に配置 */
		EffectEngine::GetInstance()->ResistEffect(0, u"Assets/effect/test/test.efk");
		EffectEmitter* m_effect = NewGO<EffectEmitter>(0);
		m_effect->Init(0);
		m_effect->SetPosition(Vector3::Zero);
		m_effect->SetRotation(Quaternion::Identity);
		m_effect->SetScale({ 15.0f,15.0f,15.0f });
		m_effect->Play();
		return true;
	}


	void DebugScene::Update()
	{
	}


	void DebugScene::Render(RenderContext& rc)
	{
	}


	bool DebugScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		return false;
	}
}