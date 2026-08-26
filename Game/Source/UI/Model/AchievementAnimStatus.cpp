/**
 * @file AchievementAnimStatus.cpp
 * @brief アチーブメントアニメーション専用のステータスクラス
 */
#include "stdafx.h"
#include "AchievementAnimStatus.h"
#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			const char* JSON_PATH = "Assets/parameter/UI/inGameAchievement/AchievementAnimParameter.json";
		}


		AchievementAnimStatus::AchievementAnimStatus()
		{
			UIAnimationParameter::Get().Load(JSON_PATH);
			SetUp();
		}


		AchievementAnimStatus::~AchievementAnimStatus()
		{}


		void AchievementAnimStatus::SetUp()
		{
			// UIAnimationParameterのシングルトンインスタンスを取得。
			const auto& param = UIAnimationParameter::Get();


			// アチーブメントアニメーション専用のfadeInアニメーションの定義を取得。
			const auto* fadeIn = param.Find(animKey::ACHIEVE_FADE_IN_ANIM_KEY);
			if (fadeIn)
			{
				m_fadeInData.startPos = fadeIn->startV2;
				m_fadeInData.endPos = fadeIn->endV2;
				m_fadeInData.startScale = fadeIn->startV3;
				m_fadeInData.endScale = fadeIn->endV3;
				m_fadeInData.startColor = fadeIn->startV4;
				m_fadeInData.endColor = fadeIn->endV4;
				m_fadeInData.duration = fadeIn->duration;
				m_fadeInData.easingType = fadeIn->easingType;
				m_fadeInData.loopMode = fadeIn->loopMode;
			}


			// アチーブメントアニメーション専用のfadeOutアニメーションの定義を取得。
			const auto* fadeOut = param.Find(animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
			if (fadeOut)
			{
				m_fadeOutData.startPos = fadeOut->startV2;
				m_fadeOutData.endPos = fadeOut->endV2;
				m_fadeOutData.startScale = fadeOut->startV3;
				m_fadeOutData.endScale = fadeOut->endV3;
				m_fadeOutData.startColor = fadeOut->startV4;
				m_fadeOutData.endColor = fadeOut->endV4;
				m_fadeOutData.duration = fadeOut->duration;
				m_fadeOutData.easingType = fadeOut->easingType;
				m_fadeOutData.loopMode = fadeOut->loopMode;
			}


			// アチーブメントアニメーション専用のstampアニメーションの定義を取得。
			const auto* stamp = param.Find(animKey::ACHIEVE_STAMP_ANIM_KEY);
			if (stamp)
			{
				m_stampData.startPos = stamp->startV2;
				m_stampData.endPos = stamp->endV2;
				m_stampData.startScale = stamp->startV3;
				m_stampData.endScale = stamp->endV3;
				m_stampData.startColor = stamp->startV4;
				m_stampData.endColor = stamp->endV4;
				m_stampData.duration = stamp->duration;
				m_stampData.easingType = stamp->easingType;
				m_stampData.loopMode = stamp->loopMode;
			}
		}


		void AchievementAnimStatus::Update()
		{
			SetUp();
		}
	}
}