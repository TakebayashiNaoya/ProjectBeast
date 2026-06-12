/**
 * @file AchievementNotificationMenu.cpp
 * @brief アチーブメント通知画面の動的処理クラス
 * @author 立山
 */
#include "stdafx.h"
#include "AchievementNotificationMenu.h"
#include "Source/Achivement/AchievementManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UIAnimationTypes.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			constexpr float STAMP_WAIT_DELAY_SEC = 1.0f;     // スタンプが押されるまでの待機時間（秒）
			constexpr float FADE_OUT_WAIT_DELAY_SEC = 2.0f;  // フェードアウト開始までの待機時間（秒）
			constexpr float FADE_IN_OFFSET_X = 300.0f;       // フェードイン時の開始X座標オフセット
			constexpr float FADE_OUT_OFFSET_X = 600.0f;      // フェードアウト時の終了X座標オフセット
		}

		AchievementNotificationMenu::AchievementNotificationMenu()
			: m_isPlaying(false)
			, m_isInitialized(false)
			, m_animState(AnimState::Idle)
			, m_animTimer(0.0f)
			, m_defaultBgPos(Vector3::Zero)
			, m_defaultCheckPos(Vector3::Zero)
			, m_defaultStampPos(Vector3::Zero)
			, m_defaultNameTextPos(Vector3::Zero)
		{
			m_animStatus = std::make_unique<AchievementAnimStatus>();
		}


		AchievementNotificationMenu::~AchievementNotificationMenu()
		{}


		void AchievementNotificationMenu::Update()
		{
			auto* am = app::achievement::AchievementManager::GetInstance();
			if (!am) return;

			if (am->GetReloadVersion() != m_lastReloadVersion)
			{
				m_lastReloadVersion = am->GetReloadVersion();
				InitializeLogic();
			}

			auto achievementList = am->GetAllAchievements();

			// サイズが変わったら再同期（InitializeLogic再呼び出しやステージ切り替えに対応）
			if (m_wasAchievedList.size() != achievementList.size())
			{
				m_wasAchievedList.assign(achievementList.size(), false);
			}

			// 達成検知
			for (int i = 0; i < static_cast<int>(achievementList.size()); ++i)
			{
				auto* achieve = achievementList[i];
				if (!achieve) continue;

				const bool alreadyNotified = (i < static_cast<int>(m_wasAchievedList.size())) && m_wasAchievedList[i];
				if (achieve->IsAchieved() && !alreadyNotified)
				{
					NotificationData newdata;
					newdata.achievement = achieve;
					m_notificationQueue.push(newdata);
					if (i < static_cast<int>(m_wasAchievedList.size()))
						m_wasAchievedList[i] = true;
				}
			}

			auto* bgIcon    = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon"));
			auto* checkBox  = GetUI<UIIcon>(Hash32("CheckBoxIcon"));
			auto* stampIcon = GetUI<UIIcon>(Hash32("Stamp"));
			auto* nameText  = GetUI<UIText>(Hash32("AchieveNameText"));

			// 3. アニメーション開始
			if (!m_isPlaying && !m_notificationQueue.empty())
			{
				NotificationData currentData = m_notificationQueue.front();
				m_notificationQueue.pop();

				// テキストに達成内容をセット
				if (nameText)
				{
					nameText->SetText(currentData.achievement->GetDescription());
					nameText->m_transform.m_localTransform.m_position = m_defaultNameTextPos;
				}

				// 各UIを初期座標にリセット
				if (bgIcon)   bgIcon->m_transform.m_localTransform.m_position   = m_defaultBgPos;
				if (checkBox) checkBox->m_transform.m_localTransform.m_position = m_defaultCheckPos;
				if (stampIcon) stampIcon->m_transform.m_localTransform.m_position = m_defaultStampPos;

				if (bgIcon)   bgIcon->m_isDraw   = true;
				if (checkBox) checkBox->m_isDraw  = true;
				if (nameText) nameText->m_isDraw  = true;

				auto attachAndPlayFadeIn = [this](UIBase* ui, const Vector3& defaultPos) {
					if (!ui) return;
					ui->RemoveAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
					ui->RemoveAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY);

					UIAnimationFactory::Attach<UITranslateAnimation>(ui, animKey::ACHIEVE_FADE_IN_ANIM_KEY);
					if (auto* anim = ui->FindAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY))
					{
						auto* translateAnim = static_cast<UITranslateAnimation*>(anim);
						Vector3 endPos   = defaultPos;
						Vector3 startPos = endPos;
						startPos.x += FADE_IN_OFFSET_X;
						translateAnim->SetParameter(
							startPos, endPos,
							m_animStatus->GetFadeInData().duration,
							m_animStatus->GetFadeInData().easingType,
							m_animStatus->GetFadeInData().loopMode
						);
						anim->PlayAnimation();
					}
				};

				attachAndPlayFadeIn(bgIcon,    m_defaultBgPos);
				attachAndPlayFadeIn(checkBox,  m_defaultCheckPos);
				attachAndPlayFadeIn(nameText,  m_defaultNameTextPos);

				m_isPlaying  = true;
				m_animState  = AnimState::FadeIn;
				m_animTimer  = 0.0f;

				app::SoundManager::Get().PlaySE(app::enSoundKind_NoticeAchievement);
			}

			m_animStatus->Update();

			if (m_isPlaying)
			{
				m_animTimer += g_gameTime->GetFrameDeltaTime();

				if (bgIcon)    bgIcon->UpdateAnimation();
				if (checkBox)  checkBox->UpdateAnimation();
				if (nameText)  nameText->UpdateAnimation();
				if (stampIcon) stampIcon->UpdateAnimation();

				switch (m_animState)
				{
				case AnimState::FadeIn:
				{
					auto* anim = bgIcon ? bgIcon->FindAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY) : nullptr;
					if (!anim || !anim->IsPlayAnimation())
					{
						m_animState = AnimState::StampWait;
						m_animTimer = 0.0f;
					}
				}
				break;

				case AnimState::StampWait:
					if (m_animTimer >= STAMP_WAIT_DELAY_SEC)
					{
						m_animState = AnimState::StampPlay;
						if (stampIcon)
						{
							stampIcon->m_isDraw = true;
							UIAnimationFactory::Attach<UIScaleAnimation>(stampIcon, animKey::ACHIEVE_STAMP_ANIM_KEY);
							if (auto* anim = stampIcon->FindAnimation(animKey::ACHIEVE_STAMP_ANIM_KEY))
								anim->PlayAnimation();
						}
					}
					break;

				case AnimState::StampPlay:
				{
					auto* anim = stampIcon ? stampIcon->FindAnimation(animKey::ACHIEVE_STAMP_ANIM_KEY) : nullptr;
					if (!anim || !anim->IsPlayAnimation())
					{
						app::SoundManager::Get().PlaySE(app::enSoundKind_Stamp);
						m_animState = AnimState::FadeOutWait;
						m_animTimer = 0.0f;
					}
				}
				break;

				case AnimState::FadeOutWait:
					if (m_animTimer >= FADE_OUT_WAIT_DELAY_SEC)
					{
						m_animState = AnimState::FadeOut;
						app::SoundManager::Get().PlaySE(app::enSoundKind_FadeOutAchievement);

						auto attachAndPlayFadeOut = [this](UIBase* ui, const Vector3& defaultPos) {
							if (!ui) return;
							ui->RemoveAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY);
							UIAnimationFactory::Attach<UITranslateAnimation>(ui, animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
							if (auto* anim = ui->FindAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY))
							{
								auto* translateAnim = static_cast<UITranslateAnimation*>(anim);
								Vector3 startPos = defaultPos;
								Vector3 endPos   = startPos;
								endPos.x += FADE_OUT_OFFSET_X;
								translateAnim->SetParameter(
									startPos, endPos,
									m_animStatus->GetFadeOutData().duration,
									m_animStatus->GetFadeOutData().easingType,
									m_animStatus->GetFadeOutData().loopMode
								);
								anim->PlayAnimation();
							}
						};

						attachAndPlayFadeOut(bgIcon,    m_defaultBgPos);
						attachAndPlayFadeOut(checkBox,  m_defaultCheckPos);
						attachAndPlayFadeOut(nameText,  m_defaultNameTextPos);
						attachAndPlayFadeOut(stampIcon, m_defaultStampPos);
					}
					break;

				case AnimState::FadeOut:
				{
					auto* anim = bgIcon ? bgIcon->FindAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY) : nullptr;
					if (!anim || !anim->IsPlayAnimation())
					{
						m_isPlaying = false;
						m_animState = AnimState::Idle;

						auto clearAnims = [](UIBase* ui) {
							if (!ui) return;
							ui->RemoveAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY);
							ui->RemoveAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
							ui->RemoveAnimation(animKey::ACHIEVE_STAMP_ANIM_KEY);
						};

						clearAnims(bgIcon);
						clearAnims(checkBox);
						clearAnims(stampIcon);
						clearAnims(nameText);

						if (stampIcon) stampIcon->m_isDraw = false;
						if (bgIcon)    bgIcon->m_isDraw    = false;
						if (checkBox)  checkBox->m_isDraw  = false;
						if (nameText)  nameText->m_isDraw  = false;
					}
				}
				break;

				case AnimState::Idle:
				default:
					break;
				}
			}

			MenuBase::Update();
		}


		void AchievementNotificationMenu::Render(RenderContext& rc)
		{
			MenuBase::Render(rc);
		}


		void AchievementNotificationMenu::InitializeLogic()
		{
			m_notificationQueue = std::queue<NotificationData>();
			m_wasAchievedList.clear();
			m_isPlaying  = false;
			m_isInitialized = false;
			m_animTimer  = 0.0f;
			m_animState  = AnimState::Idle;

			auto* bgIcon    = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon"));
			auto* checkBox  = GetUI<UIIcon>(Hash32("CheckBoxIcon"));
			auto* stampIcon = GetUI<UIIcon>(Hash32("Stamp"));
			auto* nameText  = GetUI<UIText>(Hash32("AchieveNameText"));

			if (bgIcon)    m_defaultBgPos        = bgIcon->m_transform.m_localTransform.m_position;
			if (checkBox)  m_defaultCheckPos      = checkBox->m_transform.m_localTransform.m_position;
			if (stampIcon) m_defaultStampPos      = stampIcon->m_transform.m_localTransform.m_position;
			if (nameText)  m_defaultNameTextPos   = nameText->m_transform.m_localTransform.m_position;

			if (bgIcon)    bgIcon->m_isDraw    = false;
			if (checkBox)  checkBox->m_isDraw  = false;
			if (stampIcon) stampIcon->m_isDraw = false;
			if (nameText)  nameText->m_isDraw  = false;
		}
	}
}
