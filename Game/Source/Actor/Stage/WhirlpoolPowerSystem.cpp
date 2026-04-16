/**
 * @file WhirlpoolPowerSystem.cpp
 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Stage/Whirlpool.h"
#include "WhirlpoolPowerSystem.h"

using namespace nsK2EngineLow;

namespace app
{
	namespace actor
	{

		namespace
		{
			/** 渦潮の影響範囲半径 */
			constexpr float WHIRLPOOL_RADIUS = 200.0f;
			/** 引き寄せ速度（半径方向） */
			constexpr float ATTRACT_SPEED = 30.0f;
			/** 押し出し速度（半径方向） */
			constexpr float PUSH_SPEED = 3.0f;
			/** 渦巻き回転速度（ラジアン/秒） */
			constexpr float ROTATE_SPEED = 3.0f;
			/** 引き寄せ完了とみなす中心からの距離 */
			constexpr float ATTRACT_THRESHOLD = 10.0f;
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
			m_cpManager = ChildPenguinManager::GetInstance();

			InitializeWhirlpoolInfo();
		}


		void WhirlpoolPowerSytem::InitializeWhirlpoolInfo()
		{
			const Vector3& whirlpoolPos = m_owner->GetTransform().m_position;

			auto& childPenguins = m_cpManager->GetChildPenguin();
			auto& oldInfo = m_wpPowerInfos;

			// 子ペンギンの数が変わっている場合は、情報リストを再構築する
			if (oldInfo.empty() || oldInfo.size() != childPenguins.size())
			{
				oldInfo.clear();

				std::vector<WhirlpoolPowerInfo> newInfos;

				for (auto& cp : childPenguins)
				{
					WhirlpoolPowerInfo newInfo;
					// 渦潮から子ペンギンへのベクトル
					newInfo.toTargetVector = cp->GetTransform().m_position - whirlpoolPos;
					newInfo.target = cp;
					newInfo.isAffected = false;
					newInfo.isPushing = false;
					// 初期角度をXZ平面で計算
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

			// イテレータで走査し、target が nullptr のエントリを安全に削除する
			for (auto it = m_wpPowerInfos.begin(); it != m_wpPowerInfos.end(); )
			{
				if (it->target == nullptr)
				{
					// ループ中に erase し、次の有効イテレータを受け取る
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

				// XZ平面での距離を計算（Y成分を0にして Length() を利用）
				const Vector3 toTargetXZ = Vector3(it->toTargetVector.x, 0.0f, it->toTargetVector.z);
				const float distXZ = toTargetXZ.Length();

				// 渦潮の範囲内に入ったら影響フラグを立てる
				if (!it->isAffected && distXZ <= WHIRLPOOL_RADIUS)
				{
					it->isAffected = true;
					it->isPushing = false;
					// 範囲に入った瞬間の角度を記録
					it->angle = atan2f(it->toTargetVector.z, it->toTargetVector.x);
				}

				// 影響を受けているペンギンのフェーズ処理
				if (it->isAffected)
				{
					// 子ペンギンが渦潮の影響を受けると渦潮が消えるまで変わらない
					it->target->GetStateMachine()->SetIsInWhirlpool(true);

					if (it->isPushing)
					{
						// 押し出しフェーズ
						UpdatePush(*it, deltaTime);
					}
					else
					{
						// 引き寄せフェーズ
						UpdateAttract(*it, deltaTime);
					}
				}

				++it;
			}
		}


		void WhirlpoolPowerSytem::UpdateAttract(WhirlpoolPowerInfo& info, float deltaTime)
		{
			// XZ平面での現在の距離（半径）
			const Vector3 toTargetXZ = Vector3(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
			const float currentRadius = toTargetXZ.Length();

			// 中心に十分近づいたら押し出しフェーズへ移行
			if (currentRadius <= ATTRACT_THRESHOLD)
			{
				info.isPushing = true;
				return;
			}

			// 半径を縮めて渦巻き移動
			const float newRadius = currentRadius - ATTRACT_SPEED * deltaTime;
			UpdateSpiral(info, newRadius, deltaTime);
		}


		void WhirlpoolPowerSytem::UpdatePush(WhirlpoolPowerInfo& info, float deltaTime)
		{
			// XZ平面での現在の距離（半径）
			const Vector3 toTargetXZ = Vector3(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
			const float currentRadius = toTargetXZ.Length();

			// 渦潮範囲外に出たら影響終了（渦潮の範囲より少し外側まで続ける）
			if (currentRadius >= WHIRLPOOL_RADIUS * 1.5f)
			{
				info.isAffected = false;
				info.isPushing = false;
				info.target->GetStateMachine()->SetIsInWhirlpool(false);
				return;
			}

			// 半径を広げて渦巻き移動
			const float newRadius = currentRadius + PUSH_SPEED * deltaTime;
			UpdateSpiral(info, newRadius, deltaTime);
		}


		void WhirlpoolPowerSytem::UpdateSpiral(WhirlpoolPowerInfo& info, float newRadius, float deltaTime)
		{
			const Vector3& whirlpoolPos = m_owner->GetTransform().m_position;

			// 角度を回転させる（渦巻き）
			info.angle += ROTATE_SPEED * deltaTime;

			// 極座標 → デカルト座標 でXZ位置を更新
			Vector3 pos = info.target->GetTransform().m_position;
			pos.x = whirlpoolPos.x + newRadius * cosf(info.angle);
			pos.z = whirlpoolPos.z + newRadius * sinf(info.angle);
			// Y座標は変化させない

			const Vector3 prevPos = info.target->GetCharacterController()->Execute(pos, deltaTime);
			info.target->GetStateMachine()->SetPosition(prevPos);
		}
	}
}
