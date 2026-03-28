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


		void SearchIcon::SetUIIcon(UIIcon * icon)
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
			if (!m_enemy || !m_enemy->GetEnemyStateMachine())return;
			// 敵のステートがサーチ中かどうかのフラグを取得。
			m_canFind = m_enemy->GetEnemyStateMachine()->IsSeach();
			
			Vector3 iconPos = Vector3::Zero;
			
			for (auto& info : SEARCH_ICON_KEYS)
			{
				// UIからアイコンを取得。
				auto* icon = GetUI<UIIcon>(info.key);
				// アイコンが無い場合はスキップ。
				if (icon == nullptr)continue;

				// サーチ中は「見つけるアイコン」の処理
				if (info.type == EnSearchType::CanFind)
				{
					if (m_isActive && m_canFind)
					{
						icon->m_isDraw = true;
						icon->m_transform.m_localTransform.m_position = iconPos;
					}
					else {            // サーチ中でないときは非表示。            
						icon->m_isDraw = false;
					}
				}
				// 未サーチ中は「見つけないアイコン」の処理
				else if (info.type == EnSearchType::CanNotFind)
				{
					if (m_isActive && !m_canFind)
					{
						icon->m_isDraw = true;
						icon->m_transform.m_localTransform.m_position = iconPos; // ← 追加：シロクマの上に追従させる
					}
					else
					{
						// サーチ中のときは非表示。
						icon->m_isDraw = false;
					}
				}
			}

			//for (auto& info : SEARCH_ICON_KEYS)
			//{
			//	// UIからアイコンを取得。
			//	auto* icon = GetUI<UIIcon>(info.key);
			//	// アイコンが無い場合はスキップ。
			//	if (icon == nullptr)continue;

			//	// サーチ中は見つけるアイコンを表示。
			//	if (info.type == EnSearchType::CanFind)
			//	{
			//		if (m_isActive && m_canFind)
			//		{
			//			Vector2 screenPos = Vector2::Zero;
			//			Vector3 pbPos = m_enemy->GetTransform().m_position;
			//			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, pbPos);
			//			// 敵の頭上にアイコンを表示させるために、他の変数に保存。
			//			iconPos = Vector3(screenPos.x, screenPos.y + 250.0f, 0.0f);
			//			icon->m_isDraw = true;
			//		}
			//		else
			//		{
			//			// サーチ中でないときは非表示。
			//			icon->m_isDraw = false;
			//		}
			//	}
			//	if (info.type == EnSearchType::CanNotFind)
			//	{
			//		if (m_isActive && !m_canFind)
			//		{
			//			Vector2 screenPos = Vector2::Zero;
			//			Vector3 pbPos = m_enemy->GetTransform().m_position;
			//			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, pbPos);
			//			// 敵の頭上にアイコンを表示させるために、他の変数に保存。
			//			iconPos = Vector3(screenPos.x, screenPos.y + 250.0f, 0.0f);
			//			icon->m_isDraw = true;
			//		}
			//		else
			//		{
			//			// サーチ中でないときは非表示。
			//			icon->m_isDraw = false;
			//		}
			//	}

				// TODO:ここに見つかっていないときの処理を書く。
				//else if (info.type == EnSearchType::CanNotFind)
				//{
				//	// アクティブの時かつ、見つけないとき。
				//	icon->m_isDraw = m_isActive && !m_canFind;
				//}
			//}

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