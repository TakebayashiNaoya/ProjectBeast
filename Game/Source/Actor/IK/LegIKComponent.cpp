/**
 * @file LegIKComponent.cpp
 * @brief キャラクター1体分の脚IKをまとめて管理するコンポーネントの実装
 * @author 立山
 */
#include "stdafx.h" 
#include "../../../../k2EngineLow/graphics/Skeleton.h"
#include "LegIKComponent.h"
#include "Physics/Physics.h"
#include "Source/Actor/Stage/TerrainObject.h" 
#include <cfloat>


namespace app
{
	namespace actor
	{
		namespace ik
		{
			namespace
			{
				/** IKウェイト（接地/非接地の切り替え）の補間速度 */
				constexpr float IK_WEIGHT_BLEND_SPEED = 8.0f;
				/** ルート下げ量の補間速度。速すぎるとカクつき、遅すぎると斜面で足が遅れる */
				constexpr float ROOT_DROP_BLEND_SPEED = 6.0f;
				/**
				 * 足が届かないときに、その脚のIKをフェードアウトさせるまでの距離。
				 * ルートを下げても届かない（＝ハイトマップに無い岩の上に立っている等）場合に、
				 * 脚を伸ばしきったまま宙で空振りさせないための保険。
				 */
				constexpr float LEG_FADE_RANGE = 40.0f;
				/**
				 * ルート下げ量の上限を求めるときの、地面法線Yの下限。
				 * CharacterControllerが「立てる床」とみなす上限（約63度）のcos値に合わせてある。
				 * これより急な面はそもそも接地扱いされないのでIKは走らない。
				 */
				constexpr float MIN_GROUND_NORMAL_Y = 0.45f;
				/**
				 * 斜面の理論浮き量に上乗せする許容量（カプセル半径に対する割合）。
				 * 尾根や段差の肩ではカプセルが半径内の最高点に乗るため、平面の式より余分に浮く。
				 */
				constexpr float ROOT_DROP_ALLOWANCE_RATIO = 0.5f;
				/** ウェイトがこれ以下の脚はIKを解かない（アニメのポーズそのまま） */
				constexpr float LEG_WEIGHT_EPSILON = 0.001f;

				/** 足元レイの上方向の長さ（カプセル半径に対する割合） */
				constexpr float FOOT_RAY_UP_RATIO = 2.5f;
				/** 足元レイの下方向の長さ（カプセル半径に対する割合） */
				constexpr float FOOT_RAY_DOWN_RATIO = 5.0f;
				/** カプセル半径が未設定のときに足元レイの長さの基準として使う値 */
				constexpr float FOOT_RAY_FALLBACK_SCALE = 50.0f;

				/** ワールド行列からVector3の平行移動成分を取り出す（v[3]はVector4想定） */
				inline Vector3 GetTranslation(const Matrix& m)
				{
					return Vector3(m.v[3].x, m.v[3].y, m.v[3].z);
				}
			}

			bool LegIKComponent::AddLegByBoneNames(
				nsK2EngineLow::Skeleton* skeleton,
				const wchar_t* hipBoneName,
				const wchar_t* kneeBoneName,
				const wchar_t* footBoneName,
				const Vector3& poleHintLocal,
				float bendSign,
				float footGroundOffset,
				const std::vector<const wchar_t*>& footChildBoneNames,
				const std::vector<const wchar_t*>& hipToKneeMidBoneNames,
				const std::vector<const wchar_t*>& kneeToFootMidBoneNames)
			{
				if (!skeleton) return false;

				int hipNo = skeleton->FindBoneID(hipBoneName);
				int kneeNo = skeleton->FindBoneID(kneeBoneName);
				int footNo = skeleton->FindBoneID(footBoneName);

				if (hipNo < 0 || kneeNo < 0 || footNo < 0) {
					return false;
				}

				LegIKChain chain;
				chain.hipBoneNo = hipNo;
				chain.kneeBoneNo = kneeNo;
				chain.footBoneNo = footNo;
				chain.poleHintLocal = poleHintLocal;
				chain.bendSign = bendSign;
				chain.footGroundOffset = footGroundOffset;

				for (const wchar_t* name : footChildBoneNames)
				{
					int boneNo = skeleton->FindBoneID(name);
					if (boneNo >= 0) { chain.footChildBoneNos.push_back(boneNo); }
				}
				for (const wchar_t* name : hipToKneeMidBoneNames)
				{
					int boneNo = skeleton->FindBoneID(name);
					if (boneNo >= 0) { chain.hipToKneeMidBoneNos.push_back(boneNo); }
				}
				for (const wchar_t* name : kneeToFootMidBoneNames)
				{
					int boneNo = skeleton->FindBoneID(name);
					if (boneNo >= 0) { chain.kneeToFootMidBoneNos.push_back(boneNo); }
				}

				m_legs.push_back(chain);
				return true;
			}


