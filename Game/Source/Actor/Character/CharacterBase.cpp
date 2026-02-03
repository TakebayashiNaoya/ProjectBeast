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
		CharacterBase::~CharacterBase()
		{
			if (m_animationClips)  delete[] m_animationClips;
		}
		void CharacterBase::Init(const ModelData* data)
		{
			// アニメーションクリップを作成
			m_animationClips = new AnimationClip[data->clipNum];

			for (int i = 0; i < data->clipNum; ++i)
			{
				m_animationClips[i].Load(data->animationData[i].fileName);
				m_animationClips[i].SetLoopFlag(data->animationData[i].isLoop);
			}

			// モデルをセットアップ
			m_modelRender.Init(data->fileName, m_animationClips, data->clipNum, data->upAxis);
			// トランスフォームを設定
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			// モデルレンダーを更新
			m_modelRender.Update();

			// ステータスを取得
			const auto* status = GetStatus<CharacterStatus>();
			// キャラクターコントローラーを初期化
			m_characterController.Init(status->GetRadius(), status->GetHeight(), m_transform.m_position);
		}
	}
}