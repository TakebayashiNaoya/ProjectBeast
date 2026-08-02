/**
 * @file Enemy.cpp
 * @brief エネミークラス
 * @author 立山
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
			/** 傾きの最大角度（ラジアン）。急な形状で過度に傾いて埋まって見えるのを防ぐための上限（約35度） */
			constexpr float GROUND_TILT_MAX_ANGLE = 0.6109f;
			/** レイの発射点を、キャラクター座標からどれだけ上にずらすか（地形より確実に高くするため） */
			constexpr float GROUND_TILT_RAY_START_UP_OFFSET = 500.0f;
			/** 真下レイの射程距離（発射点を上げた分、長めに取っておく） */
			constexpr float GROUND_TILT_RAY_LENGTH = 1000.0f;
			/** 地面法線を計算する際に、キャラ位置からXZ方向にサンプリングする距離 */
			constexpr float GROUND_TILT_NORMAL_SAMPLE_OFFSET = 40.0f;
			/** キャラY座標とGetHeightAtの結果の差がこれを超えたら、
			 *  「ハイトマップ地形の上に立っていない」とみなして傾き計算をスキップする
			 *  （台座オブジェクトの上に立っている場合や、地形との不整合を弾くため） */
			constexpr float GROUND_TILT_MAX_HEIGHT_DIFF = 8.0f;


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
				InitLegIK();
				m_legIKInited = true;
			}

			if (m_legIKInited)
			{
				UpdateGroundTilt();

				TerrainObject* terrain = StageSystem::GetInstance()->GetTerrain();

				m_legIK.SetTerrain(terrain);

				// CharacterBase::m_skeletonは実際の描画には使われていない孤立コピー。
				// 実際にスキニングに使われているのはm_modelRenderが内部で持つスケルトンなので、
				// そちらを取得してIKを解く。
				Skeleton* skeleton = m_modelRender.GetSkeleton();
				if (skeleton)
				{
					m_legIK.Update(skeleton);
				}
			}
		}


		void Enemy::UpdateGroundTilt()
		{
			if (!m_modelReady) return;

			/** 真下にレイを飛ばして地形法線を取得する（PenguinBase::UpdateSlideTiltと同じ） */
			const Vector3& pos = m_transform.m_position;

			TerrainObject* terrain = StageSystem::GetInstance()->GetTerrain();
			float terrainHeightAtPos = pos.y;
			if (terrain)
			{
				terrainHeightAtPos = terrain->GetHeightAt(pos);
				char buf[256];
				//sprintf_s(buf, "[Enemy#%d] EnemyPosY=%.1f TerrainHeight=%.1f Diff=%.1f\n",
				//	m_logId, pos.y, terrainHeightAtPos, pos.y - terrainHeightAtPos);
				//OutputDebugStringA(buf);
			}

			// ★修正: 泳いでいる間はキャラが実際に地形の上に立っていないため、
			//         近傍の地形サンプリング（水中の海底地形を拾ってしまう等）
			//         は意味を持たない。泳いでいる間は傾き計算そのものをスキップし、
			//         水平（Up）のまま扱う。
			Vector3 groundNormal = Vector3::Up;
			bool isSwimming = m_stateMachine && m_stateMachine->IsSwim();
			bool isOnMappedGround = terrain && (fabsf(pos.y - terrainHeightAtPos) <= GROUND_TILT_MAX_HEIGHT_DIFF);

			if (terrain && !isSwimming && isOnMappedGround)
			{
				const float sampleOffset = GROUND_TILT_NORMAL_SAMPLE_OFFSET;

				// ★修正: 1点でも極端に高さが違うサンプル（近くの装飾物、崖、
				//         水中地形などを拾ってしまった場合）が混ざると、法線の
				//         "方向"自体が大きく歪んでしまう。最終的にtiltAngleを
				//         GROUND_TILT_MAX_ANGLEでクランプしていても、法線の向き
				//         そのものが歪んでいては意味がないため、サンプリングの
				//         時点で「中心の高さから見て、最大許容角度で届く範囲」に
				//         あらかじめクランプしておく。
				const float maxSampleDelta = sampleOffset * tanf(GROUND_TILT_MAX_ANGLE);
				auto ClampHeight = [terrainHeightAtPos, maxSampleDelta](float h)
					{
						float minH = terrainHeightAtPos - maxSampleDelta;
						float maxH = terrainHeightAtPos + maxSampleDelta;
						if (h < minH) h = minH;
						if (h > maxH) h = maxH;
						return h;
					};

				// ※GetHeightAtはY成分を無視するので、第2引数は0.0fでOK
				float hL = ClampHeight(terrain->GetHeightAt(Vector3(pos.x - sampleOffset, 0.0f, pos.z)));
				float hR = ClampHeight(terrain->GetHeightAt(Vector3(pos.x + sampleOffset, 0.0f, pos.z)));
				float hD = ClampHeight(terrain->GetHeightAt(Vector3(pos.x, 0.0f, pos.z - sampleOffset)));
				float hU = ClampHeight(terrain->GetHeightAt(Vector3(pos.x, 0.0f, pos.z + sampleOffset)));

				Vector3 tangentX(2.0f * sampleOffset, hR - hL, 0.0f);
				Vector3 tangentZ(0.0f, hU - hD, 2.0f * sampleOffset);

				// tangentZ × tangentX で、平坦なら(0,1,0)になる向きの外積を取る
				Vector3 normal;
				normal.x = tangentZ.y * tangentX.z - tangentZ.z * tangentX.y;
				normal.y = tangentZ.z * tangentX.x - tangentZ.x * tangentX.z;
				normal.z = tangentZ.x * tangentX.y - tangentZ.y * tangentX.x;

				if (normal.LengthSq() > FLT_EPSILON)
				{
					normal.Normalize();
					groundNormal = normal;
				}
			}

			/** 進行方向由来のY軸回転（水平の向き）はアニメーション側の回転をそのまま使う */
			Quaternion yRot = m_transform.m_rotation;

			{
				Vector3 yRotUp = Vector3::Up;
				yRot.Apply(yRotUp);
				float yRotDot = max(-1.0f, min(1.0f, Vector3::Up.Dot(yRotUp)));
				float yRotTiltDeg = acosf(yRotDot) * 180.0f / 3.14159265f;

				char buf[256];
				//sprintf_s(buf, "[Enemy#%d] yRot alone tilts Up by %.2f deg (yRotUp=(%.3f,%.3f,%.3f))\n",
				//	m_logId, yRotTiltDeg, yRotUp.x, yRotUp.y, yRotUp.z);
				//OutputDebugStringA(buf);
			}

			float dotUpNormal = Vector3::Up.Dot(groundNormal);
			dotUpNormal = max(-1.0f, min(1.0f, dotUpNormal)); // acosに渡す前のクランプ（NaN対策）
			float tiltAngle = acosf(dotUpNormal);

			// デバッグ: 実際にどんな値になっているか確認するためのログ。
			// 原因切り分けができたら消してOK。
			{
				char buf[256];
				//sprintf_s(buf, "groundNormal=(%.3f,%.3f,%.3f) dot=%.5f tiltAngle(deg)=%.2f\n",
				//	groundNormal.x, groundNormal.y, groundNormal.z,
				//	dotUpNormal, tiltAngle * 180.0f / 3.14159265f);
				//OutputDebugStringA(buf);
			}

			// ★修正: groundNormalがVector3::Upとほぼ同じ方向（平坦な地形、または
			//         terrainがnullでUpのままの場合など）のとき、SetRotationに渡す
			//         回転軸がほぼゼロベクトルになり、正規化で不定な値（NaNや
			//         未初期化のゴミ値）を返す可能性がある。
			//         IKMath.hのRotateBasisTo/CalcMinimalRotationと同じ考え方で、
			//         「ほぼ同じ方向」のケースは事前に弾いて回転なし(Identity)として扱う。
			Quaternion tiltRot = Quaternion::Identity;
			if (dotUpNormal < 0.9999f)
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

			/** 「まず地形に合わせて傾け、次に進行方向を向く」合成 */
			Quaternion targetRotation;
			targetRotation.Multiply(yRot, tiltRot);

			// ★修正: クォータニオンは q と -q が同じ回転を表す「二重被覆」の性質を持つ。
			//         内積が負の場合、そのままSlerpすると"長い方の経路"で補間してしまい、
			//         その過程で一瞬（あるいは追いつかず持続的に）上下逆さまのような
			//         おかしな姿勢を経由してしまう。
			//         ダイブ演出のようにyRot（進行方向の回転）が短時間で大きく変化する
			//         状況ほど、この符号の食い違いが起きやすい。
			//         ※x/y/z/wというメンバー名は仮定です。Quaternion.hの実際の
			//         メンバー変数名（または既存のDot()相当のメソッド）に合わせて
			//         調整してください。
			float dotRot = m_groundTiltRotation.x * targetRotation.x
				+ m_groundTiltRotation.y * targetRotation.y
				+ m_groundTiltRotation.z * targetRotation.z
				+ m_groundTiltRotation.w * targetRotation.w;

			if (dotRot < 0.0f)
			{
				targetRotation.x = -targetRotation.x;
				targetRotation.y = -targetRotation.y;
				targetRotation.z = -targetRotation.z;
				targetRotation.w = -targetRotation.w;
			}

			// ★デバッグ: 符号反転が実際に起きているかを確認する。
			//            ダイブ演出中にdotRot<0が頻発していれば、今回の仮説が正しい。
			//{
			//	char buf[256];
			//	sprintf_s(buf, "[Enemy#%d] dotRot=%.5f (negative=flip-corrected)\n", m_logId, dotRot);
			//	OutputDebugStringA(buf);
			//}

			/** 補間して急激な回転変化を抑える */
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const float slerpFactor = min(1.0f, GROUND_TILT_SLERP_SPEED * deltaTime);
			m_groundTiltRotation.Slerp(slerpFactor, m_groundTiltRotation, targetRotation);

			{
				Vector3 finalUp = Vector3::Up;
				m_groundTiltRotation.Apply(finalUp);
				float finalDot = max(-1.0f, min(1.0f, Vector3::Up.Dot(finalUp)));
				float finalTiltDeg = acosf(finalDot) * 180.0f / 3.14159265f;

				char buf[256];
				//sprintf_s(buf, "[Enemy#%d] FINAL applied tilt = %.2f deg (finalUp=(%.3f,%.3f,%.3f)) isSwimming=%d\n",
				//	m_logId, finalTiltDeg, finalUp.x, finalUp.y, finalUp.z, isSwimming ? 1 : 0);
				//OutputDebugStringA(buf);
			}

			// ★修正: m_modelRender.Update()を呼ぶとアニメーション再生時間が
			//         CharacterBase::Update()内の分と合わせて二重に進んでしまう（倍速の原因）。
			//         Skeleton::Update()はアニメーション時間を進めず行列を掛け合わせるだけなので、
			//         こちらを直接呼び、今フレーム分すでに計算済みのローカル行列（＝アニメのポーズ）は
			//         そのままに、ワールド行列だけを「傾いたルート行列」で再計算する。
			Vector3 rowX = Vector3::AxisX; m_groundTiltRotation.Apply(rowX); rowX = rowX * m_transform.m_scale.x;
			Vector3 rowY = Vector3::AxisY; m_groundTiltRotation.Apply(rowY); rowY = rowY * m_transform.m_scale.y;
			Vector3 rowZ = Vector3::AxisZ; m_groundTiltRotation.Apply(rowZ); rowZ = rowZ * m_transform.m_scale.z;

			Matrix tiltedWorld;
			tiltedWorld.v[0].Set(rowX.x, rowX.y, rowX.z, 0.0f);
			tiltedWorld.v[1].Set(rowY.x, rowY.y, rowY.z, 0.0f);
			tiltedWorld.v[2].Set(rowZ.x, rowZ.y, rowZ.z, 0.0f);
			tiltedWorld.v[3].Set(m_transform.m_position.x, m_transform.m_position.y, m_transform.m_position.z, 1.0f);

			Skeleton* skeleton = m_modelRender.GetSkeleton();
			if (skeleton)
			{
				skeleton->Update(tiltedWorld);
			}
		}


		void Enemy::InitLegIK()
		{
			// IKで動かす対象は、実際の描画・スキニングに使われている
			// m_modelRender側のスケルトンでなければならない。
			Skeleton* skeleton = m_modelRender.GetSkeleton();
			assert(skeleton && "Enemy::InitLegIK: ModelRenderからSkeletonが取得できません。");
			if (!skeleton) return;

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

			m_groundTiltRotation = m_transform.m_rotation;

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
