/**
 * @file LevelUpIconMenu.cpp
 * @brief 陣形レベルアップ時に親ペンギンの頭上へ表示するアイコンの演出メニュー
 */
#include "stdafx.h"
#include "LevelUpIconMenu.h"
#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr float HEAD_OFFSET_Y = 150.0f;  // 親ペンギンの頭上に表示するためのスクリーン座標Yオフセット
			constexpr float WAIT_DURATION = 0.6f;    // フェードイン後、フェードアウトまで表示を維持する秒数
			constexpr float MOVE_DURATION = 1.5f;    // 浮き上がりにかける時間
			constexpr float MOVE_DISTANCE = 80.0f;   // 浮き上がる距離
		}


		LevelUpIconMenu::LevelUpIconMenu()
			: m_icon(nullptr)
			, m_targetPosition(Vector3::Zero)
			, m_isPlaying(false)
			, m_isFadeInFinished(false)
			, m_isFadeOutStarted(false)
			, m_elapsedTime(0.0f)
			, m_waitTimer(0.0f)
		{
			m_animStatus = std::make_unique<LevelUpIconAnimStatus>();
		}


		LevelUpIconMenu::~LevelUpIconMenu()
		{}


		void LevelUpIconMenu::InitializeLogic()
		{
			m_icon = GetUI<UIIcon>(Hash32("levelUpIcon"));
			K2_ASSERT(m_icon, "UIが見つかりません");

			m_icon->m_isDraw = false;
		}


		void LevelUpIconMenu::Play()
		{
			if (!m_icon) return;

			m_icon->m_isDraw = true;

			m_icon->RemoveAnimation(animKey::LEVELUP_ICON_FADE_IN_ANIM_KEY);
			m_icon->RemoveAnimation(animKey::LEVELUP_ICON_FADE_OUT_ANIM_KEY);
			UIAnimationFactory::Attach<UIColorAnimation>(m_icon, animKey::LEVELUP_ICON_FADE_IN_ANIM_KEY);
			m_icon->PlayAnimation();

			m_isPlaying = true;
			m_isFadeInFinished = false;
			m_isFadeOutStarted = false;
			m_elapsedTime = 0.0f;
			m_waitTimer = 0.0f;
		}


		void LevelUpIconMenu::Update()
		{
			m_animStatus->Update(); // JSONのライブリロード対応

			if (m_icon && m_isPlaying)
			{
				const float deltaTime = g_gameTime->GetFrameDeltaTime();
				m_elapsedTime += deltaTime;

				// ワールド座標をスクリーン座標に変換し、頭上オフセット＋浮き上がり分を加算する
				Vector2 screenPosition = Vector2::Zero;
				CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(screenPosition, m_targetPosition);

				const float moveT = (m_elapsedTime < MOVE_DURATION) ? (m_elapsedTime / MOVE_DURATION) : 1.0f;
				m_icon->m_transform.m_localTransform.m_position = Vector3(
					screenPosition.x,
					screenPosition.y + HEAD_OFFSET_Y + MOVE_DISTANCE * moveT,
					0.0f
				);

				if (!m_isFadeInFinished)
				{
					if (!m_icon->IsPlayAnimation())
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
						m_icon->RemoveAnimation(animKey::LEVELUP_ICON_FADE_IN_ANIM_KEY);
						UIAnimationFactory::Attach<UIColorAnimation>(m_icon, animKey::LEVELUP_ICON_FADE_OUT_ANIM_KEY);
						m_icon->PlayAnimation();
						m_isFadeOutStarted = true;
					}
				}
				else
				{
					if (!m_icon->IsPlayAnimation())
					{
						m_icon->RemoveAnimation(animKey::LEVELUP_ICON_FADE_OUT_ANIM_KEY);
						m_icon->m_isDraw = false;
						m_isPlaying = false;
					}
				}
			}

			MenuBase::Update();
		}
	}
}
