/**
 * @file UIAnimation.cpp
 * @brief UIAnimationのクラス
 * @author 忽那
 */
#include "stdafx.h"
#include "UIAnimation.h"
#include "Source/UI/UIParts.h"


namespace app
{
	namespace ui
	{
		UIColorAnimation::UIColorAnimation()
		{
			SetFunc([&](Vector4 v)
				{
					m_ui->m_color = v;
				});
		}





		/******************************************/


		UIScaleAnimation::UIScaleAnimation()
		{
			SetFunc([&](Vector3 s)
				{
					m_ui->m_transform.m_localTransform.m_scale = s;
				});
		}





		/******************************************/


		UITranslateAnimation::UITranslateAnimation()
		{
			SetFunc([&](Vector3 s)
				{
					m_ui->m_transform.m_localTransform.m_position = s;
				});
		}





		/******************************************/


		UITranslateOffsetAnimation::UITranslateOffsetAnimation()
		{
			SetFunc([&](Vector3 offset)
				{
					m_ui->m_transform.m_localTransform.m_position.Add(offset);
				});
		}





		/******************************************/


		UIRotationAnimation::UIRotationAnimation()
		{
			SetFunc([&](float s)
				{
					m_ui->m_transform.m_localTransform.m_rotation.SetRotationDegZ(s);
				});
		}





		/******************************************/


		void UIAnimationSequence::Update(float deltaTime)
		{
			if (!m_isPlaying || !m_target)return;

			if (m_waitingDelay) {
				m_delayTimer -= deltaTime;
				if (m_delayTimer > 0.0f) return;
				m_waitingDelay = false;
				StartCurrentStep();
				return;
			}

			/** 現在のアニメーション完了チェック */
			if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_steps.size()))
			{
				const auto& step = m_steps[m_currentIndex];
				UIAnimationBase* anim = m_target->FindAnimation(step.animationKey);
				if (anim && !anim->IsPlayAnimation())
				{
					// 完了コールバック。
					if (step.onComplete)step.onComplete();
					AdvanceToNext();
				}
			}
		}


		void UIAnimationSequence::StartCurrentStep()
		{
			if (m_currentIndex < 0 || m_currentIndex >= static_cast<int>(m_steps.size()))
			{
				const auto& step = m_steps[m_currentIndex];
				UIAnimationBase* anim = m_target->FindAnimation(step.animationKey);
				if (anim){
					if (step.onStart)step.onStart();
					anim->PlayAnimation();
				}
				else {
					// アニメーションが見つからない場合はスキップ。
					AdvanceToNext();
				}
			}
		}
	}
}