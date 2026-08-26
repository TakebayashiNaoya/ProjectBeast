/**
 * @file RemaingAnimStatus.cpp
 * @brief 救助数のアニメーションステータスクラス
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


		void RemainingAnimStatus::SetUp()
		{
			// UIAnimationParameterのシングルトンインスタンスを取得。
			const auto& param = UIAnimationParameter::Get();

			// 救助数増加アニメーション。
			const auto* startUpAnim = param.Find(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);

			if (startUpAnim)
			{
				m_startBound.rStartTlanslate = startUpAnim->startV3;
				m_startBound.rEndSink = startUpAnim->endV3;
				m_startBound.duration = startUpAnim->duration;
				m_startBound.easingType = startUpAnim->easingType;
				m_startBound.loopMode = startUpAnim->loopMode;
			}

			// 救助数増加アニメーション。(上がった位置から元に戻るアニメーション)
			const auto* endBounceDown = param.Find(animKey::RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY);

			if (endBounceDown)
			{
				m_endBound.start = endBounceDown->startV3;
				m_endBound.end = endBounceDown->endV3;
				m_endBound.duration = endBounceDown->duration;
				m_endBound.easingType = endBounceDown->easingType;
				m_endBound.loopMode = endBounceDown->loopMode;
			}

			// 救助数減少アニメーション。
			const auto* sinkDown = param.Find(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);

			if (sinkDown)
			{
				m_startSink.rSinkDown = sinkDown->startV3;
				m_startSink.rSinkBounce = sinkDown->endV3;
				m_startSink.duration = sinkDown->duration;
				m_startSink.easingType = sinkDown->easingType;
				m_startSink.loopMode = sinkDown->loopMode;
			}

			// 救助数減少アニメーション。(下がった位置から元に戻るアニメーション)
			const auto* sinkBounceDownUp = param.Find(animKey::RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY);

			if (sinkBounceDownUp)
			{
				m_endBounceDownUp.start2 = sinkBounceDownUp->startV3;
				m_endBounceDownUp.end2 = sinkBounceDownUp->endV3;
				m_endBounceDownUp.duration = sinkBounceDownUp->duration;
				m_endBounceDownUp.easingType = sinkBounceDownUp->easingType;
				m_endBounceDownUp.loopMode = sinkBounceDownUp->loopMode;
			}

			// 総数減少アニメーション。
			const auto* bounceDown = param.Find(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);

			if (bounceDown)
			{
				m_bounceDown.tStartTlanslate = bounceDown->startV3;
				m_bounceDown.tEndTlanslate = bounceDown->endV3;
				m_bounceDown.duration = bounceDown->duration;
				m_bounceDown.easingType = bounceDown->easingType;
				m_bounceDown.loopMode = bounceDown->loopMode;
			}

			// 総数減少アニメーション。(下がった位置から元に戻るアニメーション)
			const auto* bounceDownUp = param.Find(animKey::RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY);

			if (bounceDownUp)
			{
				m_bounceUp.start3 = bounceDownUp->startV3;
				m_bounceUp.end3 = bounceDownUp->endV3;
				m_bounceUp.duration = bounceDownUp->duration;
				m_bounceUp.easingType = bounceDownUp->easingType;
				m_bounceUp.loopMode = bounceDownUp->loopMode;
			}
		}


		void RemainingAnimStatus::Update()
		{
			SetUp();
		}
	}
}