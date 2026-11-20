/**
 * @file OrcaController.cpp
 * @brief シャチのAI
 */
#include "stdafx.h"
#include "OrcaController.h"
#include "Orca.h"
#include "OrcaManager.h"
#include "OrcaParameter.h"
#include "OrcaSeaGrid.h"
#include "OrcaStateMachine.h"
#include "OrcaStatus.h"
#include "Physics/Physics.h"
#include "Physics/Types.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStatus.h"
#include "Source/Noise/NoiseManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 停滞の検出: この距離以下しか動いていなければ停滞とみなす（EnemyController.cpp STUCK_MOVE_THRESHOLD） */
			constexpr float STUCK_MOVE_THRESHOLD = 5.0f;
			/** 停滞の検出: この時間停滞し続けたら諦める（EnemyController.cpp STUCK_TIME_LIMIT） */
			constexpr float STUCK_TIME_LIMIT = 3.0f;
			/** 経路を探し直す間隔の下限（秒） */
			constexpr float REPATH_INTERVAL = 0.5f;
			/** 目標がこのセル数以上動いたら経路を探し直す */
			constexpr float REPATH_GOAL_MOVE_CELLS = 2.0f;
			/** 経由点に着いたとみなす距離（セル数） */
			constexpr float WAYPOINT_ARRIVE_CELLS = 1.5f;
			/** 音の目標地点に着いたとみなす距離（クマは 5。シャチは岸の手前で止まるので広めにとる） */
			constexpr float SEARCH_ARRIVE_DISTANCE = 30.0f;
			/** 最後に見た位置に着いたとみなす距離（EnemyController.cpp ARRIVE_LAST_KNOWN_POS_DIST は 20） */
			constexpr float ARRIVE_LAST_KNOWN_POS_DISTANCE = 40.0f;
			/** 視線を飛ばす高さ（水面からの高さ） */
			constexpr float VIEW_RAY_HEIGHT = 10.0f;
		}


		OrcaController::OrcaController()
		{}


		OrcaController::~OrcaController()
		{}


		OrcaStateMachine* OrcaController::GetStateMachine() const
		{
			return m_target ? m_target->GetOrcaStateMachine() : nullptr;
		}


		const MasterOrcaParameter* OrcaController::GetParameter() const
		{
			OrcaStateMachine* sm = GetStateMachine();
			return sm ? sm->GetParameter() : nullptr;
		}


		const OrcaSeaGrid* OrcaController::GetSeaGrid() const
		{
			OrcaStateMachine* sm = GetStateMachine();
			return sm ? sm->GetSeaGrid() : nullptr;
		}


		const char* OrcaController::GetStateName(EnState state)
		{
			switch (state)
			{
			case EnState::Idle:       return "Idle";
			case EnState::Search:     return "Search";
			case EnState::Wandering:  return "Swim";
			case EnState::Roar:       return "Roar";
			case EnState::Chase:      return "Chase";
			case EnState::Attack:     return "Attack";
			case EnState::Stun:       return "Stun";
			default:                  return "Idle";
			}
		}


		void OrcaController::Update()
		{
			if (m_target == nullptr || GetParameter() == nullptr) return;

			// 追っている子が消えていないか先に確かめる（削除済みのポインタを触らないため）
			if (m_foundPenguin != nullptr && !IsTargetValid(m_foundPenguin))
			{
				m_foundPenguin = nullptr;
			}

			// 初回の Enter（EnemyController::Update :147-150 と同じ）
			if (!m_isInitialized)
			{
				EnterState(m_state);
				m_isInitialized = true;
			}

			// 遷移判定
			const EnState next = CheckState(m_state);
			if (next != EnState::Num && next != m_state)
			{
				ChangeState(next);
			}

			m_repathTimer -= g_gameTime->GetFrameDeltaTime();
			UpdateState(m_state);

			// 視界で子を探す（EnemyController::Update :168-179 と同じく、探索・巡回・追跡の間だけ）
			if (m_state == EnState::Search || m_state == EnState::Wandering || m_state == EnState::Chase)
			{
				if (ChildPenguin* found = FindTarget())
				{
					m_foundPenguin = found;
					m_lastKnownPenguinPos = found->GetTransform().m_position;
				}
			}

			GetStateMachine()->SetStateNameForLog(GetStateName(m_state));
		}


		void OrcaController::ChangeState(EnState next)
		{
			ExitState(m_state);
			m_state = next;
			m_elapsedTime = 0.0f;
			EnterState(m_state);
		}


		void OrcaController::EnterState(EnState state)
		{
			switch (state)
			{
			case EnState::Idle:      EnterIdle(); break;
			case EnState::Search:    EnterSearch(); break;
			case EnState::Wandering: EnterWandering(); break;
			case EnState::Roar:      EnterRoar(); break;
			case EnState::Chase:     EnterChase(); break;
			case EnState::Attack:    EnterAttack(); break;
			case EnState::Stun:      EnterStun(); break;
			default: break;
			}
		}


		void OrcaController::UpdateState(EnState state)
		{
			m_elapsedTime += g_gameTime->GetFrameDeltaTime();
			switch (state)
			{
			case EnState::Idle:      UpdateIdle(); break;
			case EnState::Search:    UpdateSearch(); break;
			case EnState::Wandering: UpdateWandering(); break;
			case EnState::Roar:      UpdateRoar(); break;
			case EnState::Chase:     UpdateChase(); break;
			case EnState::Attack:    UpdateAttack(); break;
			default: break;
			}
		}


		void OrcaController::ExitState(EnState state)
		{
			switch (state)
			{
			case EnState::Search:    ExitSearch(); break;
			case EnState::Wandering: ExitWandering(); break;
			case EnState::Roar:      ExitRoar(); break;
			case EnState::Chase:     ExitChase(); break;
			case EnState::Attack:    ExitAttack(); break;
			default: break;
			}
		}


		OrcaController::EnState OrcaController::CheckState(EnState state)
		{
			switch (state)
			{
			case EnState::Idle:      return CheckIdle();
			case EnState::Search:    return CheckSearch();
			case EnState::Wandering: return CheckWandering();
			case EnState::Roar:      return CheckRoar();
			case EnState::Chase:     return CheckChase();
			case EnState::Attack:    return CheckAttack();
			case EnState::Stun:      return CheckStun();
			default:                 return EnState::Idle;
			}
		}


		OrcaController::EnState OrcaController::GiveUpAndPatrol(const char* reason)
		{
			// クマはここで巣へ帰って眠るが、シャチは帰巣せずに巡回へ戻る（シャチには巣と睡眠が無い）。
			// 眠ってスタミナと満腹を戻す機会が無いので、ここで戻す（戻さないとスタミナ0のまま次の追跡をすぐ諦める）
			OrcaStateMachine* sm = GetStateMachine();
			sm->SetGiveUpRequested(false);
			if (OrcaStatus* status = sm->GetOwnerStatus())
			{
				status->FullRecoverStamina();
			}
			m_eatCount = 0;
			m_isFull = false;
			m_foundPenguin = nullptr;

			if (auto* lm = GameLogManager::GetInstance())
			{
				lm->QueueEvent({ { "ev", "orca_give_up" }, { "orca_id", m_target->GetLogId() }, { "reason", reason } });
			}
			return EnState::Wandering;
		}


		//============================================//
		// 待機（EnemyController.cpp :318-360）
		//============================================//

		void OrcaController::EnterIdle()
		{
			StopMove();
			m_foundPenguin = nullptr;

			// クマは CheckIdle のたびに 0〜5秒の乱数を引き直しているが、
			// シャチは入ったときに1回だけ 0〜idleTimeMax 秒を決める（毎フレーム引き直すとほぼ即座に抜けるため）
			const MasterOrcaParameter* parameter = GetParameter();
			const float maxTime = parameter ? parameter->idleTimeMax : 0.0f;
			m_idleTime = maxTime * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
		}


		void OrcaController::UpdateIdle()
		{
			OrcaStateMachine* sm = GetStateMachine();
			sm->SetDepthLevel(EnOrcaDepthLevel::Swim);
			StopMove();
			ListenNoise();
		}


		OrcaController::EnState OrcaController::CheckIdle()
		{
			// クマの「巣から離れすぎ → 帰巣」はシャチには無い（巣を持たないため）
			if (GetStateMachine()->IsSearch())
			{
				return EnState::Search;
			}
			if (m_elapsedTime > m_idleTime && !m_patrolPoints.empty())
			{
				return EnState::Wandering;
			}
			return EnState::Num;
		}


		//============================================//
		// 探索（音のした場所へ向かう。EnemyController.cpp :383-478）
		//============================================//

		void OrcaController::EnterSearch()
		{
			OrcaStateMachine* sm = GetStateMachine();
			sm->SetSearch(true);
			m_searchTimer = 0.0f;
			ResetStuckTimer();
			m_path.clear();

			// 音に気づいた合図の「？」を出し、ゆっくり振り向かせる
			sm->SetTurnSpeedMultiplier(GetParameter()->searchTurnMultiplier);
			BattleManager::GetInstance().NotifyOrcaReactionChanged(m_target, ui::EnCPReactionType::Question);
		}


		void OrcaController::UpdateSearch()
		{
			OrcaStateMachine* sm = GetStateMachine();
			const MasterOrcaParameter* parameter = GetParameter();
			sm->SetDepthLevel(EnOrcaDepthLevel::Swim);

			// 1. 見つけたら CheckSearch で咆哮へ移る
			if (m_foundPenguin != nullptr) return;

			// 2. 新しい音がしたら目標地点を更新する
			Vector3 loudestPos;
			const float totalNoise = NoiseManager::GetInstance().CalculateTotalNoiseAt(sm->GetSurfacePosition(), loudestPos);
			if (totalNoise >= parameter->noiseThreshold)
			{
				sm->SetSearchTargetPos(loudestPos);
				m_searchTimer = 0.0f;
			}

			// 3. 音源へ向かう。陸の音なら、海の中で最も近い点までしか行かない
			const float distance = SteerTo(sm->GetSearchTargetPos(), parameter->swimSpeed);
			UpdateStuckTimer();
			const bool arrived = (distance >= 0.0f && distance <= SEARCH_ARRIVE_DISTANCE) || m_stuckTimer >= STUCK_TIME_LIMIT;
			if (arrived)
			{
				StopMove();
				m_searchTimer += g_gameTime->GetFrameDeltaTime();
				if (m_searchTimer > parameter->searchGiveUpTime)
				{
					sm->SetSearch(false);
				}
			}
		}


		void OrcaController::ExitSearch()
		{
			OrcaStateMachine* sm = GetStateMachine();
			StopMove();
			sm->SetSearch(false);
			sm->SetTurnSpeedMultiplier(1.0f);
			m_searchTimer = 0.0f;
		}


		OrcaController::EnState OrcaController::CheckSearch()
		{
			if (m_foundPenguin != nullptr)
			{
				return EnState::Roar;
			}
			if (!GetStateMachine()->IsSearch())
			{
				return EnState::Wandering;
			}
			return EnState::Num;
		}


		//============================================//
		// 巡回（EnemyController.cpp :482-571）
		//============================================//

		void OrcaController::EnterWandering()
		{
			ResetStuckTimer();
			m_path.clear();
		}


		void OrcaController::UpdateWandering()
		{
			if (m_patrolPoints.empty()) return;

			GetStateMachine()->SetDepthLevel(EnOrcaDepthLevel::Swim);
			SteerTo(m_patrolPoints[m_patrolIndex], GetParameter()->swimSpeed);
			UpdateStuckTimer();
			ListenNoise();
		}


		void OrcaController::ExitWandering()
		{
			StopMove();
			if (m_patrolPoints.empty()) return;
			m_patrolIndex = (m_patrolIndex + 1) % static_cast<int>(m_patrolPoints.size());
		}


		OrcaController::EnState OrcaController::CheckWandering()
		{
			if (m_foundPenguin != nullptr)
			{
				return EnState::Roar;
			}
			if (GetStateMachine()->IsGiveUpRequested())
			{
				return GiveUpAndPatrol("debuff");
			}
			if (GetStateMachine()->IsSearch())
			{
				return EnState::Search;
			}
			if (m_patrolPoints.empty()) return EnState::Idle;

			Vector3 toPoint = m_patrolPoints[m_patrolIndex] - m_target->GetTransform().m_position;
			toPoint.y = 0.0f;
			if (toPoint.Length() <= GetParameter()->patrolArriveDistance)
			{
				return EnState::Idle;
			}
			// 一定時間進めなければ次の巡回点へ（EnemyController.cpp :564-568）
			if (m_stuckTimer >= STUCK_TIME_LIMIT)
			{
				return EnState::Idle;
			}
			return EnState::Num;
		}


		//============================================//
		// 咆哮（「！」を出して標的の方を向く。EnemyController.cpp :1126-1159）
		//============================================//

		void OrcaController::EnterRoar()
		{
			GetStateMachine()->SetRoar(true);
			StopMove();

			// 子を見つけた合図の「！」。発見はどの経路でも必ずここを通る
			// （クマの咆哮の画面演出 EnImpactType::BearRoar はシャチでは出さない）
			BattleManager::GetInstance().NotifyOrcaReactionChanged(m_target, ui::EnCPReactionType::Exclamation);
		}


		void OrcaController::UpdateRoar()
		{
			// 浮上するのは噛みつくときだけ。それまでは背びれだけを出して泳ぐ
			GetStateMachine()->SetDepthLevel(EnOrcaDepthLevel::Swim);
			const Vector3 targetPos = m_foundPenguin ? m_foundPenguin->GetTransform().m_position : m_lastKnownPenguinPos;
			FaceTo(targetPos);
		}


		void OrcaController::ExitRoar()
		{
			GetStateMachine()->SetRoar(false);
		}


		OrcaController::EnState OrcaController::CheckRoar()
		{
			// アニメが無いので、決まった時間で追跡に移る
			if (m_elapsedTime >= GetParameter()->roarDuration)
			{
				return m_foundPenguin ? EnState::Chase : GiveUpAndPatrol("lost");
			}
			return EnState::Num;
		}


		//============================================//
		// 追跡（EnemyController.cpp :575-730）
		//============================================//

		void OrcaController::EnterChase()
		{
			GetStateMachine()->SetIsChasing(true);
			m_path.clear();
		}


		void OrcaController::UpdateChase()
		{
			OrcaStateMachine* sm = GetStateMachine();
			const MasterOrcaParameter* parameter = GetParameter();
			sm->SetDepthLevel(EnOrcaDepthLevel::Swim);

			const Vector3 targetPos = m_foundPenguin ? m_foundPenguin->GetTransform().m_position : m_lastKnownPenguinPos;
			SteerTo(targetPos, parameter->runSpeed);

			// 追跡中はスタミナが減る（EnemyController.cpp :625-631）
			if (OrcaStatus* status = sm->GetOwnerStatus())
			{
				status->DecreaseStamina(parameter->staminaDrainRate * g_gameTime->GetFrameDeltaTime());
			}
		}


		void OrcaController::ExitChase()
		{
			GetStateMachine()->SetIsChasing(false);
			StopMove();
		}


		OrcaController::EnState OrcaController::CheckChase()
		{
			OrcaStateMachine* sm = GetStateMachine();
			const MasterOrcaParameter* parameter = GetParameter();
			OrcaStatus* status = sm->GetOwnerStatus();

			if (sm->IsGiveUpRequested())
			{
				m_foundPenguin = nullptr;
				return GiveUpAndPatrol("debuff");
			}
			// スタミナが尽きたら諦めて帰る（EnemyController.cpp :665-669）
			if (status != nullptr && status->IsStaminaEmpty())
			{
				m_foundPenguin = nullptr;
				return GiveUpAndPatrol("stamina");
			}

			if (m_foundPenguin != nullptr)
			{
				Vector3 diff = m_foundPenguin->GetTransform().m_position - sm->GetTransform().m_position;
				diff.y = 0.0f;
				// 離れすぎたら諦めて帰る（EnemyController.cpp :679-686）
				if (diff.LengthSq() > parameter->lostChaseDistance * parameter->lostChaseDistance)
				{
					m_foundPenguin = nullptr;
					return GiveUpAndPatrol("lost");
				}

				// 口先から攻撃距離に入ったら噛みつく（クマは体の中心から測る）
				Vector3 fromMouth = m_foundPenguin->GetTransform().m_position - sm->GetMouthPosition();
				fromMouth.y = 0.0f;
				if (fromMouth.LengthSq() <= parameter->attackDistance * parameter->attackDistance)
				{
					return EnState::Attack;
				}
				return EnState::Num;
			}

			// 標的が無効になった（上陸した・死んだ・渦潮に飲まれた）か、見失って最後に見た位置に着いた
			// クマは見失うと最後に見た位置まで行って待機に戻るが、シャチは標的が泳いでいなければ追えないので巡回へ戻る
			Vector3 toLastKnown = m_lastKnownPenguinPos - sm->GetTransform().m_position;
			toLastKnown.y = 0.0f;
			if (toLastKnown.LengthSq() <= ARRIVE_LAST_KNOWN_POS_DISTANCE * ARRIVE_LAST_KNOWN_POS_DISTANCE)
			{
				return EnState::Idle;
			}
			return GiveUpAndPatrol("lost");
		}


		//============================================//
		// 攻撃（EnemyController.cpp :807-991）
		//============================================//

		void OrcaController::EnterAttack()
		{
			OrcaStateMachine* sm = GetStateMachine();
			sm->SetAttack(true);
			sm->SetAttackFinished(false);
			sm->SetDepthLevel(EnOrcaDepthLevel::Attack);
		}


		void OrcaController::UpdateAttack()
		{
			OrcaStateMachine* sm = GetStateMachine();
			const MasterOrcaParameter* parameter = GetParameter();

			// 噛みつきながら前へ泳ぐ
			const Vector3 ahead = sm->GetTransform().m_position + sm->GetForward() * 100.0f;
			SteerTo(ahead, parameter->swimSpeed);

			// 噛みついた瞬間に食べた数を増やす（EnemyController.cpp :846-874。かまくらの処理はシャチには無い）
			if (sm->IsAttackImpact())
			{
				sm->SetAttackImpact(false);
				m_eatCount++;
				if (m_eatCount >= sm->GetOwnerStatus()->GetMaxEat())
				{
					m_isFull = true;
				}
			}

			// 攻撃に入った最初のフレームで当てる（EnemyController.cpp :878-957 と同じタイミング）
			if (m_foundPenguin == nullptr) return;

			// 密集陣ウルト中は、隊列の子への攻撃を無効化して弾き返す（EnemyController.cpp :884-921）
			auto* cpManager = ChildPenguinManager::GetInstance();
			if (BattleManager::GetInstance().IsBearAttackNullified()
				&& cpManager != nullptr
				&& cpManager->IsFollower(m_foundPenguin))
			{
				Vector3 away = sm->GetTransform().m_position - m_foundPenguin->GetTransform().m_position;
				away.y = 0.0f;
				if (away.LengthSq() <= FLT_EPSILON) away = sm->GetForward() * -1.0f;

				// 弾く移動も ResolveMove を通るので、海からは出ない
				sm->ApplyKnockback(away, parameter->nullifyKnockbackDistance);
				sm->SetStun(true);

				BattleManager::GetInstance().NotifyImpact(EnImpactType::BearNullified, sm->GetSurfacePosition());
				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->QueueEvent({
						{ "ev",         "orca_attack_nullified" },
						{ "orca_id",    m_target->GetLogId() },
						{ "penguin_id", m_foundPenguin->GetLogId() }
					});
				}
				m_foundPenguin = nullptr;
				return;
			}

			auto* childStateMachine = m_foundPenguin->GetStateMachine();
			const ChildPenguinStatus* childStatus = childStateMachine ? childStateMachine->GetChildPenguinStatus() : nullptr;
			if (childStatus != nullptr && !childStatus->IsDead())
			{
				const int orcaId = m_target->GetLogId();
				const int penguinId = m_foundPenguin->GetLogId();

				// 以前クマに噛まれかけたときの値が残っていると、そのクマが幽霊デバフで帰巣させられるので空にしておく
				childStateMachine->SetAttackerEnemy(nullptr);
				// 幽霊が消えた瞬間にシャチを巡回へ戻らせる（幽霊デバフ）。
				// シャチのポインタではなくIDだけを持ち、呼ばれた時点でマネージャーから引き直す（シーン破棄後でも安全にするため）
				childStateMachine->SetOnGhostHidden([orcaId]()
				{
					if (auto* manager = OrcaManager::GetInstance())
					{
						manager->NotifyVictimGhostHidden(orcaId);
					}
				});
				childStateMachine->Damage();

				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->QueueEvent({ { "ev", "orca_attack" }, { "orca_id", orcaId }, { "penguin_id", penguinId } });
					if (childStatus->IsDead())
					{
						lm->QueueEvent({ { "ev", "orca_kill" }, { "orca_id", orcaId }, { "penguin_id", penguinId } });
					}
				}
			}

			// 一度攻撃に入ったら標的は手放す（EnemyController.cpp :956）
			m_foundPenguin = nullptr;
		}


		void OrcaController::ExitAttack()
		{
			OrcaStateMachine* sm = GetStateMachine();
			sm->SetAttack(false);
			sm->SetAttackFinished(false);
			StopMove();
		}


		OrcaController::EnState OrcaController::CheckAttack()
		{
			OrcaStateMachine* sm = GetStateMachine();

			// 弾き返されたら気絶へ
			if (sm->IsStun())
			{
				return EnState::Stun;
			}
			if (!sm->IsAttackFinished())
			{
				return EnState::Num;
			}
			// 満腹なら帰る
			if (m_isFull)
			{
				return GiveUpAndPatrol("full");
			}
			// まだ標的がいれば追う
			if (m_foundPenguin != nullptr)
			{
				return EnState::Chase;
			}
			return EnState::Idle;
		}


		//============================================//
		// 気絶（EnemyStunState。解除はステートマシン側の時間で行う）
		//============================================//

		void OrcaController::EnterStun()
		{
			StopMove();
			m_foundPenguin = nullptr;
			// 噛みつきで浮上していた場合も、背びれだけの深さへ戻す
			GetStateMachine()->SetDepthLevel(EnOrcaDepthLevel::Swim);
		}


		OrcaController::EnState OrcaController::CheckStun()
		{
			if (!GetStateMachine()->IsStun())
			{
				return EnState::Idle;
			}
			return EnState::Num;
		}


		//============================================//
		// 幽霊デバフ
		//============================================//

		void OrcaController::OnVictimGhostHidden()
		{
			OrcaStateMachine* sm = GetStateMachine();
			if (sm == nullptr) return;
			// クマはここで帰巣を始めるが、シャチは追跡や巡回をやめて巡回へ戻る（判定は CheckWandering / CheckChase で拾う）
			sm->SetGiveUpRequested(true);
		}


		//============================================//
		// デバッグ欄のボタン
		//============================================//

		void OrcaController::DebugDrainStamina()
		{
			OrcaStateMachine* sm = GetStateMachine();
			if (OrcaStatus* status = sm ? sm->GetOwnerStatus() : nullptr)
			{
				status->DecreaseStamina(status->GetStamina());
			}
		}


		void OrcaController::DebugKnockbackTowardLand()
		{
			OrcaStateMachine* sm = GetStateMachine();
			const OrcaSeaGrid* grid = GetSeaGrid();
			if (sm == nullptr || grid == nullptr || !grid->IsBuilt()) return;

			// 16方向に少しずつ進めて、最も近くで航行可能でなくなる方向を「陸の方向」とする
			const Vector3 origin = sm->GetTransform().m_position;
			constexpr int DIRECTIONS = 16;
			constexpr float STEP = 10.0f;
			constexpr float MAX_DISTANCE = 2000.0f;
			Vector3 bestDirection = sm->GetForward();
			float bestDistance = FLT_MAX;
			for (int i = 0; i < DIRECTIONS; i++)
			{
				const float angle = Math::PI * 2.0f * i / DIRECTIONS;
				const Vector3 direction(sinf(angle), 0.0f, cosf(angle));
				for (float d = STEP; d < MAX_DISTANCE; d += STEP)
				{
					if (!grid->IsNavigable(origin + direction * d))
					{
						if (d < bestDistance)
						{
							bestDistance = d;
							bestDirection = direction;
						}
						break;
					}
				}
			}
			// 陸までの距離より長く弾く。ResolveMove を通るので、岸の手前で止まるはず
			sm->ApplyKnockback(bestDirection, (bestDistance < FLT_MAX ? bestDistance : 0.0f) + 300.0f);
		}


		//============================================//
		// 標的・音
		//============================================//

		bool OrcaController::IsTargetValid(const ChildPenguin* penguin) const
		{
			if (penguin == nullptr) return false;
			auto* cpManager = ChildPenguinManager::GetInstance();
			if (cpManager == nullptr) return false;

			const auto& list = cpManager->GetChildPenguin();
			auto it = std::find(list.begin(), list.end(), penguin);
			if (it == list.end()) return false;

			ChildPenguin* child = *it;
			ChildPenguinStateMachine* childStateMachine = child->GetStateMachine();
			if (childStateMachine == nullptr) return false;
			const ChildPenguinStatus* status = childStateMachine->GetChildPenguinStatus();
			if (status != nullptr && status->IsDead()) return false;

			// 海にいる（泳いでいる）子だけを標的にする。上陸した子・渦潮に飲まれた子は追わない
			return childStateMachine->IsSwimming();
		}


		ChildPenguin* OrcaController::FindTarget() const
		{
			auto* cpManager = ChildPenguinManager::GetInstance();
			if (cpManager == nullptr) return nullptr;

			const OrcaStateMachine* sm = GetStateMachine();
			const MasterOrcaParameter* parameter = GetParameter();
			const Vector3 eye = sm->GetSurfacePosition();
			const Vector3 forward = sm->GetForward();
			const float viewDistanceSq = parameter->viewDistance * parameter->viewDistance;
			const float cosViewAngle = cosf(Math::PI / 180.0f * parameter->viewAngleDeg);

			for (ChildPenguin* penguin : cpManager->GetChildPenguin())
			{
				if (!IsTargetValid(penguin)) continue;

				const Vector3 penguinPos = penguin->GetTransform().m_position;
				Vector3 diff = penguinPos - eye;
				diff.y = 0.0f;
				if (diff.LengthSq() > viewDistanceSq) continue;
				if (diff.LengthSq() <= FLT_EPSILON) return penguin;
				diff.Normalize();

				// 視界の向きは体の正面を使う（EnemyController.cpp :232-249 と同じ理由）
				if (forward.Dot(diff) < cosViewAngle) continue;

				// 地形に遮られていないか。水面の少し上から水平に視線を飛ばす
				const Vector3 rayFrom(eye.x, eye.y + VIEW_RAY_HEIGHT, eye.z);
				const Vector3 rayTo(penguinPos.x, eye.y + VIEW_RAY_HEIGHT, penguinPos.z);
				nsBeastEngine::nsCollision::RaycastHit hit;
				if (nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(
					rayFrom, rayTo, hit, nsBeastEngine::nsCollision::ALL_COLLISION_ATTRIBUTE_MASK))
				{
					if (hit.colObject && (hit.colObject->getUserIndex() & nsBeastEngine::nsCollision::CollisionAttribute::Ground))
					{
						continue;
					}
				}
				return penguin;
			}
			return nullptr;
		}


		void OrcaController::ListenNoise()
		{
			OrcaStateMachine* sm = GetStateMachine();
			Vector3 loudestPos;
			// 音の距離は高さも含むので、水面下の体ではなく水面の点で聞く
			const float totalNoise = NoiseManager::GetInstance().CalculateTotalNoiseAt(sm->GetSurfacePosition(), loudestPos);
			if (totalNoise >= GetParameter()->noiseThreshold)
			{
				sm->SetSearch(true);
				sm->SetSearchTargetPos(loudestPos);
			}
		}


		//============================================//
		// 移動
		//============================================//

		float OrcaController::SteerTo(const Vector3& goal, float speed)
		{
			OrcaStateMachine* sm = GetStateMachine();
			const OrcaSeaGrid* grid = GetSeaGrid();
			const Vector3 position = sm->GetTransform().m_position;

			if (grid == nullptr || !grid->IsBuilt())
			{
				Vector3 direction = goal - position;
				direction.y = 0.0f;
				const float distance = direction.Length();
				if (distance > FLT_EPSILON) direction.Normalize();
				sm->SetMoveDirection(direction);
				sm->SetMoveSpeed(speed);
				return distance;
			}

			// 目標が航行可能域の外なら、自分と同じ連結域の中で最も近い点を目標にする（陸の音・浅瀬の子・浅い所の渦潮）
			const int component = grid->GetComponent(position);
			Vector3 target;
			if (!grid->FindNearestNavigable(goal, component, target))
			{
				StopMove();
				return -1.0f;
			}

			Vector3 aim = target;
			if (grid->HasSeaLine(position, target))
			{
				m_path.clear();
			}
			else
			{
				// 見通しが無ければ経路に沿う。目標が大きく動いたときだけ探し直す
				Vector3 goalMove = target - m_pathGoal;
				goalMove.y = 0.0f;
				const float repathDistance = grid->GetSettings().cellSize * REPATH_GOAL_MOVE_CELLS;
				const bool needRepath = m_path.empty() || goalMove.LengthSq() > repathDistance * repathDistance;
				if (needRepath && m_repathTimer <= 0.0f)
				{
					BEAST_PROFILE_SCOPE(u8"シャチ経路探索");
					if (!grid->FindPath(position, target, m_path))
					{
						m_path.clear();
					}
					m_pathIndex = 0;
					m_pathGoal = target;
					m_repathTimer = REPATH_INTERVAL;
				}

				if (!m_path.empty())
				{
					// 見通せる先の経由点まで進める（角を早めに切る）
					while (m_pathIndex + 1 < static_cast<int>(m_path.size()) && grid->HasSeaLine(position, m_path[m_pathIndex + 1]))
					{
						m_pathIndex++;
					}
					Vector3 toWaypoint = m_path[m_pathIndex] - position;
					toWaypoint.y = 0.0f;
					const float arrive = grid->GetSettings().cellSize * WAYPOINT_ARRIVE_CELLS;
					if (toWaypoint.LengthSq() < arrive * arrive && m_pathIndex + 1 < static_cast<int>(m_path.size()))
					{
						m_pathIndex++;
					}
					aim = m_path[m_pathIndex];
				}
			}

			Vector3 direction = aim - position;
			direction.y = 0.0f;
			if (direction.LengthSq() > FLT_EPSILON) direction.Normalize();
			sm->SetMoveDirection(direction);
			sm->SetMoveSpeed(speed);

			Vector3 toTarget = target - position;
			toTarget.y = 0.0f;
			return toTarget.Length();
		}


		void OrcaController::StopMove()
		{
			OrcaStateMachine* sm = GetStateMachine();
			if (sm == nullptr) return;
			sm->SetMoveDirection(Vector3::Zero);
			sm->SetMoveSpeed(0.0f);
		}


		void OrcaController::FaceTo(const Vector3& point)
		{
			// 速さ 0 で移動方向だけ与えると、その場で向きだけ変わる（MoveInSea は向きを turnSpeedDeg で回す）
			OrcaStateMachine* sm = GetStateMachine();
			Vector3 direction = point - sm->GetTransform().m_position;
			direction.y = 0.0f;
			if (direction.LengthSq() <= FLT_EPSILON) return;
			direction.Normalize();
			sm->SetMoveDirection(direction);
			sm->SetMoveSpeed(0.0f);
		}


		void OrcaController::ResetStuckTimer()
		{
			m_stuckTimer = 0.0f;
			m_lastCheckPosition = m_target->GetTransform().m_position;
		}


		void OrcaController::UpdateStuckTimer()
		{
			const Vector3 current = m_target->GetTransform().m_position;
			Vector3 moved = current - m_lastCheckPosition;
			moved.y = 0.0f;
			if (moved.LengthSq() < STUCK_MOVE_THRESHOLD * STUCK_MOVE_THRESHOLD)
			{
				m_stuckTimer += g_gameTime->GetFrameDeltaTime();
			}
			else
			{
				m_stuckTimer = 0.0f;
				m_lastCheckPosition = current;
			}
		}
	}
}
