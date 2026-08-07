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
			/** 胴体を傾ける角度の上限。
			 *  接地判定はカプセル基準なので、真下レイの法線のほうが急な角度を返すことがある
			 *  （崖の肩にカプセルが乗っている場合など）。そのまま適用すると体が倒れて見えるため、
			 *  ここでクランプする。 */
			constexpr float GROUND_TILT_MAX_ANGLE = 1.5533f; // 約70度

			// ★削除: GROUND_TILT_NORMAL_SAMPLE_OFFSET / GROUND_TILT_MAX_HEIGHT_DIFF*
			//         / GROUND_TILT_RAY_* は、ハイトマップの4点サンプリングで法線を推定して
			//         いた頃の定数。CharacterControllerの真下レイから実コリジョンの法線を
			//         直接受け取るようになったため、すべて不要になった。


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

			// ★修正: ハイトマップの4点サンプリングをやめ、CharacterControllerが真下レイで
			//         取得した「実コリジョンの法線」をそのまま使う。旧実装には
			//         次の問題があり、いずれもこの変更で根本から無くなる。
			//         ・岩など、ハイトマップに存在しない配置オブジェクトの上では
			//           まったく違う法線になっていた
			//         ・サンプリング距離に依存して検出できる角度に上限があった
			//         ・「キャラY - ハイトマップ高さ」の差で重み付けしていたが、斜面では
			//           カプセルの浮きでこの差が正常に大きくなるため、急斜面ほど傾きが
			//           弱まるという逆の挙動になっていた（52度の斜面で14度しか傾かない等）
			//
			//         泳いでいる間はキャラが地面の上に立っていないので、
			//         従来どおり水平（Up）のまま扱う。
			//
			// ★修正: 接地していない間（急斜面の滑落中・ジャンプ中）も傾けない。
			//         滑落中は真下レイが崖の壁面（80度前後）を拾うため、そのまま適用すると
			//         胴体が上限の70度まで倒れて横倒しに見えてしまう。
			//         旧実装では「キャラY - ハイトマップ高さ」の差による重み付けが
			//         結果的にこれを抑えていたが、その重みを廃止したので明示的に弾く。
			//         脚IK側の条件（IsOnGround）とも揃う。
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

			// ★修正: 斜面ではCharacterControllerのカプセルが側面で接地するため、キャラ原点は
			//         真下の地面より radius*(1/cosθ-1) だけ高い位置で止まる。これが
			//         「急斜面で足がつかない」直接の原因。物理・ステート判定用のm_transformには
			//         手を加えず、描画用のルートだけをLegIKComponentが算出した量ぶん下げる。
			Matrix trs;
			trs.v[0].Set(rowX.x, rowX.y, rowX.z, 0.0f);
			trs.v[1].Set(rowY.x, rowY.y, rowY.z, 0.0f);
			trs.v[2].Set(rowZ.x, rowZ.y, rowZ.z, 0.0f);
			trs.v[3].Set(m_transform.m_position.x, m_transform.m_position.y - m_legIK.GetRootDropOffset(), m_transform.m_position.z, 1.0f);

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
