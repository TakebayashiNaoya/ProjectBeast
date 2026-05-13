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
			//constexpr char* FULL_PATH = "Assets/spriteData/UI/Achievement/AchieveName_/";

			// UIサイズ用の定数
			constexpr float ACHIEVE_NAME_W = 400.0f;
			constexpr float ACHIEVE_NAME_H = 40.0f;

			constexpr float STAMP_WAIT_DELAY_SEC = 1.0f;     // スタンプが押されるまでの待機時間（秒）
			constexpr float FADE_OUT_WAIT_DELAY_SEC = 2.0f;  // フェードアウト開始までの待機時間（秒）
			constexpr float FADE_IN_OFFSET_X = 300.0f;       // フェードイン時の開始X座標オフセット
			constexpr float FADE_OUT_OFFSET_X = 600.0f;      // フェードアウト時の終了X座標オフセット
		}

		AchievementNotificationMenu::AchievementNotificationMenu()
			: m_isPlaying(false)
			, m_isInitialized(false)
			, m_currentActiveNameIcon(nullptr)     // 追加: ポインタは必ず nullptr で初期化
			, m_animState(AnimState::Idle)         // 追加: enum の初期状態
			, m_animTimer(0.0f)                    // 追加: float の初期値
			, m_defaultBgPos(Vector3::Zero)        // 追加: Vector3 の初期値
			, m_defaultCheckPos(Vector3::Zero)     // 追加: Vector3 の初期値
			, m_defaultStampPos(Vector3::Zero)     // 追加: Vector3 の初期値
		{
			m_animStatus = std::make_unique<AchievementAnimStatus>();
		}


		AchievementNotificationMenu::~AchievementNotificationMenu()
		{}


		void AchievementNotificationMenu::Update()
		{
			auto* am = app::achievement::AchievementManager::GetInstance();
			if (!am) return;

			auto achievementList = am->GetAllAchievements();

			// 1. 遅延初期化
			if (!achievementList.empty() && !m_isInitialized)
			{
				m_wasAchievedList.clear();
				m_wasAchievedList.resize(achievementList.size(), false);

				auto* bgIcon = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon"));

				for (auto* achieve : achievementList) {
					if (!achieve) continue;

					//  GetID() を使って一発で確実にUIを取得する
					auto* nameIcon = GetUI<UIIcon>(achieve->GetID());

					if (bgIcon && nameIcon) {
						nameIcon->m_isDraw = false;
					}
				}

				m_isInitialized = true;
			}

			// 2. 達成検知（既存のまま）
			for (int i = 0; i < achievementList.size(); ++i)
			{
				auto* achieve = achievementList[i];
				if (!achieve) continue;

				if (achieve->IsAchieved() && !m_wasAchievedList[i])
				{
					NotificationData newdata;
					newdata.achievement = achieve;
					m_notificationQueue.push(newdata);
					m_wasAchievedList[i] = true;
				}
			}

			auto* bgIcon = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon"));
			auto* checkBox = GetUI<UIIcon>(Hash32("CheckBoxIcon"));
			auto* stampIcon = GetUI<UIIcon>(Hash32("Stamp"));

			// 3. アニメーション開始
			if (!m_isPlaying && !m_notificationQueue.empty())
			{
				NotificationData currentData = m_notificationQueue.front();
				m_notificationQueue.pop();

				m_currentActiveNameIcon = GetUI<UIIcon>(currentData.achievement->GetID());

				// 毎回表示前に、記憶しておいた「本来の初期座標」にリセットする
				if (bgIcon) bgIcon->m_transform.m_localTransform.m_position = m_defaultBgPos;
				if (checkBox) checkBox->m_transform.m_localTransform.m_position = m_defaultCheckPos;
				if (stampIcon) stampIcon->m_transform.m_localTransform.m_position = m_defaultStampPos;

				// 名前アイコンの初期座標も、表示前に保存しておく
				Vector3 defaultNamePos = Vector3::Zero;
				if (m_currentActiveNameIcon) {
					defaultNamePos = m_currentActiveNameIcon->m_transform.m_localTransform.m_position;
				}

				if (bgIcon) bgIcon->m_isDraw = true;
				if (checkBox) checkBox->m_isDraw = true;
				if (m_currentActiveNameIcon) m_currentActiveNameIcon->m_isDraw = true;

				// ★引数に defaultPos を追加し、記憶した座標を基準に計算する
				auto attachAndPlayFadeIn = [this](UIIcon* icon, const Vector3& defaultPos) {
					if (icon) {

						icon->RemoveAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
						icon->RemoveAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY);

						UIAnimationFactory::Attach<UITranslateAnimation>(icon, animKey::ACHIEVE_FADE_IN_ANIM_KEY);
						if (auto* anim = icon->FindAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY)) {
							auto* translateAnim = static_cast<UITranslateAnimation*>(anim);

							Vector3 endPos = defaultPos;
							Vector3 startPos = endPos;

							// ★ マジックナンバーを定数に置き換え
							startPos.x += FADE_IN_OFFSET_X;

							translateAnim->SetParameter(
								startPos,
								endPos,
								m_animStatus->GetFadeInData().duration,
								m_animStatus->GetFadeInData().easingType,
								m_animStatus->GetFadeInData().loopMode
							);

							anim->PlayAnimation();
						}
					}
					};

				// それぞれの初期座標を渡してアニメーションを開始
				attachAndPlayFadeIn(bgIcon, m_defaultBgPos);
				attachAndPlayFadeIn(checkBox, m_defaultCheckPos);
				attachAndPlayFadeIn(m_currentActiveNameIcon, defaultNamePos);

				m_isPlaying = true;
				m_animState = AnimState::FadeIn;
				m_animTimer = 0.0f;

				app::SoundManager::Get().PlaySE(app::enSoundKind_NoticeAchievement);
			}

			m_animStatus->Update();


			if (m_isPlaying) {
				m_animTimer += g_gameTime->GetFrameDeltaTime();

				if (bgIcon) bgIcon->UpdateAnimation();
				if (checkBox) checkBox->UpdateAnimation();
				if (m_currentActiveNameIcon) m_currentActiveNameIcon->UpdateAnimation();
				if (stampIcon) stampIcon->UpdateAnimation();

				switch (m_animState)
				{
				case AnimState::FadeIn:
				{
					auto* anim = bgIcon ? bgIcon->FindAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY) : nullptr;

					if (!anim || !anim->IsPlayAnimation()) {
						m_animState = AnimState::StampWait; // スタンプ待機へ
						m_animTimer = 0.0f;
					}
				}
				break;

				case AnimState::StampWait:
					if (m_animTimer >= STAMP_WAIT_DELAY_SEC) {
						m_animState = AnimState::StampPlay; // スタンプ再生中へ
						if (stampIcon) {
							stampIcon->m_isDraw = true;
							UIAnimationFactory::Attach<UIScaleAnimation>(stampIcon, animKey::ACHIEVE_STAMP_ANIM_KEY);
							if (auto* anim = stampIcon->FindAnimation(animKey::ACHIEVE_STAMP_ANIM_KEY)) {
								anim->PlayAnimation();
							}
						}
					}
					break;

				case AnimState::StampPlay:
				{
					auto* anim = stampIcon ? stampIcon->FindAnimation(animKey::ACHIEVE_STAMP_ANIM_KEY) : nullptr;
					if (!anim || !anim->IsPlayAnimation()) {
						app::SoundManager::Get().PlaySE(app::enSoundKind_Stamp); // スタンプSE再生
						m_animState = AnimState::FadeOutWait; // フェードアウト待機へ
						m_animTimer = 0.0f;
					}
				}
				break;

				case AnimState::FadeOutWait:
					if (m_animTimer >= FADE_OUT_WAIT_DELAY_SEC) {
						m_animState = AnimState::FadeOut; // フェードアウト中へ

						app::SoundManager::Get().PlaySE(app::enSoundKind_FadeOutAchievement);

						auto attachAndPlayFadeOut = [this](UIIcon* icon, const Vector3& defaultPos) {
							if (icon) {
								icon->RemoveAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY);
								UIAnimationFactory::Attach<UITranslateAnimation>(icon, animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
								if (auto* anim = icon->FindAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY)) {
									auto* translateAnim = static_cast<UITranslateAnimation*>(anim);

									Vector3 startPos = defaultPos;
									Vector3 endPos = startPos;

									// ★ マジックナンバーを定数に置き換え
									endPos.x += FADE_OUT_OFFSET_X;

									translateAnim->SetParameter(
										startPos,
										endPos,
										m_animStatus->GetFadeOutData().duration,
										m_animStatus->GetFadeOutData().easingType,
										m_animStatus->GetFadeOutData().loopMode
									);

									anim->PlayAnimation();
								}
							}
							};

						// 名前アイコンの初期座標を再取得
						Vector3 defaultNamePos = Vector3::Zero;
						if (m_currentActiveNameIcon) {
							// FadeIn完了時点で本来の座標にいるはずなので、ここで取得してもOK
							defaultNamePos = m_currentActiveNameIcon->m_transform.m_localTransform.m_position;
						}

						attachAndPlayFadeOut(bgIcon, m_defaultBgPos);
						attachAndPlayFadeOut(checkBox, m_defaultCheckPos);
						attachAndPlayFadeOut(m_currentActiveNameIcon, defaultNamePos);

						// スタンプは拡縮アニメーションなので、座標はそのまま退出させる
						attachAndPlayFadeOut(stampIcon, m_defaultStampPos);
					}
					break;

				case AnimState::FadeOut:
				{
					auto* anim = bgIcon ? bgIcon->FindAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY) : nullptr;
					if (!anim || !anim->IsPlayAnimation()) {
						m_isPlaying = false;
						m_animState = AnimState::Idle; // 待機状態に戻す

						auto clearAnims = [](UIIcon* icon) {
							if (icon) {
								icon->RemoveAnimation(animKey::ACHIEVE_FADE_IN_ANIM_KEY);
								icon->RemoveAnimation(animKey::ACHIEVE_FADE_OUT_ANIM_KEY);
								icon->RemoveAnimation(animKey::ACHIEVE_STAMP_ANIM_KEY);
							}
							};

						// 各UIをまっさらな状態に戻す
						clearAnims(bgIcon);
						clearAnims(checkBox);
						clearAnims(stampIcon);
						clearAnims(m_currentActiveNameIcon);

						if (stampIcon) stampIcon->m_isDraw = false;
						if (bgIcon) bgIcon->m_isDraw = false;
						if (checkBox) checkBox->m_isDraw = false;
						if (m_currentActiveNameIcon) {
							m_currentActiveNameIcon->m_isDraw = false;
							m_currentActiveNameIcon = nullptr;
						}
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
			m_isPlaying = false;
			m_animTimer = 0.0f;
			m_animState = AnimState::Idle;

			auto* bgIcon = GetUI<UIIcon>(Hash32("AchieveBackGroundIcon"));
			auto* checkBox = GetUI<UIIcon>(Hash32("CheckBoxIcon"));
			auto* stampIcon = GetUI<UIIcon>(Hash32("Stamp"));

			if (bgIcon) m_defaultBgPos = bgIcon->m_transform.m_localTransform.m_position;
			if (checkBox) m_defaultCheckPos = checkBox->m_transform.m_localTransform.m_position;
			if (stampIcon) m_defaultStampPos = stampIcon->m_transform.m_localTransform.m_position;

			// 全て非表示にするだけ（SetParentや引き算はすべて削除！）
			if (bgIcon) bgIcon->m_isDraw = false;
			if (checkBox) checkBox->m_isDraw = false;
			if (stampIcon) stampIcon->m_isDraw = false;
		}
	}
}