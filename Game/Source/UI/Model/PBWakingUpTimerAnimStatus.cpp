/**
 * @file PBWakingUpTimerAnimStatus.cpp
 * @brief PB起床タイマー専用のステータスクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "PBWakingUpTimerAnimStatus.h"
#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// PB起床タイマー専用のアニメーションパラメータのJSONファイルパス名。
			const char* JSON_PATH = "Assets/parameter/timer/PBTimer/PBWakingUpTimerAnimParameter.json";
		}


		PBWakingUpTimerAnimStatus::PBWakingUpTimerAnimStatus()
		{
			// JSONファイルからUIAnimationパラメーターを読み込む。
			UIAnimationParameter::Get().Load(JSON_PATH);
		}


		PBWakingUpTimerAnimStatus::~PBWakingUpTimerAnimStatus()
		{}


		void PBWakingUpTimerAnimStatus::SetUp()
		{
			// UIAnimationParameterのシングルトンインスタンスを取得。
			const auto& param = UIAnimationParameter::Get();

			// PB起床タイマー専用のfirstアニメーションの定義を取得。
			const auto* first = param.Find(animKey::PB_CIRCLE_COLOR_FIRST_ANIM_KEY);
			if (!first)return;
			if (first)
			{
				m_firstAnimData.startColor = first->startV4;
				m_firstAnimData.endColor = first->endV4;
				m_firstAnimData.duration = first->duration;
				m_firstAnimData.easingType = first->easingType;
				m_firstAnimData.loopMode = first->loopMode;
			}
			// PB起床タイマー専用のsecondアニメーションの定義を取得。
			const auto* second = param.Find(animKey::PB_CIRCLE_COLOR_SECOND_ANIM_KEY);
			if (!second)return;
			if (second)
			{
				m_secondAnimData.startColor = second->startV4;
				m_secondAnimData.endColor = second->endV4;
				m_secondAnimData.duration = second->duration;
				m_secondAnimData.easingType = second->easingType;
				m_secondAnimData.loopMode = second->loopMode;
			}
			// PB起床タイマー専用のthirdAnimationの定義を取得。
			const auto* third = param.Find(animKey::PB_CIRCLE_COLOR_THIRD_ANIM_KEY);
			if (!third)return;
			if (third)
			{
				m_thirdAnimData.startColor = third->startV4;
				m_thirdAnimData.endColor = third->endV4;
				m_thirdAnimData.duration = third->duration;
				m_thirdAnimData.easingType = third->easingType;
				m_thirdAnimData.loopMode = third->loopMode;
			}
		}


		void PBWakingUpTimerAnimStatus::Update()
		{
			SetUp();
		}
	}
}