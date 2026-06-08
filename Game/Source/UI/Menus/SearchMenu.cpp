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


namespace app
{
	namespace ui
	{
		SearchMenu::SearchMenu()
			: m_enemy(nullptr)
			, m_isActive(false)
		{
			// シロクマ追跡・索敵専用ステータスを生成。
			m_searchStatus = std::make_unique<SearchStatus>();
			// シロクマ追跡・索敵専用のセットアップUIを呼び出す。
			m_searchStatus->SetUp();
		}


		SearchMenu::~SearchMenu()
		{}


		void SearchMenu::Update()
		{
			// インゲーム内で一瞬だけ表示されるバグを防ぐ。
			if (!m_isDraw)
			{
				auto* iconA = GetUI<UIIcon>(Hash32("canSearchIcon"));
				if (iconA) iconA->m_isDraw = false;

				auto* iconB = GetUI<UIIcon>(Hash32("canNotSearchIcon"));
				if (iconB) iconB->m_isDraw = false;

				auto* frameA = GetUI<UIIcon>(Hash32("canSearchFrame"));
				if (frameA) frameA->m_isDraw = false;

				auto* frameB = GetUI<UIIcon>(Hash32("canNotSearchFrame"));
				if (frameB) frameB->m_isDraw = false;

				MenuBase::Update();
				return;
			}

			// 敵の情報が無い場合は処理を行わない。
			if (!m_enemy) return;


			// 子ペンギンを見つけたアクティブ、子ペンギンを見失った非アクティブ。
			Searching();

			// キャンバスの更新。
			MenuBase::Update();
		}


		void SearchMenu::Searching()
		{
			// 敵やステートマシンが存在しない場合は処理を行わない。
			if (!m_enemy || !m_enemy->GetEnemyStateMachine()) return;
			// 敵のステートマシンを取得。
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

				// ベクトルを正規化。
				toEnemy.Normalize();

				// 内積が0以下の時は、全てのUIIconを非表示。
				if (g_camera3D->GetForward().Dot(toEnemy) <= m_searchStatus->GetDotValue())
				{
					auto* canFindIcon = GetUI<UIIcon>(Hash32("canSearchIcon"));
					if (canFindIcon) canFindIcon->m_isDraw = false;

					auto* canNotFindIcon = GetUI<UIIcon>(Hash32("canNotSearchIcon"));
					if (canNotFindIcon) canNotFindIcon->m_isDraw = false;

					auto* frameA = GetUI<UIIcon>(Hash32("canSearchFrame"));
					if (frameA) frameA->m_isDraw = false;

					auto* frameB = GetUI<UIIcon>(Hash32("canNotSearchFrame"));
					if (frameB) frameB->m_isDraw = false;

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

			const Vector3 offsetA = m_searchStatus->GetOffsetA();
			const Vector3 offsetB = m_searchStatus->GetOffsetB();

			if (canFindIcon) canFindIcon->m_isDraw = canDraw;
			if (frameA) frameA->m_isDraw = canDraw;

			if (canDraw)
			{
				if (canFindIcon) canFindIcon->m_transform.m_localTransform.m_position = iconPos;
				if (frameA) frameA->m_transform.m_localTransform.m_position = iconPos + offsetA;
			}
			// 索敵状態のアイコンとフレームの描画設定。
			// 追跡状態の時の表示を優先するので、追跡中は索敵アイコンを出さないようにする。
			bool canNotDraw = m_isActive && isSearching && !isChasing;

			if (canNotFindIcon) canNotFindIcon->m_isDraw = canNotDraw;
			if (frameB) frameB->m_isDraw = canNotDraw;

			if (canNotDraw)
			{
				if (canNotFindIcon) canNotFindIcon->m_transform.m_localTransform.m_position = iconPos;
				if (frameB) frameB->m_transform.m_localTransform.m_position = iconPos + offsetB;
			}
		}


		void SearchMenu::InitializeLogic()
		{
			auto* canFindIcon = GetUI<UIIcon>(Hash32("canSearchIcon"));
			if (canFindIcon) canFindIcon->m_isDraw = false;

			auto* canNotFindIcon = GetUI<UIIcon>(Hash32("canNotSearchIcon"));
			if (canNotFindIcon) canNotFindIcon->m_isDraw = false;

			auto* frameA = GetUI<UIIcon>(Hash32("canSearchFrame"));
			if (frameA) frameA->m_isDraw = false;

			auto* frameB = GetUI<UIIcon>(Hash32("canNotSearchFrame"));
			if (frameB) frameB->m_isDraw = false;
		}
	}
}