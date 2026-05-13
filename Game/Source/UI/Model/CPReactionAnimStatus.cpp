/**
 * @file CPReactionAnimStatus.cpp
 * @brief CPReactionのアニメーションステータス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionAnimStatus.h"
#include "Source/UIAnimationTypes.h"

namespace app
{
	namespace ui
	{
		namespace
		{
			const char* JSON_PATH = "Assets/parameter/UI/cpReaction/CPReactionAnimParameter.json";
		}


		CPReactionAnimStatus::CPReactionAnimStatus()
		{
			UIAnimationParameter::Get().Load(JSON_PATH);
			SetUpUI();
		}


		CPReactionAnimStatus::~CPReactionAnimStatus()
		{}


		void CPReactionAnimStatus::SetUpUI()
		{
			const auto& param = UIAnimationParameter::Get();

			const auto* sway = param.Find(animKey::CPREACTION_SWAY_ANIM_KEY);
			if (sway)
			{
				m_swayAnimationData.startRot = sway->startFloat;
				m_swayAnimationData.endRot = sway->endFloat;
				m_swayAnimationData.startPos = sway->startV2;
				m_swayAnimationData.endPos = sway->endV2;
				m_swayAnimationData.startScale = sway->startV3;
				m_swayAnimationData.endScale = sway->endV3;
				m_swayAnimationData.startColor = sway->startV4;
				m_swayAnimationData.endColor = sway->endV4;
				m_swayAnimationData.duration = sway->duration;
				m_swayAnimationData.easingType = sway->easingType;
				m_swayAnimationData.loopMode = sway->loopMode;
			}
		}


		void CPReactionAnimStatus::Update()
		{
			SetUpUI();
		}
	}
}