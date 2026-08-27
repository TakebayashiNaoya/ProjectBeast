/**
 * @file EventMenu.cpp
 * @brief イベントの動的メニュー
 */
#include "stdafx.h"
#include "EventMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			struct EventInfo
			{
				uint32_t key;
				EnEventType type;
			};
			
			// 経過時間。
			constexpr float EVENT_ELAPSE_TIME = 0.5f;
			// 要素数。
			constexpr int EVENT_ICON_SIZE = static_cast<int>(EnEventType::Max);
			
			// イベントアイコンのキーとタイプの配列。
			constexpr EventInfo EVENT_ICON_KEYS[EVENT_ICON_SIZE] =
			{
					{ Hash32("VictoryIcon"),EnEventType::Victory }
				,	{ Hash32("DefeatIcon"),EnEventType::Defeat   }
			};
		}

		EventIcon::EventIcon(EnEventType type)
			: m_icon(nullptr)
			, m_type(type)
		{}


		EventIcon::~EventIcon()
		{}
		
		
		void EventIcon::Update()
		{
		}
		
		
		void EventIcon::SetUIIcon(UIIcon * icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗です。");
		}





		/*******************************************/


		EventMenu::EventMenu()
			: m_currentEventType(EnEventType::Victory)
			, m_isEventStart(false)
			, m_isEventFinished(false)
			, m_eventTimer(0.0f)
			, m_gamePad(g_pad[0])
		{}


		EventMenu::~EventMenu()
		{}


		void EventMenu::Update()
		{
			for (const auto& icon : m_eventIconMap)
			{
				// イベントが開始していて、アイコンのタイプが現在のイベントタイプと同じ場合は、描画する。
				bool shouldDraw = m_isEventStart && (icon.second->GetType() == m_currentEventType);
				// イベントアイコンの描画状態を設定。
				icon.second->SetIsDraw(shouldDraw);
				// イベントアイコンの更新。
				icon.second->Update();
			}
			// 1フレームの経過時間を取得。
			float deltaTime = g_gameTime->GetFrameDeltaTime();

			// イベントが開始しているかつ、イベントが終了していなければ、
			if (m_isEventStart && !m_isEventFinished)
			{
				m_eventTimer += deltaTime;
				// 指定している時間を経過したら、イベント終了。
				if (m_eventTimer >= EVENT_DURATION)
				{
					m_isEventFinished = true;
				}
				// Yボタンが押されたら、スキップして次のシーンに進めるようにする。
				if (m_eventTimer > EVENT_ELAPSE_TIME
					&& m_gamePad->IsTrigger(enButtonY))
				{
					m_isEventFinished = true;
				}
			}

			// MenuBaseの更新。
			EventClass::Update();
		}


		void EventMenu::StartEvent(EnEventType eventType)
		{
			// イベントが開始している場合は、処理を行わない。
			if (m_isEventStart)return;

			m_currentEventType = eventType;
			m_isEventStart = true;
			m_isEventFinished = false;
			m_eventTimer = 0.0f;
			for (const auto& info : EVENT_ICON_KEYS)
			{
				if (info.type == m_currentEventType)
				{
					auto* icon = GetUI<UIIcon>(info.key);
					if (icon)
					{
						// アニメーションの再生。
						icon->PlayAnimation();
					}
				}
			}
		}


		void EventMenu::ResetEvent()
		{
			m_isEventStart = false;
			m_isEventFinished = false;
			m_eventTimer = 0.0f;
			for (const auto& icon : m_eventIconMap)
			{
				// 描画をリセット。
				icon.second->SetIsDraw(false);
			}
		}


		void EventMenu::InitializeLogic()
		{
			for (const auto& info : EVENT_ICON_KEYS)
			{
				// イベントアイコンの生成。
				Icon eventIcon = std::make_unique<EventIcon>(info.type);
				// UIからIconを取得。
				auto* icon = GetUI<UIIcon>(info.key);
				// イベントアイコンに設定。
				eventIcon->SetUIIcon(icon);
				// マップにイベントアイコンを追加。
				m_eventIconMap.emplace(info.key, std::move(eventIcon));
				
				
				if (icon)
				{
					if (info.type == EnEventType::Victory)
					{
						// スケールアニメーション。
						auto scaleAnim = std::make_unique<UIScaleAnimation>();
						Vector3 startScale(0.0f, 0.0f, 0.0f);
						Vector3 endScale(1.0f, 1.0f, 1.0f);
						float duration = 0.5f;
						// アニメーションのパラメーターを設定。
						scaleAnim->SetParameter(
								startScale
							,	endScale
							,	duration
							,	util::EasingType::EaseInOut
							,	util::LoopMode::Once
						);
						// Victoryアイコンにアニメーションを追加。
						icon->AddAnimation(Hash32("ScaleAnim"), std::move(scaleAnim));
					}
					if (info.type == EnEventType::Defeat)
					{
						// 移動アニメーション。
						auto translateAnim = std::make_unique<UITranslateAnimation>();
						Vector3 startPos(0.0f, 900.0f, 0.0f);
						Vector3 endPos(0.0f, 0.0f, 0.0f);
						float duration = 0.82f;
						// アニメーションのパラメーターを設定。
						translateAnim->SetParameter(
								startPos
							,	endPos
							,	duration
							,	util::EasingType::EaseInOut
							,	util::LoopMode::Once
						);
						// Defeatアイコンにアニメーションを追加。
						icon->AddAnimation(Hash32("TranslateAnim"), std::move(translateAnim));

						// スケールアニメーション。
						auto scaleAnim = std::make_unique<UIScaleAnimation>();
						Vector3 startScale(1.0f, 1.0f, 1.0f);
						Vector3 endScale(1.5f, 1.5f, 1.5f);
						duration = 0.34f;
						// アニメーションのパラメーターを設定
						scaleAnim->SetParameter(
								startScale
							,	endScale
							,	duration
							,	util::EasingType::EaseInOut
							,	util::LoopMode::Once
						);
						// Defeatアイコンにアニメーションを追加。
						icon->AddAnimation(Hash32("ScaleAnim"), std::move(scaleAnim));
					}
				}
			}
		}
	}
}