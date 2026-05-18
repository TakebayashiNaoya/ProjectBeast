/**
 * @file WpWarningAnimStatus.cpp
 * @brief WpWarningのアニメーションステータス
 * @author 藤谷
 */
#include "stdafx.h"
#include "WpWarningAnimStatus.h"


namespace app
{
	namespace ui
	{
		WpWarningAnimStatus::WpWarningAnimStatus()
		{
			// JSONファイルからUIAnimationパラメーターを読み込む。
			UIAnimationParameter::Get().Load("Assets/parameter/UI/wpWarning/WpWarningAnimParameter.json");
		}


		WpWarningAnimStatus::~WpWarningAnimStatus()
		{}


		void WpWarningAnimStatus::SetUpUI()
		{
			// UIAnimationParameterのシングルトンインスタンスを取得。
			const auto& param = UIAnimationParameter::Get();


			const auto* first = param.Find(animKey::WP_GROW_AND_SHRINK_ANIM_KEY);
			if (!first)return;
			if (first)
			{
				m_growAndShrinkAnimData.startScale = first->startV3;
				m_growAndShrinkAnimData.endScale = first->endV3;
				m_growAndShrinkAnimData.duration = first->duration;
				m_growAndShrinkAnimData.easingType = first->easingType;
				m_growAndShrinkAnimData.loopMode = first->loopMode;
			}
		}


		void WpWarningAnimStatus::Update()
		{
			SetUpUI();
		}
	}
}