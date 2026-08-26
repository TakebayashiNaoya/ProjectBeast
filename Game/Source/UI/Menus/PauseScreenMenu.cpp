/**
 * @file PauseScreenMenu.cpp
 * @brief ポーズ画面の動的クラス
 */
#include "stdafx.h"
#include "PauseScreenMenu.h"
#include "SoundOptionMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/CRC32.h"
#include "UIMenuConstants.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			SoundOptionMenu* soundOptionMenu = nullptr;

			// アイコンが選択された時のスケール。
			constexpr float PAUSE_SCREEN_ICON_SELECT_SCALE = 1.2f;
			// アイコンが選択されていないときのスケール。
			constexpr float PAUSE_SCREEN_ICON_UNSELECT_SCALE = 1.0f;


			// keyとtypeの構造体。
			struct PauseScreenInfo
			{
				uint32_t key;
				PauseScreenType type;
			};


			constexpr int PAUSE_SCREEN_ONE_VALUE = 1;
			constexpr int PAUSE_SCREEN_MIN_VALUE = static_cast<int>(PauseScreenType::ReturnPlayType);
			// 共通の要素数。
			constexpr int PAUSE_SCREEN_ICON_SIZE = static_cast<int>(PauseScreenType::Max);
			constexpr int PAUSE_SCREEN_BUTTON_SIZE = static_cast<int>(PauseScreenType::GoBackTitleType);
			// nameとtypeの配列。UI側でkeyをもとにUIを取得するために必要。
			constexpr PauseScreenInfo PAUSE_SCREEN_ICON_KEYS[PAUSE_SCREEN_ICON_SIZE] =
			{
					{ Hash32("PauseTitleIcon"),              PauseScreenType::TitleType      }
				,	{ Hash32("PauseRetryIcon"),              PauseScreenType::ReturnPlayType }
				,	{ Hash32("PauseSoundOptionTextIcon"),    PauseScreenType::SoundOptionType}
				,	{ Hash32("PauseRuleTextIcon"),           PauseScreenType::RuleType       }
				,	{ Hash32("PauseGoBackTileTextIcon"),     PauseScreenType::GoBackTitleType}
			};

			// nameとtypeの配列。UI側でkeyをもとにUIを取得するために必要。
			constexpr PauseScreenInfo PAUSE_SCREEN_BUTTON_KEYS[PAUSE_SCREEN_BUTTON_SIZE] =
			{
					{ Hash32("PauseOnePartsButton"),   PauseScreenType::ReturnPlayType }
				,	{ Hash32("PauseTwoPartsButton"),   PauseScreenType::SoundOptionType}
				,	{ Hash32("PauseThreePartsButton"), PauseScreenType::RuleType       }
				,	{ Hash32("PauseFourPartsButton"),  PauseScreenType::GoBackTitleType}
			};

		}

		PauseScreenIcon::PauseScreenIcon(PauseScreenType type)
			: m_gamePad(g_pad[0])
			, m_text(nullptr)
			, m_type(type)
		{}


		PauseScreenIcon::~PauseScreenIcon()
		{}


		void PauseScreenIcon::UpdateSelect(PauseScreenType currentType)
		{
			if (m_type == currentType)
			{
				m_text->m_transform.m_localTransform.m_scale.x = PAUSE_SCREEN_ICON_SELECT_SCALE;
				m_text->m_transform.m_localTransform.m_scale.y = PAUSE_SCREEN_ICON_SELECT_SCALE;
				m_text->m_transform.m_localTransform.m_scale.z = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
			}
			else
			{
				m_text->m_transform.m_localTransform.m_scale.x = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
				m_text->m_transform.m_localTransform.m_scale.y = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
				m_text->m_transform.m_localTransform.m_scale.z = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
			}
		}


		void PauseScreenIcon::SetUIText(UIText* text)
		{
			m_text = text;
			K2_ASSERT(m_text != nullptr, "登録失敗です。");
		}


		/***************************************/


		PauseScreenButton::PauseScreenButton(PauseScreenType type)
			: m_button(nullptr)
			, m_pauseIcon(nullptr)
			, m_gamePad(g_pad[0])
			, m_type(type)
		{}


		PauseScreenButton::~PauseScreenButton()
		{}


		void PauseScreenButton::UpdateSelect(PauseScreenType currentType)
		{
			if (m_type == currentType)
			{
				m_button->m_transform.m_localTransform.m_scale.x = PAUSE_SCREEN_ICON_SELECT_SCALE;
				m_button->m_transform.m_localTransform.m_scale.y = PAUSE_SCREEN_ICON_SELECT_SCALE;
				m_button->m_transform.m_localTransform.m_scale.z = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
			}
			else
			{
				m_button->m_transform.m_localTransform.m_scale.x = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
				m_button->m_transform.m_localTransform.m_scale.y = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
				m_button->m_transform.m_localTransform.m_scale.z = PAUSE_SCREEN_ICON_UNSELECT_SCALE;
			}
		}


		void PauseScreenButton::SetUIButton(UIButton* button)
		{
			m_button = button;
			K2_ASSERT(m_button != nullptr, "登録失敗です。");
		}


		void PauseScreenButton::SetPauseIcon(PauseScreenIcon* icon)
		{
			m_pauseIcon = icon;
			K2_ASSERT(m_pauseIcon != nullptr, "登録失敗です。");
		}


		/***************************************/


		PauseScreenMenu::PauseScreenMenu()
			: m_currentType(PauseScreenType::ReturnPlayType)
			, m_isVisible(false)
			, m_isRetry(false)
			, m_isGoTitle(false)
			, m_isSound(false)
			, m_isRule(false)
			, m_isStickNeutralY(true)
			, m_axisInputDetector()
			, m_cursorSelector(PAUSE_SCREEN_BUTTON_SIZE)
		{}


		void PauseScreenMenu::Update()
		{
			// カーソルの移動処理。
			MoveCursor();

			// アイコンとボタンの更新処理。
			UpdateSelect();

			// 決定処理。
			EnterType();

			// キャンバスの更新処理。
			PauseClass::Update();
		}


		void PauseScreenMenu::UpdateSelect()
		{
			for (const auto& icon : m_pauseIconMap)
			{
				icon.second->UpdateSelect(m_currentType);
			}

			for (const auto& button : m_pauseButtonMap)
			{
				button.second->UpdateSelect(m_currentType);
			}
		}


		void PauseScreenMenu::MoveCursor()
		{
			const float stickY = g_pad[0]->GetLStickYF();

			// 上入力でNegative（-1）、下入力でPositive（+1）として、
			// PAUSE_SCREEN_MIN_VALUEを基準にしたオフセット付きインデックスをループ移動する。
			const auto dir = m_axisInputDetector.Update(
				-stickY, g_pad[0]->IsTrigger(enButtonUp), g_pad[0]->IsTrigger(enButtonDown), STICK_THRESHOLD);

			if (m_cursorSelector.TryMove(dir))
			{
				m_currentType = static_cast<PauseScreenType>(PAUSE_SCREEN_MIN_VALUE + m_cursorSelector.Get());
			}
		}


		void PauseScreenMenu::EnterType()
		{
			if (!g_pad[0]->IsTrigger(enButtonA))return;



			switch (m_currentType)
			{
			case PauseScreenType::ReturnPlayType:
				// ゲームに戻る場合は別でSEを再生
				m_isRetry = true;
				break;
			case PauseScreenType::SoundOptionType:
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonEnter);
				m_isSound = true;
				break;
			case PauseScreenType::RuleType:
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonEnter);
				m_isRule = true;
				break;
			case PauseScreenType::GoBackTitleType:
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonEnter);
				m_isGoTitle = true;
				break;
			default:
				K2_ASSERT(false, "存在しないタイプです。");
				break;
			}
		}


		void PauseScreenMenu::InitializeLogic()
		{
			// アイコンの初期化。
			InitializeIcon();
			// ボタンの初期化。
			InitializeButton();

			// 入力検出・カーソル選択の状態をリセット。
			m_axisInputDetector.Reset();
			m_cursorSelector.Reset();
			m_currentType = static_cast<PauseScreenType>(PAUSE_SCREEN_MIN_VALUE + m_cursorSelector.Get());
		}


		void PauseScreenMenu::InitializeIcon()
		{
			// ダングリングポインタを防ぐために、マップをクリアする。
			m_pauseIconMap.clear();
			// ポーズアイコンの数だけ、マップの容量を確保する。
			m_pauseIconMap.reserve(PAUSE_SCREEN_ICON_SIZE);


			for (const auto& info : PAUSE_SCREEN_ICON_KEYS)
			{
				// ポーズアイコンの生成。
				Icon pauseIcon = std::make_unique<PauseScreenIcon>(info.type);

				// UIからテキストを取得して、アイコンに設定する。
				pauseIcon->SetUIText(GetUI<UIText>(info.key));
				// ポーズアイコンマップにアイコンを追加する。
				m_pauseIconMap.emplace(info.key, std::move(pauseIcon));
			}
		}


		void PauseScreenMenu::InitializeButton()
		{
			// ダングリングポインタを防ぐため、クリア
			m_pauseButtonMap.clear();
			// ポーズボタンの数だけ、マップの容量を確保。
			m_pauseButtonMap.reserve(PAUSE_SCREEN_BUTTON_SIZE);


			for (const auto& info : PAUSE_SCREEN_BUTTON_KEYS)
			{
				// ボタンの生成。
				Button pauseButton = std::make_unique<PauseScreenButton>(info.type);
				// UIパーツの設定。
				pauseButton->SetUIButton(GetUI<UIButton>(info.key));
				// アイコンのキーを保存。（enum値を利用してインデックスを取得）
				const Key pauseIconKey = PAUSE_SCREEN_ICON_KEYS[static_cast<int>(info.type)].key;
				// アイコンの情報を持ってくる。
				pauseButton->SetPauseIcon(m_pauseIconMap[pauseIconKey].get());

				// マップにボタンを追加。
				m_pauseButtonMap.emplace(info.key, std::move(pauseButton));
			}
		}
	}
}