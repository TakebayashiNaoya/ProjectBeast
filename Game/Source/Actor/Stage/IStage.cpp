/**
 * @file IStage.cpp
 * @brief ステージの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStage.h"
#include "Source/Graphics/PBRStatus.h"


namespace app
{
	namespace actor
	{
		IStageObject::~IStageObject()
		{
			// ディザリングマネージャーから登録解除
			OcclusionDitherManager::Get().Unregister(&m_modelRender);
		}


		void IStageObject::Start()
		{
			/** 物理判定を作成 */
			//m_physicsStaticObject.CreateFromModel(m_modelRender.GetModel(), m_modelRender.GetModel().GetWorldMatrix());
		}


		void IStageObject::Update()
		{
			if (!m_isModelLoaded)
			{
				if (!m_pendingModelPath.empty() && m_tkmLoader.IsReady())
				{
					// TKM非同期ロード完了 → バンク登録＋モデル初期化
					nsK2EngineLow::ModelInitData initData;
					m_tkmLoader.Finalize(initData);
					// ステージはスケルトン・アニメーション不要なのでnullptr
					// pbrNameが指定されている場合はPBR補正パラメータを設定する
					if (!m_pbrName.empty())
					{
						m_modelRender.SetPBRParam(graphics::PBRStatus::Get()->GetPBRParam(m_pbrName));
					}
					m_modelRender.Init(m_pendingModelPath.c_str());
					m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
					// 影を落とすかどうかはモデル初期化後に反映する
					m_modelRender.SetCastShadow(m_isCastShadow);
					m_modelRender.Update();

					if (m_IsNeedCollision)
					{
						m_physicalObj.CreateFromModel(
							m_modelRender.GetModel(),
							m_modelRender.GetModel().GetWorldMatrix(),
							nsBeastEngine::nsCollision::CollisionAttribute::Ground
						);

						// ディザリングマネージャーに登録
						OcclusionDitherManager::Get().Register(&m_modelRender);
					}

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
			if (!m_isVisible) return;
			m_modelRender.Draw(rc);
		}


		void IStageObject::Init(const char* fileName, const std::string& pbrName)
		{
			m_isModelLoaded = false;
			m_pendingModelPath = fileName;
			m_pbrName = pbrName;
			m_tkmLoader.Reset();

			// TKMファイルを非同期ロードリクエスト
			m_tkmLoader.RequestLoad(nsBeastEngine::ResourceManager::GetInstance(), fileName);
		}
	}
}