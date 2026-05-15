/**
 * @file RemaingAnimStatus.cpp
 * @brief 救助数のアニメーションステータスクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "RemainingAnimStatus.h"
#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// JSONのパス。
			const char* JSON_PATH = "Assets/parameter/UI/remainingChild/remainingChildAnimParameter.json";
		}
		

		RemainingAnimStatus::RemainingAnimStatus()
		{
			// JSONファイルからUIAnimationパラメーターを読み込む。
			UIAnimationParameter::Get().Load(JSON_PATH);
		}


		RemainingAnimStatus::~RemainingAnimStatus()
		{}
		
		
		void RemainingAnimStatus::SetUpUI()
		{
			// UIAnimationParameterのシングルトンインスタンスを取得。
			const auto& param = UIAnimationParameter::Get();

			const auto* startUpAnim = param.Find(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);

			if (startUpAnim)
			{
				m_startBound.startBound = startUpAnim->startV3;
				m_startBound.endBound = startUpAnim->endV3;
				m_startBound.duration = startUpAnim->duration;
				m_startBound.easingType = startUpAnim->easingType;
				m_startBound.loopMode = startUpAnim->loopMode;
			}

			const auto* startDownAnim = param.Find(animKey::RESCUE_REMAIN_TLANSLATE_DOWN_ANIM_KEY);

			if (startDownAnim)
			{
				m_endBound.startBound = startDownAnim->startV3;
				m_endBound.endBound = startDownAnim->endV3;
				m_endBound.duration = startDownAnim->duration;
				m_endBound.easingType = startDownAnim->easingType;
				m_endBound.loopMode = startDownAnim->loopMode;
			}

			const auto* sinkUpAnim = param.Find(animKey::RESCUE_REMAIN_SINK_UP_ANIM_KEY);

			if (sinkUpAnim)
			{
				m_startSink.startBound = sinkUpAnim->startV3;
				m_startSink.endBound = sinkUpAnim->endV3;
				m_startSink.duration = sinkUpAnim->duration;
				m_startSink.easingType = sinkUpAnim->easingType;
				m_startSink.loopMode = sinkUpAnim->loopMode;
			}

			const auto* sinkDownAnim = param.Find(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);

			if (sinkDownAnim)
			{
				m_endSink.startBound = sinkDownAnim->startV3;
				m_endSink.endBound = sinkDownAnim->endV3;
				m_endSink.duration = sinkDownAnim->duration;
				m_endSink.easingType = sinkDownAnim->easingType;
				m_endSink.loopMode = sinkDownAnim->loopMode;
			}

			const auto* littleUpAnim = param.Find(animKey::RESCUE_REMAIN_LITTLE_UP_ANIM_KEY);

			if (littleUpAnim)
			{
				m_littleUp.startBound = littleUpAnim->startV3;
				m_littleUp.endBound = littleUpAnim->endV3;
				m_littleUp.duration = littleUpAnim->duration;
				m_littleUp.easingType = littleUpAnim->easingType;
				m_littleUp.loopMode = littleUpAnim->loopMode;
			}

			const auto* littleDownAnim = param.Find(animKey::RESCUE_REMAIN_LITTLE_DOWN_ANIM_KEY);

			if (littleDownAnim)
			{
				m_littleDown.startBound = littleDownAnim->startV3;
				m_littleDown.endBound = littleDownAnim->endV3;
				m_littleDown.duration = littleDownAnim->duration;
				m_littleDown.easingType = littleDownAnim->easingType;
				m_littleDown.loopMode = littleDownAnim->loopMode;
			}
		}
		
		
		void RemainingAnimStatus::Update()
		{
			SetUpUI();
		}
	}
}