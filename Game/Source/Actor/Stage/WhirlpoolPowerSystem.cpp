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

			UpdateWhirlpoolInfo();

			for (auto& info : m_whirlpoolPowerInfos)
			{
				if (!info.isAffected) continue;

				// 子ペンギンが渦潮の影響を受けると渦潮が消えるまで変わらない
				info.target->GetStateMachine()->SetIsInWhirlpool(true);

				if (info.isPushing)
				{
					// 押し出しフェーズ
					UpdatePush(info, deltaTime);
				}
				else
				{
					// 引き寄せフェーズ
					UpdateAttract(info, deltaTime);
				}
			}
		}


		void WhirlpoolPowerSytem::Render(RenderContext& rc)
		{}


		WhirlpoolPowerSytem::WhirlpoolPowerSytem(Whirlpool* ownerWhirlpool)
			: m_ownerWhirlpool(ownerWhirlpool)
			, m_childPenguinManager(nullptr)
			, m_childPenguinNum(0)
		{
			m_childPenguinManager = ChildPenguinManager::GetInstance();
			m_childPenguinNum = m_childPenguinManager->GetChildPenguinNum();

			// 引き寄せ、押し出しの情報を初期化
			for (auto& cp : m_childPenguinManager->GetChildPenguin())
			{
				WhirlpoolPowerInfo newInfo;
				// 渦潮から子ペンギンへのベクトル
				newInfo.toTargetVector = cp->GetTransform().m_position - m_ownerWhirlpool->GetTransform().m_position;
				newInfo.target = cp;
				newInfo.isAffected = false;
				newInfo.isPushing = false;
				// 初期角度をXZ平面で計算
				newInfo.angle = atan2f(newInfo.toTargetVector.z, newInfo.toTargetVector.x);

				m_whirlpoolPowerInfos.push_back(newInfo);
			}
		}


		WhirlpoolPowerSytem::~WhirlpoolPowerSytem()
		{}


		void WhirlpoolPowerSytem::UpdateWhirlpoolInfo()
		{
			const Vector3& whirlpoolPos = m_ownerWhirlpool->GetTransform().m_position;

			for (auto& info : m_whirlpoolPowerInfos)
			{
				if (info.target == nullptr) continue;

				// 渦潮から子ペンギンへのベクトルを更新
				info.toTargetVector = info.target->GetTransform().m_position - whirlpoolPos;

				// XZ平面での距離を計算（Y成分を0にして Length() を利用）
				const Vector3 toTargetXZ = Vector3(info.toTargetVector.x, 0.0f, info.toTargetVector.z);
				const float distXZ = toTargetXZ.Length();

				// 渦潮の範囲内に入ったら影響フラグを立てる
				if (!info.isAffected && distXZ <= WHIRLPOOL_RADIUS)
				{
					info.isAffected = true;
					info.isPushing = false;
					// 範囲に入った瞬間の角度を記録
					info.angle = atan2f(info.toTargetVector.z, info.toTargetVector.x);
				}
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
				return;
			}

			// 半径を広げて渦巻き移動
			const float newRadius = currentRadius + PUSH_SPEED * deltaTime;
			UpdateSpiral(info, newRadius, deltaTime);
		}


		void WhirlpoolPowerSytem::UpdateSpiral(WhirlpoolPowerInfo& info, float newRadius, float deltaTime)
		{
			const Vector3& whirlpoolPos = m_ownerWhirlpool->GetTransform().m_position;

			// 角度を回転させる（渦巻き）
			info.angle += ROTATE_SPEED * deltaTime;

			// 極座標 → デカルト座標 でXZ位置を更新
			Vector3 pos = info.target->GetTransform().m_position;
			pos.x = whirlpoolPos.x + newRadius * cosf(info.angle);
			pos.z = whirlpoolPos.z + newRadius * sinf(info.angle);
			// Y座標は変化させない

			info.target->GetStateMachine()->SetPosition(pos);
		}
	}
}
