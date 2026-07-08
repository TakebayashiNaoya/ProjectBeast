/**
 * @file InGameButtonGaugeAnimStatus.cpp
 * @brief インゲームボタンのスタミナゲージ専用のアニメーションステータスクラス
 * @author 立山
 */
#include "stdafx.h"
#include "InGameButtonGaugeAnimStatus.h"
#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// インゲームボタンのスタミナゲージ専用のアニメーションパラメータのJSONファイルパス名。
			const char* JSON_PATH = "Assets/parameter/UI/inGameButton/InGameButtonGaugeAnimParameter.json";
		}


		InGameButtonGaugeAnimStatus::InGameButtonGaugeAnimStatus()
		{
			// JSONファイルからUIAnimationパラメーターを読み込む。
			UIAnimationParameter::Get().Load(JSON_PATH);
		}


		InGameButtonGaugeAnimStatus::~InGameButtonGaugeAnimStatus()
		{}


		void InGameButtonGaugeAnimStatus::SetUp()
		{
			const auto& param = UIAnimationParameter::Get();

			// ジャンプゲージのロック演出の定義を取得。
			if (const auto* jumpLock = param.Find(animKey::JUMP_GAUGE_LOCK_ANIM_KEY))
			{
				m_jumpLockAnimData.startColor = jumpLock->startV4;
				m_jumpLockAnimData.endColor = jumpLock->endV4;
				m_jumpLockAnimData.duration = jumpLock->duration;
				m_jumpLockAnimData.easingType = jumpLock->easingType;
				m_jumpLockAnimData.loopMode = jumpLock->loopMode;
			}
			// ジャンプゲージの復帰演出の定義を取得。
			if (const auto* jumpUnlock = param.Find(animKey::JUMP_GAUGE_UNLOCK_ANIM_KEY))
			{
				m_jumpUnlockAnimData.startColor = jumpUnlock->startV4;
				m_jumpUnlockAnimData.endColor = jumpUnlock->endV4;
				m_jumpUnlockAnimData.duration = jumpUnlock->duration;
				m_jumpUnlockAnimData.easingType = jumpUnlock->easingType;
				m_jumpUnlockAnimData.loopMode = jumpUnlock->loopMode;
			}
			// スライドゲージのロック演出の定義を取得。
			if (const auto* slideLock = param.Find(animKey::SLIDE_GAUGE_LOCK_ANIM_KEY))
			{
				m_slideLockAnimData.startColor = slideLock->startV4;
				m_slideLockAnimData.endColor = slideLock->endV4;
				m_slideLockAnimData.duration = slideLock->duration;
				m_slideLockAnimData.easingType = slideLock->easingType;
				m_slideLockAnimData.loopMode = slideLock->loopMode;
			}
			// スライドゲージの復帰演出の定義を取得。
			if (const auto* slideUnlock = param.Find(animKey::SLIDE_GAUGE_UNLOCK_ANIM_KEY))
			{
				m_slideUnlockAnimData.startColor = slideUnlock->startV4;
				m_slideUnlockAnimData.endColor = slideUnlock->endV4;
				m_slideUnlockAnimData.duration = slideUnlock->duration;
				m_slideUnlockAnimData.easingType = slideUnlock->easingType;
				m_slideUnlockAnimData.loopMode = slideUnlock->loopMode;
			}
		}


		void InGameButtonGaugeAnimStatus::Update()
		{
			SetUp();
		}
	}
}
