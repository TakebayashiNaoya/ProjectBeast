/**
 * @file LegIKComponent.h
 * @brief キャラクター1体分の脚IKをまとめて管理するコンポーネント
 * @author 立山
 */
#pragma once
#include "TwoBoneLegIK.h"
#include <vector>

namespace nsK2EngineLow
{
	class Skeleton;
}

/** レイキャストで自分自身を除外するために型だけ必要 */
class btCollisionObject;

namespace app
{
	namespace actor
	{
		class TerrainObject;

		namespace ik
		{
			class LegIKComponent
			{
			public:
				/**
				 * @brief 参照する地形を設定
				 */
				void SetTerrain(TerrainObject* terrain) { m_terrain = terrain; }

				/**
				 * @brief キャラクターが接地しているかを設定する
				 * @details CharacterController::IsOnGround()の結果をそのまま渡す。
				 *          ジャンプ中・遊泳中・急斜面の滑落中（CharacterControllerが
				 *          接地扱いしない状態）はIKを切ってアニメのポーズへ戻すために使う。
				 */
				void SetGrounded(bool isGrounded) { m_isGrounded = isGrounded; }

				/**
				 * @brief 脚IKそのものの有効／無効を設定する
				 * @details 睡眠など、地面に合わせると姿勢が崩れるアニメーションの間にfalseにする。
				 *          脚の解決とルートの下げ量（GetRootDropOffset）の両方が止まり、
				 *          モデルはアニメーション本来のポーズ・高さのままになる。
				 *          切り替えはどちらも補間されるのでポップしない。
				 *
				 *          下げ量も止めるのは、睡眠が巣の中で行われるため。真下レイは巣を
				 *          貫通してその下の地面を拾うので、下げ続けるとモデルが巣に埋まる。
				 */
				void SetEnable(bool isEnable) { m_isEnable = isEnable; }

				/**
				 * @brief キャラクターのワールド座標（＝CharacterControllerが返す足元座標）を設定する
				 * @details ルートの下げ量は、この座標のYと真下の地面の高さの差から直接求める。
				 *          脚の届く／届かないから逆算すると、足を高く上げるアニメーションの
				 *          フレームで平地でも体が沈んでしまうため。
				 */
				void SetCharacterPosition(const Vector3& position) { m_characterPosition = position; }

				/**
				 * @brief 足元の地面情報を設定する
				 * @details CharacterControllerが真下レイで取得した実際の地面の高さと法線。
				 *          ハイトマップではなく実コリジョンの結果なので、岩などの
				 *          配置オブジェクトの上に立っている場合も正しい値が入る。
				 * @param isValid      地面情報が取得できているか
				 * @param groundHeight 真下の地面の高さ
				 * @param groundNormal 真下の地面の法線
				 */
				void SetGroundInfo(bool isValid, float groundHeight, const Vector3& groundNormal)
				{
					m_isGroundInfoValid = isValid;
					m_groundHeight = groundHeight;
					m_groundNormal = groundNormal;
				}

				/**
				 * @brief キャラクターのカプセル半径を設定する
				 * @details 斜面での想定浮き量 radius*(1/cosθ-1) を求めるのに使う。
				 *          これがルート下げ量の上限になる。
				 */
				void SetCapsuleRadius(float radius) { m_capsuleRadius = radius; }

				/**
				 * @brief 自分自身のコリジョンオブジェクトを設定する
				 * @details 足元へ飛ばすレイが自分のカプセルに当たらないよう除外するために使う。
				 *          CharacterController::GetRigidBody()->GetBody() を渡すこと。
				 */
				void SetSelfCollisionObject(btCollisionObject* self) { m_selfCollisionObject = self; }

				/**
				 * @brief 描画用ルートを下げるべき量（補間済み）を取得する
				 * @details 呼び出し側は、スケルトンのワールド行列を組むときに
				 *          Y座標からこの値を引くこと。物理座標(m_transform)は変更しない。
				 * @return 下げ量（常に0以上）
				 */
				float GetRootDropOffset() const { return m_rootDropOffset; }

				/** @brief 現在のIKの効き具合（0.0～1.0、デバッグ表示用） */
				float GetIKWeight() const { return m_ikWeight; }

				/**
				 * @brief 脚のチェーンを直接追加（ボーン番号が分かっている場合）
				 */
				void AddLeg(const LegIKChain& chain) { m_legs.push_back(chain); }

				/**
				 * @brief 登録済みの脚をすべて削除する
				 * @details InitLegIK()などが途中で失敗して再試行する際、
				 *          前回分の脚が重複登録されるのを防ぐために使う。
				 */
				void Clear() { m_legs.clear(); }

