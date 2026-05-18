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

			const auto* animParam = param.Find(animKey::WP_GROW_AND_SHRINK_ANIM_KEY);
			if (!animParam) return;

			m_growAndShrinkAnimData.startScale = animParam->startV3;
			m_growAndShrinkAnimData.endScale = animParam->endV3;
			m_growAndShrinkAnimData.duration = animParam->duration;
			m_growAndShrinkAnimData.easingType = animParam->easingType;
			m_growAndShrinkAnimData.loopMode = animParam->loopMode;
		}


		void WpWarningAnimStatus::Update()
		{
			SetUpUI();
		}
	}
}