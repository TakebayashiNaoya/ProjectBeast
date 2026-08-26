/**
 * @file WhirlpoolPowerSystem.cpp
 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Achivement/AchievementManager.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Nature/Ocean.h"
#include "Source/Util/RandomDevice.h"
#include "Whirlpool.h"
#include "WhirlpoolParameter.h"
#include "WhirlpoolPowerSystem.h"


using namespace nsK2EngineLow;

namespace app
{
	namespace nature
	{
		namespace
		{
			/**
			 * @brief パラメーターを取得するヘルパー関数
			 * @return パラメーターポインタ（取得失敗時はnullptr）
			 */
			const MasterWhirlpoolParameter* GetParam()
			{
				return core::ParameterManager::Get()->GetParameter<MasterWhirlpoolParameter>();
			}

			/**
			 * @brief 同じ子の飲み込みを別の被害として数え直すまでの間隔（秒）
			 * @details 渦潮の縁では捕獲と救出が数フレーム周期で繰り返されるため、
			 *          この間隔を空けずに再捕獲されたものは同じ被害の継続として扱う
			 */
			constexpr float CAPTURE_RECOUNT_INTERVAL = 3.0f;

			/** 吸い込み中に海面（波の高さ）からどれだけ沈めて回すか */
			constexpr float CAPTURED_SUBMERGE_DEPTH = 6.0f;
		}


		void WhirlpoolPowerSytem::Start()
		{}


		void WhirlpoolPowerSytem::Update()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			UpdateWhirlpoolInfo(deltaTime);
		}


		void WhirlpoolPowerSytem::Render(RenderContext& rc)
		{}


		WhirlpoolPowerSytem::WhirlpoolPowerSytem(Whirlpool* ownerWhirlpool)
			: m_owner(ownerWhirlpool)
			, m_cpManager(nullptr)
		{
			m_cpManager = actor::ChildPenguinManager::GetInstance();
			InitializeWhirlpoolInfo();
		}


		void WhirlpoolPowerSytem::InitializeWhirlpoolInfo()
		{
			// ChildPenguinManager が生成されていない文脈（リプレイ再生など）では何もしない
			if (!m_cpManager) return;

			const Vector3& whirlpoolPos = m_owner->GetTransform().m_position;

			auto childPenguins = m_cpManager->GetChildPenguin();
			auto& oldInfo = m_wpPowerInfos;

			// 子ペンギンの数が変わっている場合は、情報リストを再構築する
			if (oldInfo.empty() || oldInfo.size() != childPenguins.size())
			{
				oldInfo.clear();

				std::vector<WhirlpoolPowerInfo> newInfos;

				for (auto& cp : childPenguins)
				{
					WhirlpoolPowerInfo newInfo;
					newInfo.toTargetVector = cp->GetTransform().m_position - whirlpoolPos;
					newInfo.target = cp;
					newInfo.isAffected = false;
					newInfo.angle = atan2f(newInfo.toTargetVector.z, newInfo.toTargetVector.x);
					newInfo.radiusOffset = 0.0f;
					newInfo.radiusOffsetTarget = 0.0f;
					newInfo.individualOrbitOffset = 0.0f;
					newInfo.individualRotateScale = 1.0f;

					newInfos.push_back(newInfo);
				}

				oldInfo = std::move(newInfos);
			}
		}


		void WhirlpoolPowerSytem::UpdateWhirlpoolInfo(const float deltaTime)
		{
			const Vector3& whirlpoolPos = m_owner->GetTransform().m_position;

			InitializeWhirlpoolInfo();

			const MasterWhirlpoolParameter* param = GetParam();
			const float wpRadius = (param != nullptr) ? param->whirlpoolRadius : 200.0f;

			// イテレータで走査し、targetがnullptrのエントリを安全に削除する
			for (auto it = m_wpPowerInfos.begin(); it != m_wpPowerInfos.end(); )
			{
				if (it->target == nullptr)
				{
					it = m_wpPowerInfos.erase(it);
					continue;
				}

				// 渦潮が消滅済みなら影響を解除してスキップ
				if (m_owner->GetState() == Whirlpool::EnWhirlpoolState::None)
				{
					it->isAffected = false;
					it->target->GetStateMachine()->SetIsInWhirlpool(false);
					m_cpManager->UnregisterDowning(it->target);
					++it;
					continue;
				}

				// 渦潮から子ペンギンへのベクトルを更新
				it->toTargetVector = it->target->GetTransform().m_position - whirlpoolPos;

				// Bigger状態ではスケール比率に応じて判定半径を動的に広げる
				float effectiveRadius = wpRadius;
				if (m_owner->GetState() == Whirlpool::EnWhirlpoolState::Bigger)
				{
					const float currentScaleXZ = m_owner->GetTransform().m_scale.x;
					const float maxScaleXZ = m_owner->GetMaxScaleXZ();
					const float ratio = (maxScaleXZ > 0.0f) ? (currentScaleXZ / maxScaleXZ) : 1.0f;
					effectiveRadius = wpRadius * ratio;
				}

				// 捕獲判定
				if (ShouldCapture(*it, effectiveRadius))
				{
					const float orbitOffsetVariation = (param != nullptr) ? param->orbitOffsetVariation : 30.0f;
					const float rotateScaleVariation = (param != nullptr) ? param->rotateScaleVariation : 0.3f;

					it->isAffected = true;
					it->angle = atan2f(it->toTargetVector.z, it->toTargetVector.x);
					it->radiusOffset = 0.0f;
					it->radiusOffsetTarget = 0.0f;
					it->individualOrbitOffset = util::RandomDevice::Random(-orbitOffsetVariation, orbitOffsetVariation);
					it->individualRotateScale = 1.0f + util::RandomDevice::Random(-rotateScaleVariation, rotateScaleVariation);

					// 飲まれた記録は隊列から外す前に取る。
					// 外した後だと IsFollower() が必ず偽になり、隊列にいた子の被害を1件も数えられない
					const bool wasFollower = m_cpManager->IsFollower(it->target);

					// 渦潮の縁では捕獲と救出が数フレーム周期で繰り返されるため、
					// 直前の捕獲から間隔が空いたものだけを新しい被害として数える。
					// 残り時間は減っていくので、前回の時刻から現在時刻を引いたものが経過秒数になる
					const int   logId = it->target->GetLogId();
					const float now   = TimeManager::GetInstance().GetCurTime();
					const auto  found = m_lastCaptureTimes.find(logId);
					const bool  isNewCapture =
						(found == m_lastCaptureTimes.end()) ||
						(found->second - now >= CAPTURE_RECOUNT_INTERVAL);
					m_lastCaptureTimes[logId] = now;

					if (isNewCapture)
					{
						if (auto* lm = GameLogManager::GetInstance())
						{
							lm->QueueEvent({
								{ "ev",           "whirlpool_capture" },
								{ "penguin_id",   it->target->GetLogId() },
								{ "penguin_type", it->target->GetChildPenguinTypeStr() },
								{ "was_follower", wasFollower }
							});
						}

						// アチーブメントは「隊列にいた子を持っていかれた」回数を数える。
						// やんちゃが自分から飛び込んだ分はプレイヤーの被害ではないため対象外
						if (wasFollower)
						{
							if (auto* am = app::achievement::AchievementManager::GetInstance())
							{
								am->AddWhirlpoolCapture();
							}
						}
					}

					// 渦潮に飲まれた瞬間に隊から抜ける
					m_cpManager->RemoveFollower(it->target);
				}

				// 影響を受けているペンギンのフェーズ処理
				if (it->isAffected)
				{
					// 救出判定
					if (ShouldRescue(*it))
					{
						it->isAffected = false;
						it->target->GetStateMachine()->SetIsInWhirlpool(false);
						m_cpManager->UnregisterDowning(it->target);
						m_cpManager->AddFollower(it->target);
						++it;
						continue;
					}

					it->target->GetStateMachine()->SetIsInWhirlpool(true);
					UpdateAttract(*it, deltaTime);
				}

				++it;
			}
		}


		bool WhirlpoolPowerSytem::ShouldCapture(const WhirlpoolPowerInfo& info, float effectiveRadius) const
		{
			if (info.isAffected) return false;

			const Vector3 toTargetXZ(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
			if (toTargetXZ.Length() > effectiveRadius) return false;

			return !m_cpManager->IsWhirlpoolImmune(info.target);
		}


		bool WhirlpoolPowerSytem::ShouldRescue(const WhirlpoolPowerInfo& info) const
		{
			if (!info.isAffected) return false;
			if (!m_cpManager->HasWhirlpoolResistance()) return false;

			const Vector3 daddyPos = m_cpManager->GetDaddyPosition();
			const float dx = info.target->GetTransform().m_position.x - daddyPos.x;
			const float dz = info.target->GetTransform().m_position.z - daddyPos.z;
			return sqrtf(dx * dx + dz * dz) <= m_cpManager->GetJoinRadius(m_cpManager->GetFollowersNum());
		}


		void WhirlpoolPowerSytem::UpdateAttract(WhirlpoolPowerInfo& info, float deltaTime)
		{
			const MasterWhirlpoolParameter* param = GetParam();
			const float attractSpeed = (param != nullptr) ? param->attractSpeed : 30.0f;
			const float orbitRadius = (param != nullptr) ? param->orbitRadius : 80.0f;
			const float orbitRadiusVariation = (param != nullptr) ? param->orbitRadiusVariation : 20.0f;
			const float wpRadius = (param != nullptr) ? param->whirlpoolRadius : 200.0f;

			// Smaller状態では渦潮の現在スケールに比例して軌道半径の上限を縮める
			float effectiveOrbitRadius = orbitRadius + info.individualOrbitOffset;
			if (m_owner->GetState() == Whirlpool::EnWhirlpoolState::Smaller)
			{
				const float currentScaleXZ = m_owner->GetTransform().m_scale.x;
				const float maxScaleXZ = m_owner->GetMaxScaleXZ();
				const float ratio = (maxScaleXZ > 0.0f) ? (currentScaleXZ / maxScaleXZ) : 1.0f;
				const float scaledWpRadius = wpRadius * ratio;
				effectiveOrbitRadius = min(effectiveOrbitRadius, scaledWpRadius);
			}

			// 負にならないようにクランプ
			effectiveOrbitRadius = max(effectiveOrbitRadius, 0.0f);

			const bool  isSmaller = (m_owner->GetState() == Whirlpool::EnWhirlpoolState::Smaller);
			const Vector3 toTargetXZ = Vector3(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
			const float currentRadius = toTargetXZ.Length();

			if (isSmaller)
			{
				// Smaller状態では毎フレーム強制的に effectiveOrbitRadius に追従させる
				// attractSpeed に依存せず縮小速度に必ず追いつく
				info.radiusOffset = 0.0f;
				info.radiusOffsetTarget = 0.0f;
				UpdateSpiral(info, effectiveOrbitRadius, deltaTime);
			}
			else if (currentRadius > effectiveOrbitRadius)
			{
				// 軌道半径より外側にいる：effectiveOrbitRadius に向かって近づく
				const float newRadius = currentRadius - attractSpeed * deltaTime;
				UpdateSpiral(info, max(newRadius, effectiveOrbitRadius), deltaTime);
			}
			else
			{
				// 軌道半径に到達：radiusOffset を目標に向けて近づけながら軌道を維持する
				const float step = attractSpeed * deltaTime;
				const float diff = info.radiusOffsetTarget - info.radiusOffset;

				if (fabsf(diff) <= step)
				{
					// 目標に到達したら次のランダム目標をセットする
					info.radiusOffset = info.radiusOffsetTarget;
					info.radiusOffsetTarget = util::RandomDevice::Random(-orbitRadiusVariation, orbitRadiusVariation);
				}
				else
				{
					// 目標に向かって一定速度で近づく
					info.radiusOffset += (diff > 0.0f ? step : -step);
				}

				// radiusOffset が effectiveOrbitRadius を超えないようにクランプする
				info.radiusOffset = min(info.radiusOffset, 0.0f);
				const float finalRadius = effectiveOrbitRadius + info.radiusOffset;
				UpdateSpiral(info, max(finalRadius, 0.0f), deltaTime);
			}
		}


		void WhirlpoolPowerSytem::OnPenguinDestroyed(actor::ChildPenguin* penguin)
		{
			// target を直接 nullptr 化する。実際のリストからの除去は
			// UpdateWhirlpoolInfo() 側の「target == nullptr なら erase」処理に任せる
			for (auto& info : m_wpPowerInfos)
			{
				if (info.target == penguin)
				{
					info.target = nullptr;
					break;
				}
			}
		}


		void WhirlpoolPowerSytem::UpdateSpiral(WhirlpoolPowerInfo& info, float newRadius, float deltaTime)
		{
			const MasterWhirlpoolParameter* param = GetParam();
			const float                     rotateSpeed = (param != nullptr) ? param->rotateSpeed : 3.0f;

			const Vector3& whirlpoolPos = m_owner->GetTransform().m_position;

			// 角度を逆方向に回転させる（渦巻き）。個体固有の速度倍率を適用する
			info.angle -= rotateSpeed * info.individualRotateScale * deltaTime;

			// 極座標 → デカルト座標でXZ位置を更新
			Vector3 pos = info.target->GetTransform().m_position;
			pos.x = whirlpoolPos.x + newRadius * cosf(info.angle);
			pos.z = whirlpoolPos.z + newRadius * sinf(info.angle);

			// Yは海面（波を含む）に少し沈めた高さへ毎フレーム吸着させる
			if (const auto* ocean = Ocean::GetInstance())
			{
				pos.y = ocean->SampleWaveHeight(pos.x, pos.z) - CAPTURED_SUBMERGE_DEPTH;
			}

			/**
			 * 吸い込み中はキノマティックに動かす（かまくらイベントと同じ方式）。
			 * 以前は CharacterController::Execute() で衝突解決込みで動かしていたが、
			 * 水路の岸近くの渦潮では「旋回の目標位置 vs 岸コリジョン」が毎フレーム
			 * 衝突して押し戻され、カクカク震える見た目になっていた。
			 * 渦潮に捕まっている間は吸い込みが最優先なので、衝突は解決しない
			 */
			info.target->GetCharacterController()->SetPosition(pos);
			info.target->GetCharacterController()->RequestTeleport();
			info.target->GetStateMachine()->SetPosition(pos);
		}
	}
}