			bool LegIKComponent::CalcDesiredRootDrop(float& outDrop) const
			{
				outDrop = 0.0f;
				if (!m_isGroundInfoValid) return false;

				// キャラ座標（＝カプセルが接地して止まった足元の高さ）と、真下の実際の地面の高さの差。
				// これがそのままカプセルが斜面で浮いている量になる。
				const float floatAmount = m_characterPosition.y - m_groundHeight;

				// 地面より下（水中やレイの誤差）のときは持ち上げない
				if (floatAmount <= 0.0f) return true;

				// 斜面の角度から、浮いていて当然の量 radius*(1/cosθ-1) を求める。
				// これを上限にすることで、角度が急なほど必要になる下げ量がきちんと確保される。
				// （固定値を上限にすると、その値がちょうど「立てる床の上限63度」での浮き量と
				//   同じになったときに、一番補正が必要な急角度でIKが無効化されてしまう）
				float normalY = m_groundNormal.y;
				if (normalY < MIN_GROUND_NORMAL_Y) normalY = MIN_GROUND_NORMAL_Y;
				const float expectedFloat = m_capsuleRadius * (1.0f / normalY - 1.0f);

				// 尾根や段差の肩ではカプセルが半径内の最高点に乗るため、平面の式より余分に浮く。
				// その分の許容量を上乗せする。
				const float limit = expectedFloat + m_capsuleRadius * ROOT_DROP_ALLOWANCE_RATIO;

				// 許容量を超える＝崖の縁など、真下に地面が無いところにカプセルだけが
				// 乗っている状態。ここで下げると体が地面を突き抜けるのでIKごと切る。
				if (floatAmount > limit) return false;

				outDrop = floatAmount;
				return true;
			}


			float LegIKComponent::CalcFootGroundHeight(const Vector3& footPos) const
			{
				using namespace nsBeastEngine::nsCollision;

				// レイの区間はカプセル半径基準で決める（キャラの体格に自動で追従させるため）
				const float scale = (m_capsuleRadius > 1.0f) ? m_capsuleRadius : FOOT_RAY_FALLBACK_SCALE;

				const Vector3 rayStart(footPos.x, footPos.y + scale * FOOT_RAY_UP_RATIO, footPos.z);
				const Vector3 rayEnd(footPos.x, footPos.y - scale * FOOT_RAY_DOWN_RATIO, footPos.z);

				const btCollisionObject* me = m_selfCollisionObject;
				RaycastHit hit;
				const bool isHit = PhysicsWorld::Get().Raycast(
					rayStart, rayEnd, hit, ALL_COLLISION_ATTRIBUTE_MASK,
					[me](const btCollisionObject& obj) {
						if (me && &obj == me) return false;
						if (obj.getInternalType() == btCollisionObject::CO_GHOST_OBJECT) return false;
						return true;
					});

				if (isHit) return hit.point.y;

				// レイが何にも当たらなかったときだけハイトマップにフォールバックする
				return m_terrain ? m_terrain->GetHeightAt(footPos) : footPos.y;
			}


			void LegIKComponent::CalcLegSolveInfos(nsK2EngineLow::Skeleton* skeleton)
			{
				m_solveCache.resize(m_legs.size());

				for (size_t i = 0; i < m_legs.size(); ++i)
				{
					const LegIKChain& leg = m_legs[i];
					LegSolveInfo& info = m_solveCache[i];
					info.isValid = false;

					nsK2EngineLow::Bone* hipBone = skeleton->GetBone(leg.hipBoneNo);
					nsK2EngineLow::Bone* kneeBone = skeleton->GetBone(leg.kneeBoneNo);
					nsK2EngineLow::Bone* footBone = skeleton->GetBone(leg.footBoneNo);
					if (!hipBone || !kneeBone || !footBone) continue;

					const Vector3 hipPos = GetTranslation(hipBone->GetWorldMatrix());
					const Vector3 kneePos = GetTranslation(kneeBone->GetWorldMatrix());
					const Vector3 footPos = GetTranslation(footBone->GetWorldMatrix());

					// 現姿勢からボーン長を求める（SolveTwoBoneLegIK内と同じ計算）
					const float L1 = (kneePos - hipPos).Length();
					const float L2 = (footPos - kneePos).Length();
					if (L1 < FLT_EPSILON || L2 < FLT_EPSILON) continue;

					// XZはアニメーションのまま、Yだけ地面の高さに合わせる。
					// ハイトマップ(GetHeightAt)ではなく実コリジョンへのレイを使う。
					// 岩などハイトマップに存在しないオブジェクトの上に立つと、
					// 体はレイ基準で正しい高さに降りているのに足だけが
					// オブジェクトを突き抜けた地形（数十単位下）を狙ってしまい、
					// 届かずにフェードアウトしていたため。
					Vector3 target = footPos;
					target.y = CalcFootGroundHeight(footPos) + leg.footGroundOffset;

					// 股関節からターゲットまでの距離が、脚の伸ばしきり長を超えていたら足は届かない
					const float maxReach = (L1 + L2) * leg.maxReachRatio;
					const float distToTarget = (target - hipPos).Length();

					info.animFootPos = footPos;
					info.groundTarget = target;
					info.slack = maxReach - distToTarget;
					info.isValid = true;
				}
			}


