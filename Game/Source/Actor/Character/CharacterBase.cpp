/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CharacterBase.h"
#include "CharacterStatus.h"


namespace app
{
	namespace actor
	{
		CharacterBase::CharacterBase()
			: m_animationClips(nullptr)
		{
		}


		bool CharacterBase::Start()
		{

			// ステータスを取得
			const auto* status = GetStatus<CharacterStatus>();
			// キャラクターコントローラーを初期化
			m_characterController.Init(status->GetRadius(), status->GetHeight(), m_transform.m_position);
			return true;
		}


		void CharacterBase::Update()
		{
			// モデルレンダーを更新
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			m_modelRender.Update();
		}


		void CharacterBase::Render(RenderContext& rc)
		{
			// モデルレンダーを描画
			m_modelRender.Draw(rc);
		}


		void CharacterBase::Init(const ModelData& data)
		{
			// アニメーションクリップを作成
			m_animationClips = std::make_unique<AnimationClip[]>(data.clipNum);

			for (int i = 0; i < data.clipNum; ++i)
			{
				m_animationClips[i].Load(data.animationData[i].fileName);
				m_animationClips[i].SetLoopFlag(data.animationData[i].isLoop);
			}

			// モデルをセットアップ
			m_modelRender.Init(data.fileName, m_animationClips.get(), data.clipNum, data.upAxis);
			// トランスフォームを設定
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			// モデルレンダーを更新
			m_modelRender.Update();
		}
	}
}