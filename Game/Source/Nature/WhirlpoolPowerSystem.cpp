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
					newInfo.isPushing = false;
					newInfo.angle = atan2f(newInfo.toTargetVector.z, newInfo.toTargetVector.x);

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
			const float                     wpRadius = (param != nullptr) ? param->whirlpoolRadius : 200.0f;

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
					it->isPushing = false;
					it->target->GetStateMachine()->SetIsInWhirlpool(false);
					m_cpManager->UnregisterDowning(it->target);
					++it;
					continue;
				}

				// 渦潮から子ペンギンへのベクトルを更新
				it->toTargetVector = it->target->GetTransform().m_position - whirlpoolPos;

				const Vector3 toTargetXZ = Vector3(it->toTargetVector.x, 0.0f, it->toTargetVector.z);
				const float   distXZ = toTargetXZ.Length();

				// 渦潮の範囲内に入ったら影響フラグを立てる
				if (!it->isAffected && distXZ <= wpRadius)
				{
					it->isAffected = true;
					it->isPushing = false;
					it->angle = atan2f(it->toTargetVector.z, it->toTargetVector.x);
				}

				// 影響を受けているペンギンのフェーズ処理
				if (it->isAffected)
				{
					it->target->GetStateMachine()->SetIsInWhirlpool(true);

					if (it->isPushing)
					{
						UpdatePush(*it, deltaTime);
					}
					else
					{
						UpdateAttract(*it, deltaTime);
					}
				}

				++it;
			}
		}


		void WhirlpoolPowerSytem::UpdateAttract(WhirlpoolPowerInfo& info, float deltaTime)
		{
			const MasterWhirlpoolParameter* param = GetParam();
			const float                     attractSpeed = (param != nullptr) ? param->attractSpeed : 30.0f;
			const float                     attractThreshold = (param != nullptr) ? param->attractThreshold : 10.0f;

			const Vector3 toTargetXZ = Vector3(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
			const float   currentRadius = toTargetXZ.Length();

			// 中心に十分近づいたら押し出しフェーズへ移行
			if (currentRadius <= attractThreshold)
			{
				info.isPushing = true;
				return;
			}

			// 半径を縮めて渦巻き移動
			const float newRadius = currentRadius - attractSpeed * deltaTime;
			UpdateSpiral(info, newRadius, deltaTime);
		}


		void WhirlpoolPowerSytem::UpdatePush(WhirlpoolPowerInfo& info, float deltaTime)
		{
			const MasterWhirlpoolParameter* param = GetParam();
			const float                     pushSpeed = (param != nullptr) ? param->pushSpeed : 3.0f;
			const float                     wpRadius = (param != nullptr) ? param->whirlpoolRadius : 200.0f;

			const Vector3 toTargetXZ = Vector3(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
			const float   currentRadius = toTargetXZ.Length();

			// 渦潮範囲外に出たら影響終了
			if (currentRadius >= wpRadius * 1.5f)
			{
				info.isAffected = false;
				info.isPushing = false;
				return;
			}

			// 半径を広げて渦巻き移動
			const float newRadius = currentRadius + pushSpeed * deltaTime;
			UpdateSpiral(info, newRadius, deltaTime);
		}


		void WhirlpoolPowerSytem::UpdateSpiral(WhirlpoolPowerInfo& info, float newRadius, float deltaTime)
		{
			const MasterWhirlpoolParameter* param = GetParam();
			const float                     rotateSpeed = (param != nullptr) ? param->rotateSpeed : 3.0f;

			const Vector3& whirlpoolPos = m_owner->GetTransform().m_position;

			// 角度を回転させる（渦巻き）
			info.angle += rotateSpeed * deltaTime;

			// 極座標 → デカルト座標でXZ位置を更新
			Vector3 pos = info.target->GetTransform().m_position;
			pos.x = whirlpoolPos.x + newRadius * cosf(info.angle);
			pos.z = whirlpoolPos.z + newRadius * sinf(info.angle);

			const Vector3 prevPos = info.target->GetCharacterController()->Execute(pos, deltaTime);
			info.target->GetStateMachine()->SetPosition(prevPos);
		}
	}
}