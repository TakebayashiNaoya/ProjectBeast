/**
 * @file SearchMenu.cpp
 * @brief シロクマがプレイヤーを見つけるか見つけないかの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "SearchMenu.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/UI/Model/SearchStatus.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{	
		SearchMenu::SearchMenu()
			: m_enemy(nullptr)
			, m_isActive(false)
			, m_canFind(false)
		{
			// シロクマ追跡・索敵専用ステータスを生成。
			m_searchStatus = std::make_unique<SearchStatus>();
			// シロクマ追跡・索敵専用のセットアップUIを呼び出す。
			m_searchStatus->SetUpUI();
		}


		SearchMenu::~SearchMenu()
		{}


		void SearchMenu::Update()
		{
			// 敵の情報が無い場合は処理を行わない。
			if (!m_enemy) return;


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

			// 索敵状態と追跡状態の取得。
			const bool isSearching = sm->IsSeach();
			const bool isChasing = sm->IsActionButtonB();
			const bool showAny = m_isActive && (isSearching || isChasing);

			Vector3 iconPos = Vector3::Zero;

			if (showAny)
			{
				// シロクマの座標を取得。
				Vector3 enemyPos = m_enemy->GetTransform().m_position;
				// ワールド座標でシロクマの頭上の座標を計算した後に、スクリーン空間に変換する。
				Vector3 iconWorldPos = enemyPos + Vector3(m_searchStatus->GetIconPosX(), m_searchStatus->GetOffsetValueY(), m_searchStatus->GetIconPosZ());
				
				// カメラの座標を取得。
				Vector3 cameraPos = g_camera3D->GetPosition();
				// カメラからシロクマへのベクトルを計算。
				Vector3 toEnemy = enemyPos - cameraPos;
				
				// 内積が0以下の時は、全てのUIIconを非表示。
				if (g_camera3D->GetForward().Dot(toEnemy) <= m_searchStatus->GetDotValue())
				{
					// シロクマがカメラの後ろにいる場合は、全てのアイコンを非表示にする。
					SetAllIconActive(false);
					return;
				}

				Vector2 screenPos = Vector2::Zero;
				// シロクマの頭上のスクリーン座標を計算
				g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, iconWorldPos);
				iconPos = Vector3(screenPos.x, screenPos.y, 0.0f);
			}

			// アイコンとフレームの取得。
			auto* canFindIcon = GetUI<UIIcon>(Hash32("canSearchIcon"));
			auto* canNotFindIcon = GetUI<UIIcon>(Hash32("canNotSearchIcon"));
			auto* frameA = GetUI<UIIcon>(Hash32("canSearchFrame"));
			auto* frameB = GetUI<UIIcon>(Hash32("canNotSearchFrame"));

			// 追跡状態のアイコンとフレームの描画設定。
			bool canDraw = m_isActive && isChasing;

			if (canFindIcon) canFindIcon->m_isDraw = canDraw;
			if (frameA) frameA->m_isDraw = canDraw;

			if (canDraw)
			{
				if (canFindIcon) canFindIcon->m_transform.m_localTransform.m_position = iconPos;
				if (frameA) frameA->m_transform.m_localTransform.m_position = iconPos + m_searchStatus->GetOffsetA();
			}
			// 索敵状態のアイコンとフレームの描画設定。
			bool canNotDraw = m_isActive && isSearching;

			if (canNotFindIcon) canNotFindIcon->m_isDraw = canNotDraw;
			if (frameB) frameB->m_isDraw = canNotDraw;
			
			if (canNotDraw)
			{
				if (canNotFindIcon) canNotFindIcon->m_transform.m_localTransform.m_position = iconPos;
				if (frameB) frameB->m_transform.m_localTransform.m_position = iconPos + m_searchStatus->GetOffsetB();
			}
		}


		void SearchMenu::SetAllIconActive(bool isDraw)
		{
			auto* canFindIcon = GetUI<UIIcon>(Hash32("canSearchIcon"));
			auto* canNotFindIcon = GetUI<UIIcon>(Hash32("canNotSearchIcon"));
			auto* frameA = GetUI<UIIcon>(Hash32("canSearchFrame"));
			auto* frameB = GetUI<UIIcon>(Hash32("canNotSearchFrame"));

			if (canFindIcon)    canFindIcon->m_isDraw = isDraw;
			if (canNotFindIcon) canNotFindIcon->m_isDraw = isDraw;
			if (frameA)         frameA->m_isDraw = isDraw;
			if (frameB)         frameB->m_isDraw = isDraw;
		}


		void SearchMenu::InitializeLogic()
		{
			// ゲームが開始段階であれば、アイコンとフレームは全て非表示にする。
			SetAllIconActive(false);
		}
	}
}