/**
 * @file AchievementNotificationMenu.cpp
 * @brief アチーブメント通知画面の動的処理クラス
 * @author 立山
 */
#include "stdafx.h"
#include "AchievementNotificationMenu.h"
#include "Source/Achivement/AchievementManager.h"
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UIAnimationTypes.h"


namespace app
{
	namespace ui
	{
		AchievementNotificationMenu::AchievementNotificationMenu()
			: m_isPlaying(false)
		{
			m_animStatus = std::make_unique<AchievementAnimStatus>();
		}


		AchievementNotificationMenu::~AchievementNotificationMenu()
		{}


		void AchievementNotificationMenu::Update()
		{
			auto* am = app::achievement::AchievementManager::GetInstance();
			if (!am) return;


			//達成状態の検知とキューに追加
			auto achievementList = am->GetAllAchievements();
			for (int i = 0; i < achievementList.size(); ++i)
			{
				// ここで各アチーブメントの状態をチェックして処理を行う
				auto* achieve = achievementList[i];
				if (!achieve) continue;


				if (achieve->IsAchieved() && !m_wasAchievedList[i])
				{
					// 新たに達成されたアチーブメントを通知キューに追加
					NotificationData newdata;
					newdata.achievement = achieve;


					// キューに追加
					m_notificationQueue.push(newdata);


					// 達成状態を更新
					m_wasAchievedList[i] = true;
				}
			}


			// アニメーション開始
			if (!m_isPlaying && !m_notificationQueue.empty())
			{
				NotificationData currentData = m_notificationQueue.front();


				// キューから削除
				m_notificationQueue.pop();


				// アニメーションの開始、再生中のフラグを立てる
				//m_sequence.Play(this);
				m_isPlaying = true;
			}

			m_animStatus->Update();
			m_sequence.Update(g_gameTime->GetFrameDeltaTime());


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
			m_sequence.Clear();

			// アチーブメントマネージャーから全リストを取得し、m_wasAchievedListを初期化
			auto* am = app::achievement::AchievementManager::GetInstance();
			if (am)
			{
				auto achieveList = am->GetAllAchievements();
				m_wasAchievedList.clear();
				// 最初はすべて未達成(false)としてメモ帳を作る
				m_wasAchievedList.resize(achieveList.size(), false);
			}


			// AnimationTypesで定義したアニメーションをシーケンスに追加
			// 例：
			// auto* bgIcon = GetUI<UIIcon>(Hash32("AchieveNotifyBG"));
			// UIAnimationFactory::Attach<UITranslateAnimation>(bgIcon, animKey::ACHIEVE_FADE_IN_ANIM_KEY);


			// 画面外からフェードイン
			m_sequence.Add(animKey::ACHIEVE_FADE_IN_ANIM_KEY);

			// スタンプのアニメーション
			m_sequence.Add(animKey::ACHIEVE_STAMP_ANIM_KEY, 1.0f);

			// 画面外へフェードアウト
			m_sequence.Add(animKey::ACHIEVE_FADE_OUT_ANIM_KEY, 2.0f);

			// アニメーションが最後まで終わった時の処理
			m_sequence.OnComplete([this]() {
				m_isPlaying = false;
				});
		}
	}
}