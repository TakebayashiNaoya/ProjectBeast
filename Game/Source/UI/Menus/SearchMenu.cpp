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
			// シロクマの頭上のアイコンのオフセット値。
			constexpr float POLAR_BEAR_OFFSET_Y = 150.0f;

			// 内積が0の値。
			constexpr float DOT_ZERO = 0.0f;

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
		{
			if (m_icon)return;
		}


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
			// 子ペンギンを見つけたアクティブ、子ペンギンを見失った非アクティブ。
			Searching();

			// キャンバスの更新。
			SearchClass::Update();
		}


		void SearchMenu::Searching()
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
				// カメラの前方に敵がいるかを判定する。
				Vector3 cameraForward = g_camera3D->GetForward();
				// シロクマの座標を取得。
				Vector3 enemyPos = m_enemy->GetTransform().m_position;
				// ワールド座標でシロクマの頭上の座標を計算した後に、スクリーン空間に変換する。
				Vector3 iconWorldPos = enemyPos + Vector3(0.0f, POLAR_BEAR_OFFSET_Y, 0.0f);
				
				// カメラの座標を取得。
				Vector3 cameraPos = g_camera3D->GetPosition();
				// カメラからシロクマへのベクトルを計算。
				Vector3 toEnemy = enemyPos - cameraPos;
				
				// カメラの後ろ側で内積が0以下なら、アイコンを表示しない。
				float dot = cameraForward.Dot(toEnemy);
				if (dot <= DOT_ZERO)
				{
					for (auto& info : SEARCH_ICON_KEYS)
					{
						auto* icon = GetUI<UIIcon>(info.key);
						if (icon)icon->m_isDraw = false;
					}
					return;
				}
				
				Vector2 screenPos = Vector2::Zero;
				// シロクマの頭上のスクリーン座標を計算
				g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, iconWorldPos);
				iconPos = Vector3(screenPos.x, screenPos.y, 0.0f);
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
		}


		void SearchMenu::InitializeLogic()
		{
			for (const auto& info : SEARCH_ICON_KEYS)
			{
				Icon searchIcon = std::make_unique<SearchIcon>(info.type);
				searchIcon->SetUIIcon(GetUI<UIIcon>(info.key));
				m_searchIconMap.emplace(info.key, std::move(searchIcon));

				// 生成直後は非表示にする
				auto* icon = GetUI<UIIcon>(info.key);
				if (icon) icon->m_isDraw = false;
			}
		}
	}
}