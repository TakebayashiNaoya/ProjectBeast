/**
 * @file WhirlpoolPowerSystem.cpp
 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "Whirlpool.h"
#include "WhirlpoolPowerSystem.h"
#include "WhirlpoolParameter.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Core/ParameterManager.h"
#include <random>

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
			 * @brief ランダムな浮動小数点値を生成する
			 * @param min 最小値
			 * @param max 最大値
			 * @return min ～ max のランダム値
			 */
			float GenerateRandomFloat(const float min, const float max)
			{
				static std::mt19937 engine(std::random_device{}());
				std::uniform_real_distribution<float> dist(min, max);
				return dist(engine);
			}
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
					it->individualOrbitOffset = GenerateRandomFloat(-orbitOffsetVariation, orbitOffsetVariation);
					it->individualRotateScale = 1.0f + GenerateRandomFloat(-rotateScaleVariation, rotateScaleVariation);

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
			return sqrtf(dx * dx + dz * dz) <= m_cpManager->GetJoinRadius();
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
					info.radiusOffsetTarget = GenerateRandomFloat(-orbitRadiusVariation, orbitRadiusVariation);
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

			const Vector3 prevPos = info.target->GetCharacterController()->Execute(pos, deltaTime);
			info.target->GetStateMachine()->SetPosition(prevPos);
		}
	}
}