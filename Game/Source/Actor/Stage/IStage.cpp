/**
 * @file IStage.cpp
 * @brief ステージの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStage.h"


namespace app
{
	namespace actor
	{
		void IStageObject::Start()
		{
			/** 物理判定を作成 */
			//m_physicsStaticObject.CreateFromModel(m_modelRender.GetModel(), m_modelRender.GetModel().GetWorldMatrix());
		}


		void IStageObject::Update()
		{
			if (!m_isModelLoaded)
			{
				if (!m_pendingModelPath.empty() && m_tksLoader.IsReady())
				{
					// tks をバンクに登録
					m_tksLoader.Finalize();
					// モデル初期化（スケルトンはバンクヒットでIOスキップ）
					m_modelRender.Init(m_pendingModelPath.c_str());
					m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
					m_modelRender.Update();
					m_isModelLoaded = true;
				}
				return;
			}

			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			m_modelRender.Update();
		}


		void IStageObject::Render(RenderContext& rc)
		{
			if (!m_isModelLoaded) return;
			m_modelRender.Draw(rc);
		}

		void IStageObject::Init(const char* fileName)
		{
			m_isModelLoaded = false;
			m_pendingModelPath = fileName;
			m_tksLoader.Reset();

			// tksパスを作成して非同期ロード（.tkm -> .tks）
			std::string tksPath = fileName;
			auto extPos = tksPath.rfind(".tkm");
			if (extPos != std::string::npos) {
				tksPath.replace(extPos, 4, ".tks");
			}
			else {
				tksPath += ".tks";
			}
			m_tksLoader.RequestLoad(ResourceManager::GetInstance(), tksPath.c_str());
		}
	}
}