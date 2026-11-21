/**
 * @file InGameButtonGaugeAnimStatus.cpp
 * @brief インゲームボタンの再集合ゲージ専用のアニメーションステータスクラス
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
			// インゲームボタンの再集合ゲージ専用のアニメーションパラメータのJSONファイルパス名。
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

			// 再集合ゲージのロック演出の定義を取得。
			if (const auto* regroupLock = param.Find(animKey::REGROUP_GAUGE_LOCK_ANIM_KEY))
			{
				m_regroupLockAnimData.startColor = regroupLock->startV4;
				m_regroupLockAnimData.endColor = regroupLock->endV4;
				m_regroupLockAnimData.duration = regroupLock->duration;
				m_regroupLockAnimData.easingType = regroupLock->easingType;
				m_regroupLockAnimData.loopMode = regroupLock->loopMode;
			}
			// 再集合ゲージの復帰演出の定義を取得。
			if (const auto* regroupUnlock = param.Find(animKey::REGROUP_GAUGE_UNLOCK_ANIM_KEY))
			{
				m_regroupUnlockAnimData.startColor = regroupUnlock->startV4;
				m_regroupUnlockAnimData.endColor = regroupUnlock->endV4;
				m_regroupUnlockAnimData.duration = regroupUnlock->duration;
				m_regroupUnlockAnimData.easingType = regroupUnlock->easingType;
				m_regroupUnlockAnimData.loopMode = regroupUnlock->loopMode;
			}
		}


		void InGameButtonGaugeAnimStatus::Update()
		{
			SetUp();
		}
	}
}
