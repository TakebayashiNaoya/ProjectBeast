/**
 * @file SearchMenu.cpp
 * @brief シロクマがプレイヤーを見つけるか見つけないかの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "SearchMenu.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			struct SearchInfo
			{
				uint32_t key;
				EnSearchType type;
			};

			// 要素数。
			constexpr int SEARCH_ICON_SIZE = static_cast<int>(EnSearchType::Max);
			// キーとタイプの配列。
			constexpr SearchInfo SEARCH_ICON_KEYS[SEARCH_ICON_SIZE] =
			{
					{ Hash32("canSearchIcon"),EnSearchType::CanFind }
				,	{ Hash32("canNotSearchIcon"),EnSearchType::CanNotFind }
			};
		}


		SearchIcon::SearchIcon(EnSearchType type)
			: m_icon(nullptr)
			, m_type(type)
		{}


		SearchIcon::~SearchIcon()
		{}


		void SearchIcon::Update()
		{}


		void SearchIcon::SetUIIcon(UIIcon* icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗です。");
		}





		/*****************************************/


		SearchMenu::SearchMenu()
			: m_enemy(nullptr)
			, m_currentType(EnSearchType::CanFind)
			, m_isActive(false)
			, m_canFind(false)
		{}


		SearchMenu::~SearchMenu()
		{}


		void SearchMenu::Update()
		{
			// 敵やステートマシンが存在しない場合は処理を行わない。
			if (!m_enemy || !m_enemy->GetEnemyStateMachine()) return;
			auto* sm = m_enemy->GetEnemyStateMachine();

			bool isSearching = sm->IsSeach();
			bool isChasing = sm->IsActionButtonB();
			Vector3 iconPos = Vector3::Zero;

			// シロクマの上にアイコンを表示する時だけ、計算を行う。
			if (m_isActive && (isSearching || isChasing))
			{
				// シロクマの頭上のスクリーン座標を計算
				Vector2 screenPos = Vector2::Zero;
				Vector3 enemyPos = m_enemy->GetTransform().m_position;
				g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, enemyPos);
				iconPos = Vector3(screenPos.x, screenPos.y + 250.0f, 0.0f);
			}

			// 各タイプの描画のオンオフの座標の更新を行う。
			for (auto& info : SEARCH_ICON_KEYS)
			{
				// UIIconを取得。
				auto* icon = GetUI<UIIcon>(info.key);
				if (icon == nullptr) continue;

				if (info.type == EnSearchType::CanFind)
				{
					// アクティブかつ、Chase状態の時は
					if (m_isActive && isChasing)
					{
						icon->m_isDraw = true;
						icon->m_transform.m_localTransform.m_position = iconPos;
					}
					else
					{
						icon->m_isDraw = false;
					}
				}
				else if (info.type == EnSearchType::CanNotFind)
				{
					// アクティブかつ、Search状態の時は
					if (m_isActive && isSearching)
					{
						icon->m_isDraw = true;
						icon->m_transform.m_localTransform.m_position = iconPos;
					}
					else
					{
						icon->m_isDraw = false;
					}
				}
			}
			// キャンバスの更新。
			SearchClass::Update();
		}


		void SearchMenu::InitializeLogic()
		{
			for (const auto& info : SEARCH_ICON_KEYS)
			{
				Icon searchIcon = std::make_unique<SearchIcon>(info.type);
				searchIcon->SetUIIcon(GetUI<UIIcon>(info.key));
				m_searchIconMap.emplace(info.key, std::move(searchIcon));
			}
		}
	}
}