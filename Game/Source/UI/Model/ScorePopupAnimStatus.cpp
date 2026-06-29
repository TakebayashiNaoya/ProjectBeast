/**
 * @file ScorePopupAnimStatus.cpp
 * @brief スコア加算ポップアップ専用のアニメーションステータスクラス
 * @author 立山
 */
#include "stdafx.h"
#include "ScorePopupAnimStatus.h"
#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// NOTE: AchievementAnimParameter.jsonと同じ形式のファイルです
			//       (同梱したScorePopupAnimParameter.jsonをこのパスに配置してください)
			const char* JSON_PATH = "Assets/parameter/UI/result/ScorePopupAnimParameter.json";
		}


		ScorePopupAnimStatus::ScorePopupAnimStatus()
		{
			UIAnimationParameter::Get().Load(JSON_PATH);
			SetUp();
		}


		ScorePopupAnimStatus::~ScorePopupAnimStatus()
		{}


		void ScorePopupAnimStatus::SetUp()
		{
			const auto& param = UIAnimationParameter::Get();

			if (const auto* fadeIn = param.Find(animKey::SCORE_POPUP_FADE_IN_ANIM_KEY))
			{
				m_fadeInData.startColor = fadeIn->startV4;
				m_fadeInData.endColor = fadeIn->endV4;
				m_fadeInData.duration = fadeIn->duration;
				m_fadeInData.easingType = fadeIn->easingType;
				m_fadeInData.loopMode = fadeIn->loopMode;
			}

			if (const auto* fadeOut = param.Find(animKey::SCORE_POPUP_FADE_OUT_ANIM_KEY))
			{
				m_fadeOutData.startColor = fadeOut->startV4;
				m_fadeOutData.endColor = fadeOut->endV4;
				m_fadeOutData.duration = fadeOut->duration;
				m_fadeOutData.easingType = fadeOut->easingType;
				m_fadeOutData.loopMode = fadeOut->loopMode;
			}
		}


		void ScorePopupAnimStatus::Update()
		{
			SetUp();
		}
	}
}