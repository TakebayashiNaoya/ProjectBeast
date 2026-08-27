/**
 * @file Enemy.cpp
 * @brief エネミークラス
 */
#include "stdafx.h"
#include "Enemy.h"
#include "EnemyStateMachine.h"
#include "EnemyStatus.h"
#include "Physics/Physics.h"
#include "Source/Actor/Stage/StageSystem.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 地形法線に沿わせる補間速度 */
			constexpr float GROUND_TILT_SLERP_SPEED = 10.0f;
			/** 胴体を傾ける角度の上限。
			 *  接地判定はカプセル基準なので、真下レイの法線のほうが急な角度を返すことがある
			 *  （崖の肩にカプセルが乗っている場合など）。そのまま適用すると体が倒れて見えるため、
			 *  ここでクランプする。 */
			constexpr float GROUND_TILT_MAX_ANGLE = 1.5533f; // 約70度
			/** groundNormalがほぼ真上と同じ向きとみなす内積のしきい値（IKMath.hのPARALLEL_DOT_THRESHOLDと同じ考え方） */
			constexpr float GROUND_TILT_PARALLEL_DOT_THRESHOLD = 0.9999f;



			AnimationData ANIMATION_DATA[] =
			{
				{ "Assets/animData/bear/idle.tka", true },
				{ "Assets/animData/bear/idle_UnderWater.tka", true },
				{ "Assets/animData/bear/walk.tka", true },
				{ "Assets/animData/bear/attack.tka", false },
				{ "Assets/animData/bear/attack_UnderWater.tka", false },
				{ "Assets/animData/bear/backWalk.tka", true },
				{ "Assets/animData/bear/run.tka", true },
				{ "Assets/animData/bear/swim.tka", true },
				{ "Assets/animData/bear/buff.tka", false },
				{ "Assets/animData/bear/damage.tka", true },
				{ "Assets/animData/bear/eat.tka", true },
				{ "Assets/animData/bear/stun.tka", true },
				{ "Assets/animData/bear/sleep.tka", true },

			};


			ModelData ENEMY_MODEL_DATA =
			{
				"Assets/modelData/whiteBear/WhiteBear.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};

			/** 輪郭線の太さ（スクリーン幅比の法線押し出し量。約1.5px相当） */
			constexpr float OUTLINE_WIDTH = 0.0018f;
			/** 輪郭線の色。雪に対して沈む濃い青灰 */
			const Vector4 OUTLINE_COLOR(0.08f, 0.10f, 0.16f, 1.0f);

		}


		Enemy::Enemy()
		{
			Init(ENEMY_MODEL_DATA);

			m_stateMachine = std::make_unique<EnemyStateMachine>(this);
			m_status = std::make_unique<EnemyStatus>();
			m_status->Setup();

			m_stateMachine->Setup(this);
			m_characterStateMachine = m_stateMachine.get();
		}


		void Enemy::Start()
		{
			CharacterBase::Start();
		}


		void Enemy::Update()
		{
			m_stateMachine->Update();

			CharacterBase::Update();

			if (m_modelReady && !m_legIKInited)
			{
				/** モデルロード完了後の一度きりの初期化。
				 *  白い体が雪原に溶け込まないよう、濃い青灰の太めの輪郭線で縁取る */
				GetModelRender().SetOutlineParam(OUTLINE_WIDTH, OUTLINE_COLOR);
				GetModelRender().EnableOutline();

				m_legIKInited = InitLegIK();
			}

			if (m_legIKInited)
			{
				// UpdateGroundTilt()は前フレームに求まったルート下げ量を使ってスケルトンを組み直す。
				// そのあとでIKを解き、次フレーム用の下げ量を更新する順番になっている。
				UpdateGroundTilt();

				TerrainObject* terrain = StageSystem::GetInstance()->GetTerrain();

				m_legIK.SetTerrain(terrain);
				// 急斜面の滑落中・ジャンプ中・遊泳中はCharacterControllerが接地扱いしないので、
				// その間はIKを切ってアニメーションのポーズに戻す
				m_legIK.SetGrounded(m_characterController.IsOnGround());
				// 睡眠など、足を地面に貼り付けると姿勢が崩れるアニメーションの間は脚IKを切る
				m_legIK.SetEnable(!m_stateMachine->IsLegIKSuppressed());
				// ルート下げ量は「カプセルが止まった高さ」と「真下レイが拾った実際の地面の高さ」の差。
				// 法線も渡して、その角度で浮いていて当然の量を上限に使う。
				m_legIK.SetCharacterPosition(m_transform.m_position);
				m_legIK.SetGroundInfo(
					m_characterController.IsGroundInfoValid(),
					m_characterController.GetGroundHeight(),
					m_characterController.GetGroundNormal());

				// CharacterBase::m_skeletonは実際の描画には使われていない孤立コピー。
				// 実際にスキニングに使われているのはm_modelRenderが内部で持つスケルトンなので、
				// そちらを取得してIKを解く。
				Skeleton* skeleton = m_modelRender.GetSkeleton();
				if (skeleton)
				{
					m_legIK.Update(skeleton, g_gameTime->GetFrameDeltaTime());
				}
			}
		}


		void Enemy::UpdateGroundTilt()
		{
			if (!m_modelReady) return;

			Vector3 groundNormal = Vector3::Up;
			const bool isSwimming = m_stateMachine && m_stateMachine->IsSwim();

			if (!isSwimming && m_characterController.IsOnGround() && m_characterController.IsGroundInfoValid())
			{
				const Vector3& hitNormal = m_characterController.GetGroundNormal();
				if (hitNormal.LengthSq() > FLT_EPSILON)
				{
					groundNormal = hitNormal;
				}
			}

			/** 進行方向由来のY軸回転（水平の向き）はアニメーション側の回転をそのまま使う */
			Quaternion yRot = m_transform.m_rotation;

			float dotUpNormal = Vector3::Up.Dot(groundNormal);
			dotUpNormal = max(-1.0f, min(1.0f, dotUpNormal)); // acosに渡す前のクランプ（NaN対策）
			float tiltAngle = acosf(dotUpNormal);


			Quaternion tiltRot = Quaternion::Identity;
			if (dotUpNormal < GROUND_TILT_PARALLEL_DOT_THRESHOLD)
			{
				tiltRot.SetRotation(Vector3::Up, groundNormal);

				if (tiltAngle > GROUND_TILT_MAX_ANGLE && tiltAngle > FLT_EPSILON)
				{
					// tiltRot（Up→groundNormalの全回転）を、上限角度の分だけに縮小する
					float clampFactor = GROUND_TILT_MAX_ANGLE / tiltAngle;
					Quaternion clampedTiltRot;
					clampedTiltRot.Slerp(clampFactor, Quaternion::Identity, tiltRot);
					tiltRot = clampedTiltRot;
				}
			}

			// クォータニオンの二重被覆対策（m_groundTiltRotationとtiltRotはどちらも「傾きのみ」なので比較可能）
			// ※x/y/z/wというメンバー名は仮定です。Quaternion.hの実際の
			//   メンバー変数名（または既存のDot()相当のメソッド）に合わせて調整してください。
			float dotRot = m_groundTiltRotation.x * tiltRot.x
				+ m_groundTiltRotation.y * tiltRot.y
				+ m_groundTiltRotation.z * tiltRot.z
				+ m_groundTiltRotation.w * tiltRot.w;

			if (dotRot < 0.0f)
			{
				tiltRot.x = -tiltRot.x;
				tiltRot.y = -tiltRot.y;
				tiltRot.z = -tiltRot.z;
				tiltRot.w = -tiltRot.w;
			}

			/** 傾きだけを補間して急激な変化を抑える */
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const float slerpFactor = min(1.0f, GROUND_TILT_SLERP_SPEED * deltaTime);
			m_groundTiltRotation.Slerp(slerpFactor, m_groundTiltRotation, tiltRot);

			Quaternion finalRotation;
			finalRotation.Multiply(yRot, m_groundTiltRotation);

			Vector3 rowX = Vector3::AxisX; finalRotation.Apply(rowX); rowX = rowX * m_transform.m_scale.x;
			Vector3 rowY = Vector3::AxisY; finalRotation.Apply(rowY); rowY = rowY * m_transform.m_scale.y;
			Vector3 rowZ = Vector3::AxisZ; finalRotation.Apply(rowZ); rowZ = rowZ * m_transform.m_scale.z;

			Matrix trs;
			trs.v[0].Set(rowX.x, rowX.y, rowX.z, 0.0f);
			trs.v[1].Set(rowY.x, rowY.y, rowY.z, 0.0f);
			trs.v[2].Set(rowZ.x, rowZ.y, rowZ.z, 0.0f);
			trs.v[3].Set(m_transform.m_position.x, m_transform.m_position.y - m_legIK.GetRootDropOffset(), m_transform.m_position.z, 1.0f);

			Matrix mBias = Matrix::Identity;
			if (m_upAxis == EnModelUpAxis::enModelUpAxisZ)
			{
				mBias.MakeRotationX(Math::PI * -0.5f);
			}

			Matrix tiltedWorld = mBias * trs;

			Skeleton* skeleton = m_modelRender.GetSkeleton();
			if (skeleton)
			{
				skeleton->Update(tiltedWorld);
			}
		}


		bool Enemy::InitLegIK()
		{
			// IKで動かす対象は、実際の描画・スキニングに使われている
			// m_modelRender側のスケルトンでなければならない。
			Skeleton* skeleton = m_modelRender.GetSkeleton();
			if (!skeleton)
			{
				return false;
			}

			m_legIK.Clear();

			bool ok = true;

			ok &= m_legIK.AddLegByBoneNames(
				skeleton, L"Hip_L", L"Knee_L", L"Ankle_L",
				Vector3(0.0f, -1.0f, 1.0f), 1.0f, 0.0f,
				{ L"Toes1_L", L"MiddleToe1_L", L"MiddleToe2_L" });

			ok &= m_legIK.AddLegByBoneNames(
				skeleton, L"Hip_R", L"Knee_R", L"Ankle_R",
				Vector3(0.0f, -1.0f, 1.0f), 1.0f, 0.0f,
				{ L"Toes1_R", L"MiddleToe1_R", L"MiddleToe2_R" });

			ok &= m_legIK.AddLegByBoneNames(
				skeleton, L"Shoulder_L", L"Elbow_L", L"Wrist_L",
				Vector3(0.0f, -1.0f, -1.0f), 1.0f, 0.0f,
				{ L"Fingers1_L", L"MiddleFinger1_L", L"MiddleFinger2_L" },
				{ L"ShoulderPart1_L" },
				{ L"ElbowPart1_L", L"ElbowPart2_L" });

			ok &= m_legIK.AddLegByBoneNames(
				skeleton, L"Shoulder_R", L"Elbow_R", L"Wrist_R",
				Vector3(0.0f, -1.0f, -1.0f), 1.0f, 0.0f,
				{ L"Fingers1_R", L"MiddleFinger1_R", L"MiddleFinger2_R" },
				{ L"ShoulderPart1_R" },
				{ L"ElbowPart1_R", L"ElbowPart2_R" });

			// ボーン名のtypoやリグ変更にすぐ気づけるよう、開発中は止める
			// （NDEBUGビルド＝リリース版ではassertは無効化されるので本番影響なし）
			assert(ok && "InitLegIK: ボーンが見つからず脚IKの登録に失敗した箇所があります");

			if (!ok)
			{
				m_legIK.Clear();
				return false;
			}

			// 斜面での想定浮き量 radius*(1/cosθ-1) を計算するのに使う。
			// これがルート下げ量の上限になり、急角度ほど自動的に大きくなる。
			const auto* status = GetStatus<CharacterStatus>();
			if (status)
			{
				m_legIK.SetCapsuleRadius(status->GetRadius());
			}

			// 足元へ飛ばすレイが自分のカプセルに当たらないよう、除外対象として渡しておく
			m_legIK.SetSelfCollisionObject(m_characterController.GetRigidBody()->GetBody());

			//m_groundTiltRotation = m_transform.m_rotation;
			return true;
		}


		bool Enemy::ShouldSuppressFootprint() const
		{
			return m_stateMachine->IsSwim();
		}


		void Enemy::Render(RenderContext& rc)
		{
			CharacterBase::Render(rc);
		}
	}
}
