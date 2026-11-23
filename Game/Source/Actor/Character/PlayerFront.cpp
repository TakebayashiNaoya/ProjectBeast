/**
 * @file PlayerFront.cpp
 * @brief プレイヤー（親ペンギン）の「正面」の計算と、敵の回り込みの計算
 */
#include "stdafx.h"
#include "PlayerFront.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 度 → ラジアン */
			constexpr float DEG_TO_RAD = Math::PI / 180.0f;
			/**
			 * @brief 正面との角度差がこれ以上（ほぼ真後ろ）のときは、回る向きを前回のまま保つ（度）
			 * @details 真後ろでは左右どちらに回っても同じ距離なので、毎フレーム選び直すと向きが入れ替わって敵が揺れる
			 */
			constexpr float KEEP_ORBIT_SIGN_DEG = 160.0f;
			/** ベクトルの長さが0とみなす値 */
			constexpr float ZERO_LENGTH_SQ = 0.0001f;


			/** @brief XZ平面での距離 */
			float DistanceXZ(const Vector3& a, const Vector3& b)
			{
				Vector3 diff = a - b;
				diff.y = 0.0f;
				return diff.Length();
			}


			/** @brief 角度を(-PI, PI]に収める */
			float WrapAngle(float angle)
			{
				while (angle > Math::PI) angle -= Math::PI2;
				while (angle <= -Math::PI) angle += Math::PI2;
				return angle;
			}


			/** @brief SelfTest用: 誤差を許して等しいか */
			bool IsNear(float a, float b, float tolerance = 0.01f)
			{
				return fabsf(a - b) <= tolerance;
			}
		}


		Vector3 PlayerFront::CalcFrontDirXZ(const Vector3& cameraForward, const Quaternion& parentRotation)
		{
			Vector3 front(cameraForward.x, 0.0f, cameraForward.z);
			if (front.LengthSq() > ZERO_LENGTH_SQ)
			{
				front.Normalize();
				return front;
			}

			// カメラが真上・真下を向いていてXZ成分が潰れたときは、親の向きを使う
			front = Vector3::Front;
			parentRotation.Apply(front);
			front.y = 0.0f;
			if (front.LengthSq() <= ZERO_LENGTH_SQ)
			{
				return Vector3::Front;
			}
			front.Normalize();
			return front;
		}


		bool PlayerFront::IsInFrontXZ(const Vector3& basePos, const Vector3& frontDir, const Vector3& targetPos, float cosThreshold)
		{
			Vector3 toTarget = targetPos - basePos;
			toTarget.y = 0.0f;
			// 起点と同じ位置なら正面扱いにする（FrontCheckerと同じ）
			if (toTarget.LengthSq() <= ZERO_LENGTH_SQ)
			{
				return true;
			}
			toTarget.Normalize();

			const Vector3 front(frontDir.x, 0.0f, frontDir.z);
			return front.Dot(toTarget) >= cosThreshold;
		}


		PlayerFront::ApproachState PlayerFront::StepApproach(const ApproachState& state, const ApproachInput& input, const ApproachParams& params)
		{
			ApproachState next = state;
			next.endReason = EnApproachEndReason::None;

			// 1. 回り込みを終えたら、以後は直行のまま
			if (next.phase == EnApproachPhase::Done)
			{
				return next;
			}

			const float targetToParent = DistanceXZ(input.targetPosition, input.parentPosition);

			if (next.phase == EnApproachPhase::Idle)
			{
				// 2. 群れから外れた子には「正面」が定義できないので、回り込まずに直行する
				if (targetToParent > params.directDistance)
				{
					return next;
				}
				// 敵が子の近くにいても回り込ませる（背後から襲わせないため）
				next.phase = EnApproachPhase::Flanking;
				next.flankTimer = 0.0f;
				next.orbitSign = 0.0f;
			}
			else
			{
				next.flankTimer += input.deltaTime;
			}

			// 3. 回り込み中の打ち切り判定
			if (targetToParent > params.releaseDistance)
			{
				next.phase = EnApproachPhase::Idle;
				next.endReason = EnApproachEndReason::Far;
				return next;
			}
			if (next.flankTimer >= params.timeLimit)
			{
				next.phase = EnApproachPhase::Done;
				next.endReason = EnApproachEndReason::Timeout;
				return next;
			}
			const float frontCos = cosf(params.frontHalfAngleDeg * DEG_TO_RAD);
			if (IsInFrontXZ(input.parentPosition, input.frontDirection, input.enemyPosition, frontCos))
			{
				next.phase = EnApproachPhase::Done;
				next.endReason = EnApproachEndReason::Front;
				return next;
			}

			// 4. 親から見た敵の方位角を、正面寄りにleadAngleだけ回した点へ向かう
			Vector3 fromParent = input.enemyPosition - input.parentPosition;
			fromParent.y = 0.0f;
			const float enemyAngle = atan2f(fromParent.x, fromParent.z);
			const float frontAngle = atan2f(input.frontDirection.x, input.frontDirection.z);
			const float delta = WrapAngle(frontAngle - enemyAngle);

			if (next.orbitSign == 0.0f || fabsf(delta) < KEEP_ORBIT_SIGN_DEG * DEG_TO_RAD)
			{
				next.orbitSign = (delta >= 0.0f) ? 1.0f : -1.0f;
			}

			const float leadAngle = params.leadAngleDeg * DEG_TO_RAD;
			const float step = min(fabsf(delta), leadAngle) * next.orbitSign;
			const float steerAngle = enemyAngle + step;

			// 半径は正面に近いほど子の距離まで縮める（円ではなく渦巻きで正面から迫る）
			const float frontHalfAngle = params.frontHalfAngleDeg * DEG_TO_RAD;
			const float remainAngle = fabsf(delta) - fabsf(step);
			const float spiral = max(0.0f, min(1.0f, (remainAngle - frontHalfAngle) / (Math::PI - frontHalfAngle)));
			const float innerRadius = min(targetToParent, params.orbitRadius);
			// 先の点を純追尾すると軌道の内側へ落ちるので、その分だけ遠くに置く
			const float steerDistance = (innerRadius + (params.orbitRadius - innerRadius) * spiral) / cosf(leadAngle);

			next.steerPoint = Vector3(
				input.parentPosition.x + sinf(steerAngle) * steerDistance,
				input.parentPosition.y,
				input.parentPosition.z + cosf(steerAngle) * steerDistance);
			return next;
		}


		bool PlayerFront::GetFront(Vector3& outParentPos, Vector3& outFrontDir)
		{
			ChildPenguinManager* cpManager = ChildPenguinManager::GetInstance();
			if (cpManager == nullptr) return false;
			const DaddyPenguin* daddy = cpManager->GetDaddyPenguin();
			if (daddy == nullptr) return false;

			const auto& transform = daddy->GetTransform();
			outParentPos = transform.m_position;
			outFrontDir = CalcFrontDirXZ(CameraSystem::Get().GetMainCamera().GetForward(), transform.m_rotation);
			return true;
		}


		bool PlayerFront::IsVisible(const Vector3& center, float radius)
		{
			// RenderingEngine::GetFrustum()はRenderingEngine::Executeの先頭で更新されるので、シーン更新中は前フレームの値になる。
			// ここでは今のメインカメラから組み直す（縮小率1.0固定なので、デバッグ用の縮小設定にも左右されない）
			auto& camera = CameraSystem::Get().GetMainCamera();
			Matrix viewProjMatrix;
			viewProjMatrix.Multiply(camera.GetViewMatrix(), camera.GetProjectionMatrix());

			nsBeastEngine::Frustum frustum;
			frustum.Update(viewProjMatrix, 1.0f);
			return frustum.IsIntersectSphere(center, radius);
		}


		bool PlayerFront::IsPointVisible(const Vector3& point)
		{
			auto& camera = CameraSystem::Get().GetMainCamera();
			Matrix viewProjMatrix;
			viewProjMatrix.Multiply(camera.GetViewMatrix(), camera.GetProjectionMatrix());

			nsBeastEngine::Frustum frustum;
			frustum.Update(viewProjMatrix, 1.0f);
			return frustum.IsPointInside(point);
		}


		bool PlayerFront::SelfTest()
		{
			bool passed = true;
			auto check = [&passed](bool condition, const char* message)
			{
				K2_ASSERT(condition, message);
				passed = passed && condition;
			};

			// --- CalcFrontDirXZ ---
			{
				const Vector3 front = CalcFrontDirXZ(Vector3(0.0f, -0.5f, 1.0f), Quaternion::Identity);
				check(IsNear(front.x, 0.0f) && IsNear(front.y, 0.0f) && IsNear(front.z, 1.0f), "PlayerFront: カメラ前方の XZ 成分が正面になっていません");

				// 真下を向いたカメラでは親の向きを使う
				Quaternion rotation;
				rotation.SetRotationDegY(90.0f);
				Vector3 expected = Vector3::Front;
				rotation.Apply(expected);
				const Vector3 fallback = CalcFrontDirXZ(Vector3(0.0f, -1.0f, 0.0f), rotation);
				check(IsNear(fallback.x, expected.x) && IsNear(fallback.z, expected.z) && IsNear(fallback.Length(), 1.0f), "PlayerFront: カメラが真下のとき親の向きになっていません");
			}

			// --- IsInFrontXZ ---
			{
				const float cos40 = cosf(40.0f * DEG_TO_RAD);
				check(IsInFrontXZ(Vector3::Zero, Vector3::Front, Vector3(0.0f, 0.0f, 100.0f), cos40), "PlayerFront: 正面の点が正面と判定されません");
				check(!IsInFrontXZ(Vector3::Zero, Vector3::Front, Vector3(0.0f, 0.0f, -100.0f), cos40), "PlayerFront: 背後の点が正面と判定されました");
				check(!IsInFrontXZ(Vector3::Zero, Vector3::Front, Vector3(100.0f, 0.0f, 100.0f), cos40), "PlayerFront: 45度の点が40度コーン内と判定されました");
				check(IsInFrontXZ(Vector3::Zero, Vector3::Front, Vector3(100.0f, 50.0f, 100.0f), 0.0f), "PlayerFront: 前方の半平面の点が前方と判定されません（高さは無視する）");
			}

			// --- StepApproach ---
			{
				ApproachParams params;
				params.orbitRadius = 400.0f;
				params.timeLimit = 5.0f;

				ApproachInput input;
				input.parentPosition = Vector3::Zero;
				input.frontDirection = Vector3::Front;
				input.targetPosition = Vector3(100.0f, 0.0f, 0.0f);
				input.deltaTime = 1.0f / 60.0f;

				// 群れから外れた子（親から450より遠い）は直行
				{
					ApproachInput farTarget = input;
					farTarget.targetPosition = Vector3(500.0f, 0.0f, 0.0f);
					farTarget.enemyPosition = Vector3(0.0f, 0.0f, -600.0f);
					const ApproachState next = StepApproach(ApproachState(), farTarget, params);
					check(next.phase == EnApproachPhase::Idle, "PlayerFront: 群れから外れた子に回り込んでいます");
				}

				// 最初から正面にいる敵は直行（IdleからそのままDone）
				{
					ApproachInput front = input;
					front.enemyPosition = Vector3(0.0f, 0.0f, 600.0f);
					const ApproachState next = StepApproach(ApproachState(), front, params);
					check(next.phase == EnApproachPhase::Done && next.endReason == EnApproachEndReason::Front, "PlayerFront: 正面にいる敵が直行になっていません");
				}

				// 真後ろの敵は回り込む。向かう点は軌道半径と子の距離の間にあり、正面から135度の方位
				ApproachInput behind = input;
				behind.enemyPosition = Vector3(0.0f, 0.0f, -600.0f);
				const ApproachState flanking = StepApproach(ApproachState(), behind, params);
				const float leadCos = cosf(params.leadAngleDeg * DEG_TO_RAD);
				Vector3 behindSteer = flanking.steerPoint;
				behindSteer.y = 0.0f;
				{
					check(flanking.phase == EnApproachPhase::Flanking, "PlayerFront: 背後の敵が回り込みに入りません");
					check(behindSteer.Length() < params.orbitRadius / leadCos && behindSteer.Length() > 100.0f / leadCos, "PlayerFront: 回り込みの点の距離が違います");
					Vector3 dir = behindSteer;
					dir.Normalize();
					check(IsNear(dir.Dot(Vector3::Front), cosf(135.0f * DEG_TO_RAD)), "PlayerFront: 回り込みの点が45度だけ正面寄りになっていません");
				}

				// 真横の敵は正面側へ回り、真後ろより内側を狙う
				{
					ApproachInput side = input;
					side.enemyPosition = Vector3(600.0f, 0.0f, 0.0f);
					const ApproachState next = StepApproach(ApproachState(), side, params);
					Vector3 sideSteer = next.steerPoint;
					sideSteer.y = 0.0f;
					check(next.phase == EnApproachPhase::Flanking && next.steerPoint.x > 0.0f && next.steerPoint.z > 0.0f, "PlayerFront: 真横の敵が正面側へ回っていません");
					check(sideSteer.Length() < behindSteer.Length(), "PlayerFront: 正面に近いほど半径が縮んでいません");
				}

				// 回り込み中に子が親から900より遠くなったら直行へ戻す
				{
					ApproachInput released = behind;
					released.targetPosition = Vector3(1000.0f, 0.0f, 0.0f);
					const ApproachState next = StepApproach(flanking, released, params);
					check(next.phase == EnApproachPhase::Idle && next.endReason == EnApproachEndReason::Far, "PlayerFront: 子が群れから離れても回り込みを続けています");
				}

				// 時間切れで直行
				{
					ApproachState almost = flanking;
					almost.flankTimer = params.timeLimit - input.deltaTime * 0.5f;
					const ApproachState next = StepApproach(almost, behind, params);
					check(next.phase == EnApproachPhase::Done && next.endReason == EnApproachEndReason::Timeout, "PlayerFront: 時間切れで直行になりません");
				}

				// DoneはDoneのまま（背後に戻っても回り込み直さない）
				{
					ApproachState done;
					done.phase = EnApproachPhase::Done;
					const ApproachState next = StepApproach(done, behind, params);
					check(next.phase == EnApproachPhase::Done && next.endReason == EnApproachEndReason::None, "PlayerFront: 回り込み後に回り込み直しています");
				}

				// 真後ろ付近では回る向きを保つ（左右が入れ替わらない）
				{
					ApproachInput slightlyLeft = behind;
					slightlyLeft.enemyPosition = Vector3(-1.0f, 0.0f, -600.0f);
					ApproachInput slightlyRight = behind;
					slightlyRight.enemyPosition = Vector3(1.0f, 0.0f, -600.0f);
					const ApproachState first = StepApproach(ApproachState(), slightlyLeft, params);
					const ApproachState second = StepApproach(first, slightlyRight, params);
					check(first.orbitSign != 0.0f && first.orbitSign == second.orbitSign, "PlayerFront: 真後ろで回る向きが入れ替わりました");
				}
			}

			return passed;
		}
	}
}