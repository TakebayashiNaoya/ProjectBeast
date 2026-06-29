/**
 * @file ScorePopupAnimator.cpp
 * @brief スコア加算ポップアップ（"+2000"等）のアニメーションを担当するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "ScorePopupAnimatorMenu.h"
#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr float WAIT_DURATION = 0.6f;   // フェードイン後、フェードアウトまで表示を維持する秒数
			constexpr float MOVE_DURATION = 1.2f;   // 浮き上がりにかける時間（手動演出）
			constexpr float MOVE_DISTANCE = 60.0f;  // 浮き上がる距離
		}


		ScorePopupAnimator::ScorePopupAnimator()
			: m_basePosition(Vector3::Zero)
			, m_isPlaying(false)
			, m_isFadeInFinished(false)
			, m_isFadeOutStarted(false)
			, m_elapsedTime(0.0f)
			, m_waitTimer(0.0f)
		{
			m_animStatus = std::make_unique<ScorePopupAnimStatus>();
		}


		ScorePopupAnimator::~ScorePopupAnimator()
		{}


		void ScorePopupAnimator::Initialize(const Vector3& basePosition)
		{
			m_basePosition = basePosition;
			m_text.m_transform.m_localTransform.m_position = basePosition;
			m_text.SetIsDraw(false);
			m_isPlaying = false;
		}


		void ScorePopupAnimator::Play(const int addScore)
		{
			m_text.SetText("+" + std::to_string(addScore));
			m_text.m_transform.m_localTransform.m_position = m_basePosition;
			m_text.SetIsDraw(true);

			m_text.RemoveAnimation(animKey::SCORE_POPUP_FADE_IN_ANIM_KEY);
			m_text.RemoveAnimation(animKey::SCORE_POPUP_FADE_OUT_ANIM_KEY);
			UIAnimationFactory::Attach<UIColorAnimation>(&m_text, animKey::SCORE_POPUP_FADE_IN_ANIM_KEY);
			m_text.PlayAnimation();

			m_isPlaying = true;
			m_isFadeInFinished = false;
			m_isFadeOutStarted = false;
			m_elapsedTime = 0.0f;
			m_waitTimer = 0.0f;
		}


		void ScorePopupAnimator::Update()
		{
			m_animStatus->Update(); // JSONのライブリロード対応

			if (m_isPlaying)
			{
				const float deltaTime = g_gameTime->GetFrameDeltaTime();
				m_elapsedTime += deltaTime;

				// 浮き上がる演出（std::minを使わず三項演算子でクランプ）
				const float moveT = (m_elapsedTime < MOVE_DURATION) ? (m_elapsedTime / MOVE_DURATION) : 1.0f;
				Vector3 pos = m_basePosition;
				pos.y += MOVE_DISTANCE * moveT;
				m_text.m_transform.m_localTransform.m_position = pos;

				if (!m_isFadeInFinished)
				{
					if (!m_text.IsPlayAnimation())
					{
						m_isFadeInFinished = true;
						m_waitTimer = 0.0f;
					}
				}
				else if (!m_isFadeOutStarted)
				{
					m_waitTimer += deltaTime;
					if (m_waitTimer >= WAIT_DURATION)
					{
						m_text.RemoveAnimation(animKey::SCORE_POPUP_FADE_IN_ANIM_KEY);
						UIAnimationFactory::Attach<UIColorAnimation>(&m_text, animKey::SCORE_POPUP_FADE_OUT_ANIM_KEY);
						m_text.PlayAnimation();
						m_isFadeOutStarted = true;
					}
				}
				else
				{
					if (!m_text.IsPlayAnimation())
					{
						m_text.RemoveAnimation(animKey::SCORE_POPUP_FADE_OUT_ANIM_KEY);
						m_text.SetIsDraw(false);
						m_isPlaying = false;
					}
				}
			}

			m_text.Update();
		}


		void ScorePopupAnimator::Render(RenderContext& rc)
		{
			m_text.Render(rc);
		}
	}
}