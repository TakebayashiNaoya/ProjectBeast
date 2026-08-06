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
			/** ★修正: 崖のような急斜面を反映できるよう、35度→約70度まで引き上げ。
			 *  この値はサンプリングで検出できる傾きの理論上限も兼ねているため、
			 *  ここを上げないとサンプリング距離をどれだけ縮めても35度で頭打ちだった。 */
			constexpr float GROUND_TILT_MAX_ANGLE = 1.5533f; // 約70度
			/** レイの発射点を、キャラクター座標からどれだけ上にずらすか（地形より確実に高くするため） */
			constexpr float GROUND_TILT_RAY_START_UP_OFFSET = 500.0f;
			/** 真下レイの射程距離（発射点を上げた分、長めに取っておく） */
			constexpr float GROUND_TILT_RAY_LENGTH = 1000.0f;
			/** ★修正: 40だと坂の途中の平らな部分（崖の上や下）まで一緒にサンプリングしてしまい、
			 *  傾きが平均化されて実際より緩やかに見積もられていた。足元の局所的な傾きを
			 *  拾えるよう、キャラの体格（GetFootprintStanceWidth=20程度）に合わせて狭める。 */
			constexpr float GROUND_TILT_NORMAL_SAMPLE_OFFSET = 12.0f;
			/** キャラY座標とGetHeightAtの結果の差がこれを超えたら、
			 *  「ハイトマップ地形の上に立っていない」とみなして傾き計算をスキップする
			 *  （台座オブジェクトの上に立っている場合や、地形との不整合を弾くため） */
			constexpr float GROUND_TILT_MAX_HEIGHT_DIFF = 8.0f;
			/** キャラY座標とGetHeightAtの結果の差がこの範囲内なら、地形の上に立っているとみなし
 *  地形法線でしっかり傾ける */
			constexpr float GROUND_TILT_MAX_HEIGHT_DIFF_FULL = 8.0f;
			/** ★修正: この値を超えたら「地形の上に立っていない」（台座オブジェクトの上など）とみなし
			 *  傾きを完全にゼロにする。FULL～ZEROの間はDiffが大きくなるほど徐々に傾きを弱める
			 *  （旧GROUND_TILT_MAX_HEIGHT_DIFFの0/1判定だと、坂を登っている途中で接地判定が
			 *  追いついていないだけのケースまで弾いてしまい、坂で全く傾かない原因になっていた） */
			constexpr float GROUND_TILT_MAX_HEIGHT_DIFF_ZERO = 80.0f;


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

				m_legIKInited = InitLegIK();
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

			// ★修正: 0/1の完全スキップをやめ、Diffの大きさに応じて0.0～1.0のブレンド係数にする
			float groundTiltWeight = 0.0f;
			if (terrain && !isSwimming)
			{
				float diff = fabsf(pos.y - terrainHeightAtPos);
				if (diff <= GROUND_TILT_MAX_HEIGHT_DIFF_FULL)
				{
					groundTiltWeight = 1.0f;
				}
				else if (diff < GROUND_TILT_MAX_HEIGHT_DIFF_ZERO)
				{
					float t = (diff - GROUND_TILT_MAX_HEIGHT_DIFF_FULL) /
						(GROUND_TILT_MAX_HEIGHT_DIFF_ZERO - GROUND_TILT_MAX_HEIGHT_DIFF_FULL);
					groundTiltWeight = 1.0f - t; // 線形フェード
				}
				// diff >= ZERO のときは 0.0f のまま（台座等とみなし傾けない）
			}

			if (terrain && !isSwimming && groundTiltWeight > 0.0f)
			{
				const float sampleOffset = GROUND_TILT_NORMAL_SAMPLE_OFFSET;

				const float maxSampleDelta = sampleOffset * tanf(GROUND_TILT_MAX_ANGLE);
				auto ClampHeight = [terrainHeightAtPos, maxSampleDelta](float h)
					{
						float minH = terrainHeightAtPos - maxSampleDelta;
						float maxH = terrainHeightAtPos + maxSampleDelta;
						if (h < minH) h = minH;
						if (h > maxH) h = maxH;
						return h;
					};

				float hL = ClampHeight(terrain->GetHeightAt(Vector3(pos.x - sampleOffset, 0.0f, pos.z)));
				float hR = ClampHeight(terrain->GetHeightAt(Vector3(pos.x + sampleOffset, 0.0f, pos.z)));
				float hD = ClampHeight(terrain->GetHeightAt(Vector3(pos.x, 0.0f, pos.z - sampleOffset)));
				float hU = ClampHeight(terrain->GetHeightAt(Vector3(pos.x, 0.0f, pos.z + sampleOffset)));

				Vector3 tangentX(2.0f * sampleOffset, hR - hL, 0.0f);
				Vector3 tangentZ(0.0f, hU - hD, 2.0f * sampleOffset);

				Vector3 normal;
				normal.x = tangentZ.y * tangentX.z - tangentZ.z * tangentX.y;
				normal.y = tangentZ.z * tangentX.x - tangentZ.x * tangentX.z;
				normal.z = tangentZ.x * tangentX.y - tangentZ.y * tangentX.x;

				if (normal.LengthSq() > FLT_EPSILON)
				{
					normal.Normalize();

					// ★修正: 求めた地形法線をそのまま使うのではなく、Diffに応じたweightで
					//         Vector3::Up とブレンドする。Diffが大きいほどUpに近づく。
					Vector3 blended = Vector3::Up * (1.0f - groundTiltWeight) + normal * groundTiltWeight;
					if (blended.LengthSq() > FLT_EPSILON)
					{
						blended.Normalize();
						groundNormal = blended;
					}
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
				sprintf_s(buf, "[Enemy#%d] terrain=%s isSwimming=%d diffSigned=%.1f weight=%.2f groundNormal=(%.3f,%.3f,%.3f) dot=%.5f tiltAngle(deg)=%.2f\n",
					m_logId, terrain ? "OK" : "NULL", isSwimming ? 1 : 0,
					pos.y - terrainHeightAtPos, groundTiltWeight,
					groundNormal.x, groundNormal.y, groundNormal.z,
					dotUpNormal, tiltAngle * 180.0f / 3.14159265f);
				OutputDebugStringA(buf);
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

			// ★修正: 以前はここでyRotとtiltRotを合成してからm_groundTiltRotationに
			//         Slerpしていましたが、それだとm_groundTiltRotationが「傾き」だけでなく
			//         「向き(ヨー)」も一緒に補間する形になってしまいます。
			//         キャラが歩き回って向きを変え続けている間、Slerpは常に
			//         「向きへの追いつき」をやり続けることになり、しかも真上ベクトルは
			//         ヨー回転だけでは変化しないため、向きへの追いつきが終わらない限り
			//         finalUpが(0,1,0)に張り付いて見えてしまいます。
			//         「一部のエネミーだけ傾きがまったく反映されない」症状の正体はこれだと
			//         考えられます。対策として、m_groundTiltRotationには「傾きのみ」を持たせ、
			//         向き(yRot)は毎フレーム素の値をそのまま掛け合わせる形にしました。
			//         Slerpが扱うのは常に0～GROUND_TILT_MAX_ANGLE程度の小さい回転だけになるので、
			//         キャラがどれだけ速く向きを変えても傾きの補間には影響しません。

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

			{
				Vector3 finalUp = Vector3::Up;
				m_groundTiltRotation.Apply(finalUp);
				float finalDot = max(-1.0f, min(1.0f, Vector3::Up.Dot(finalUp)));
				float finalTiltDeg = acosf(finalDot) * 180.0f / 3.14159265f;

				char buf[256];
				sprintf_s(buf, "[Enemy#%d] FINAL applied tilt = %.2f deg (finalUp=(%.3f,%.3f,%.3f)) isSwimming=%d\n",
					m_logId, finalTiltDeg, finalUp.x, finalUp.y, finalUp.z, isSwimming ? 1 : 0);
				OutputDebugStringA(buf);
			}

			// ★修正: 向き(yRot)は補間しない生の値をここで合成する。
			//         スムーズな向き変更自体はステートマシン/アニメーション側の役割。
			Quaternion finalRotation;
			finalRotation.Multiply(yRot, m_groundTiltRotation);

			// ★修正: m_modelRender.Update()を呼ぶとアニメーション再生時間が
			//         CharacterBase::Update()内の分と合わせて二重に進んでしまう（倍速の原因）。
			//         Skeleton::Update()はアニメーション時間を進めず行列を掛け合わせるだけなので、
			//         こちらを直接呼び、今フレーム分すでに計算済みのローカル行列（＝アニメのポーズ）は
			//         そのままに、ワールド行列だけを「傾いたルート行列」で再計算する。
			Vector3 rowX = Vector3::AxisX; finalRotation.Apply(rowX); rowX = rowX * m_transform.m_scale.x;
			Vector3 rowY = Vector3::AxisY; finalRotation.Apply(rowY); rowY = rowY * m_transform.m_scale.y;
			Vector3 rowZ = Vector3::AxisZ; finalRotation.Apply(rowZ); rowZ = rowZ * m_transform.m_scale.z;

			Matrix trs;
			trs.v[0].Set(rowX.x, rowX.y, rowX.z, 0.0f);
			trs.v[1].Set(rowY.x, rowY.y, rowY.z, 0.0f);
			trs.v[2].Set(rowZ.x, rowZ.y, rowZ.z, 0.0f);
			trs.v[3].Set(m_transform.m_position.x, m_transform.m_position.y, m_transform.m_position.z, 1.0f);

			// ★修正: ここで組み直しているtiltedWorldは、Model::CalcWorldMatrix()が
			//         本来作るワールド行列を丸ごと置き換えてSkeleton::Update()に
			//         渡している。CalcWorldMatrix()側はupAxis==Zのモデル（WhiteBear等）
			//         に対してX軸-90度の補正(mBias)を含めているが、ここではTRSだけを
			//         組んでいたためmBiasが抜け落ち、モデル全体がローカルX軸まわりに
			//         90度ズレて見えていた（＝頭が地面側に傾き、脚IKだけは正しく
			//         ワールド空間の地面ターゲットに足を伸ばそうとするので、
			//         結果的に「胴体は傾いているのに足だけ地面に真っ直ぐ」という
			//         見た目になっていた）。
			//         Model.cppのCalcWorldMatrix()と同じ補正をここでも掛ける。
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
				// ★修正: ここでassertして止めない。m_modelReady直後の数フレームは
				//         Skeletonがまだ完全に組み上がっていないだけの可能性があるため、
				//         falseを返して次フレームの再試行に委ねる。
				return false;
			}

			// ★修正: 前回の呼び出しで一部の脚だけ登録できてしまっている場合に備え、
			//         毎回クリアしてから登録し直す（再試行時に脚が重複登録されるのを防ぐ）。
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
				// ★修正: 1本でも失敗していたら中途半端な状態のまま確定させず、
				//         クリアして次フレーム再試行する。
				m_legIK.Clear();
				return false;
			}

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