				/**
				 * @brief ボーン名から脚のチェーンを追加
				 * @return ボーンが見つからず追加できなかった場合はfalse
				 */
				bool AddLegByBoneNames(
					nsK2EngineLow::Skeleton* skeleton,
					const wchar_t* hipBoneName,
					const wchar_t* kneeBoneName,
					const wchar_t* footBoneName,
					const Vector3& poleHintLocal,
					float bendSign = 1.0f,
					float footGroundOffset = 0.0f,
					const std::vector<const wchar_t*>& footChildBoneNames = {},
					const std::vector<const wchar_t*>& hipToKneeMidBoneNames = {},
					const std::vector<const wchar_t*>& kneeToFootMidBoneNames = {});

				/**
				 * @brief 全ての脚のIKを解いて反映する
				 * @details Skeleton::Update() の後、そのフレームの描画・
				 *          足跡デカール生成（DecalManagerへの登録）より前に呼んでください。
				 *          IK後の足の位置に足跡を出したほうが見た目が合うはずです。
				 *
				 *          あわせて次フレーム用のルート下げ量（GetRootDropOffset）を更新する。
				 *          下げ量の反映は呼び出し側の責務（1フレーム遅れるが、
				 *          どのみち補間しているので見た目には出ない）。
				 * @param skeleton  対象のスケルトン
				 * @param deltaTime フレームの経過時間（ウェイト・下げ量の補間に使う）
				 */
				void Update(nsK2EngineLow::Skeleton* skeleton, float deltaTime);

				/** 登録済みの脚の数（デバッグ表示等に） */
				size_t GetLegCount() const { return m_legs.size(); }

			private:
				/** 1フレーム分の脚ごとの計算結果（毎フレームのヒープ確保を避けるためメンバに持つ） */
				struct LegSolveInfo
				{
					/** アニメーションが決めた本来の足首ワールド座標 */
					Vector3 animFootPos = Vector3::Zero;
					/** 地形に合わせたい足首ワールド座標 */
					Vector3 groundTarget = Vector3::Zero;
					/** 脚の到達可能距離 - ターゲットまでの距離。負なら足が届いていない */
					float slack = 0.0f;
					/** 計算が成立したか（ボーンが取れない等でfalse） */
					bool isValid = false;
				};

				/** 脚ごとのターゲットと到達余裕を計算して m_solveCache に詰める */
				void CalcLegSolveInfos(nsK2EngineLow::Skeleton* skeleton);

				/**
				 * @brief 足のXZ位置の真下にある地面の高さを求める
				 * @details 実コリジョンへのレイキャスト。ハイトマップ(GetHeightAt)では
				 *          岩などの配置オブジェクトの上に立ったときに、そのオブジェクトを
				 *          突き抜けた地形の高さを返してしまうため。
				 *          レイが外れた場合のみハイトマップにフォールバックする。
				 * @param footPos 足首のワールド座標（Yは無視され、XZだけ使う）
				 * @return 地面の高さ
				 */
				float CalcFootGroundHeight(const Vector3& footPos) const;

				/**
				 * @brief 今フレームのルート下げ量の目標値を求める
				 * @details キャラ座標のYと真下の地面の高さの差＝カプセルが斜面で浮いている量。
				 * @param[out] outDrop 下げ量（0以上）
				 * @return 地面の上に立っていると判断できたか（falseならIKごと切る）
				 */
				bool CalcDesiredRootDrop(float& outDrop) const;

			private:
				std::vector<LegIKChain> m_legs;
				std::vector<LegSolveInfo> m_solveCache;
				TerrainObject* m_terrain = nullptr;
				/** 足元レイから除外する自分自身のコリジョンオブジェクト */
				btCollisionObject* m_selfCollisionObject = nullptr;

				/** キャラクターのワールド座標（CharacterControllerの結果） */
				Vector3 m_characterPosition = Vector3::Zero;
				/** 足元の地面の法線（CharacterControllerの真下レイの結果） */
				Vector3 m_groundNormal = Vector3::Up;
				/** 足元の実際の地面の高さ（CharacterControllerの真下レイの結果） */
				float m_groundHeight = 0.0f;
				/** 描画用ルートの下げ量（補間済み） */
				float m_rootDropOffset = 0.0f;
				/** キャラクターのカプセル半径 */
				float m_capsuleRadius = 0.0f;
				/** IK全体の効き具合（0.0～1.0、補間済み） */
				float m_ikWeight = 0.0f;
				/** 接地中かどうか */
				bool m_isGrounded = false;
				/** 脚IKが有効か（睡眠アニメ中などはfalseにする） */
				bool m_isEnable = true;
				/** 足元の地面情報が有効か */
				bool m_isGroundInfoValid = false;
			};
		}
	}
}