			void LegIKComponent::Update(nsK2EngineLow::Skeleton* skeleton, float deltaTime)
			{
				// --- 次フレームのルート下げ量を求める ---
				// 斜面ではCharacterControllerのカプセルが側面で接地するため、キャラ原点は
				// 真下の地面より radius*(1/cosθ-1) だけ高い位置で止まる。その浮きぶんだけ
				// 描画用のルートを下げてやらないと、脚をどれだけ伸ばしても足が地面に届かない。
				// 浮き量は座標差から直接測れるので、脚が届くかどうかからは逆算しない
				// （足を高く上げるアニメーションのフレームで平地でも沈んでしまうため）。
				//
				// 接地していない間（ジャンプ中・遊泳中・急斜面の滑落中）や、真下に地面が無い
				// （崖の縁にカプセルだけ乗っている）ときは下げない。
				//
				// SetEnable(false)のとき（睡眠アニメ中など）も下げない。
				// 睡眠は巣の中で行われるが、真下レイは巣を貫通してその下の地面を拾うため、
				// 下げ続けるとモデルが巣に埋まってしまう。またこれらの状態ではキャラが
				// 移動しないので、斜面への追従自体が不要でもある。
				float desiredDrop = 0.0f;
				const bool isOnGroundSurface = CalcDesiredRootDrop(desiredDrop);
				const bool canDrop = m_isGrounded && m_isEnable && isOnGroundSurface;
				if (!canDrop) desiredDrop = 0.0f;

				const float dropBlend = min(1.0f, ROOT_DROP_BLEND_SPEED * deltaTime);
				m_rootDropOffset += (desiredDrop - m_rootDropOffset) * dropBlend;

				// --- 脚IKのウェイトを更新 ---
				// ※足元のターゲットは実コリジョンへのレイで取るので、m_terrainがnullでも成立する
				//   （m_terrainはレイが外れたときのフォールバックにしか使っていない）
				const bool canSolveLegs = canDrop && (skeleton != nullptr);

				const float weightBlend = min(1.0f, IK_WEIGHT_BLEND_SPEED * deltaTime);
				m_ikWeight += ((canSolveLegs ? 1.0f : 0.0f) - m_ikWeight) * weightBlend;

				// ウェイトが0まで落ちきったら、ボーンには何も書かずアニメのポーズをそのまま使う
				if (m_ikWeight <= LEG_WEIGHT_EPSILON) return;
				if (!skeleton) return;

				CalcLegSolveInfos(skeleton);

				// --- 各脚のIKを解く ---
				for (size_t i = 0; i < m_legs.size(); ++i)
				{
					const LegSolveInfo& info = m_solveCache[i];
					if (!info.isValid) continue;

					float legWeight = m_ikWeight;

					// ルートを下げても届かない脚は、伸ばしきったまま宙に残すより
					// アニメーションのポーズへ戻したほうが破綻が目立たない
					if (info.slack < 0.0f)
					{
						const float fade = 1.0f + info.slack / LEG_FADE_RANGE;
						legWeight *= max(0.0f, fade);
					}

					if (legWeight <= LEG_WEIGHT_EPSILON) continue;

					// ウェイトぶんだけ、アニメの足位置から地形ターゲットへ寄せる
					const Vector3 target = info.animFootPos + (info.groundTarget - info.animFootPos) * legWeight;

					SolveTwoBoneLegIK(skeleton, m_legs[i], target);
				}
			}
		}
	}
}
