/**
 * @file ChildPenguinAIController.cpp
 * @brief 子ペンギンのAIコントローラー
 */
#include "stdafx.h"

#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "ChildPenguinTypes.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "graphics/effect/BeastEffectEmitter.h"
#include "NaughtyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Actor/Stage/StageNavGrid.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Actor/Stage/TerrainObject.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Manager/BattleManager.h"
#include "Source/Manager/IglooManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Util/RandomDevice.h"
#include "Source/UI/CPReaction/CPReactionTypes.h"
#include <algorithm>


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 甘えん坊ペンギンのエフェクトオフセット */
			const Vector3 CLINGY_HART_EFFECT_POSITION = { 0.0f,30.0f,0.0f };
			/** 甘えん坊ペンギンのエフェクトスケール */
			const Vector3 CLINGY_HART_EFFECT_SCALE = { 10.0f,10.0f,10.0f };
			/** 世話焼きペンギンのエフェクトオフセット */
			const Vector3 CARING_SWEAT_EFFECT_POSITION = { 0.0f,30.0f,0.0f };
			/** 世話焼きペンギンのエフェクトスケール */
			const Vector3 CARING_SWEAT_EFFECT_SCALE = { 10.0f,10.0f,10.0f };
			/** やんちゃペンギンのエフェクトスケール */
			const Vector3 NAUGHTY_LIVELY_EFFECT_SCALE = { 50.0f,50.0f,50.0f };
			/** 世話焼きペンギンの最大再生回数 */
			constexpr int MAX_SWEAT_COUNT = 3;
			/** ずらす間隔 */
			constexpr float SWEAT_INTERVAL = 0.3f;
			/** 秒数判定 */
			constexpr float LIVELY_INTERVAL = 1.5f;


			/** シロクマを起こすための距離 */
			constexpr float WAKE_BEAR_TRIGGER_DISTANCE = 300.0f;
			/** シロクマに到達するための距離 */
			constexpr float REACH_BEAR_DISTANCE = 80.0f;
			/** やんちゃペンギンがシロクマを起こした後、または制止された後の再行動抑制時間 */
			constexpr float SCOLD_COOLDOWN_DURATION = 5.0f;
			/** 渦潮消滅時の短い反省時間 */
			constexpr float WHIRLPOOL_MISS_COOLDOWN_DURATION = 2.0f;
			/** 渦潮に到達するための距離 */
			constexpr float REACH_WHIRLPOOL_DISTANCE = 30.0f;
			/** 渦潮を見つけて向かい始める距離 */
			constexpr float WHIRLPOOL_TRIGGER_DISTANCE = 300.0f;

			/**
			 * @brief ヒステリシス幅
			 * @details フェーズを下げるとき、閾値からさらにこの距離だけ内側に入って初めて下げる。
			 *          m_stopDistance より小さい値にすること。
			 */
			constexpr float HYSTERESIS = 5.0f;

			/**
			 * @brief フェーズを上げるためのマージン（遊び）
			 * @details 閾値からさらにこの距離だけ外側に離れて初めてフェーズを上げる。
			 */
			constexpr float PHASE_UP_MARGIN = 5.0f;

			/**
			 * @brief Stopフェーズを抜けるためのマージン（遊び）
			 * @details 強制Stop（距離＋速度の判定）は stopDistance + HYSTERESIS まで許すため、
			 *          抜けの閾値はそれより外側に置く。PHASE_UP_MARGIN と同じ値だと
			 *          入りと出が同一距離で接し、親が止まった直後に Stop/Walk が毎フレーム
			 *          反転して子がその場でばたつく（2026-08-25 実バグ・修正済み）。
			 *          大きくしすぎると渦潮の吸引などのドリフトへ抵抗し始めるのが遅れて
			 *          飲まれが増えるため、HYSTERESIS より少し外側に留める
			 *          （15で試したらHardの飲まれが激増した。2026-08-26 ボット実測）。
			 */
			constexpr float STOP_EXIT_MARGIN = 8.0f;

			/**
			 * @brief 停止判定で共通利用する速度の閾値（速度の二乗で比較）
			 * @details Walk → Stop 遷移と BuildInputToTarget() 冒頭の強制 Stop 判定の
			 *          両方でこの定数を参照する。
			 *          lerpの慣性が残っているうちは Stop に入らず Walk を維持し、
			 *          停止アニメ中も滑り続ける問題を防ぐ。
			 *          厳しすぎる（0.1=0.32u/s）と目標付近の微速移動が延々と続いて
			 *          Stopに入れず足踏みになるため、5u/s（残留滑りは知覚できない量）まで許す
			 */
			constexpr float STOP_VELOCITY_THRESHOLD_SQ = 25.0f;


			/**
			 * @brief 逃走方向を次に変えるまでの最短保持時間（秒）
			 */
			constexpr float FLEE_DIR_HOLD_MIN = 1.5f;
			/**
			 * @brief 逃走方向を次に変えるまでの最長保持時間（秒）
			 */
			constexpr float FLEE_DIR_HOLD_MAX = 2.0f;
			/**
			 * @brief 逃走時に直進を選ぶ確率（残りは横方向回避）
			 */
			constexpr float FLEE_STRAIGHT_CHANCE = 0.4f;
			/**
			 * @brief 回避時の最小角度（ラジアン） = 45度
			 */
			constexpr float FLEE_DODGE_ANGLE_MIN = 0.785398f;
			/**
			 * @brief 回避時の最大角度（ラジアン） = 90度
			 */
			constexpr float FLEE_DODGE_ANGLE_MAX = 1.570796f;
			/**
			 * @brief 左右どちらによけるかの確率
			 */
			constexpr float FLEE_SIGN_FLIP_CHANCE = 0.5f;

			//============================================//
			// 散開の距離と復帰の速さ（要調整のつまみ）
			//
			// 逃走をシロクマ最優先にすると「群れごと失って集め直す」体験が生まれるが、
			// やりすぎると理不尽になる（プレイログ計測基盤と実測結果 7-4）。
			// 散り方はこの2つと ChildPenguinManager の REGROUP_CALL_* で決まる。
			//============================================//

			/**
			 * @brief クマが離れてからも逃げ続ける時間（秒）
			 * @details 0にすると、クマの脇をすり抜けた瞬間に立ち止まって不自然になる。
			 *          長くするほど遠くまで散る。
			 */
			constexpr float FLEE_HOLD_TIME = 1.0f;
			/**
			 * @brief 逃げ終わってから隊列へ戻れるようになるまでの時間（秒）
			 * @details ここが「集め直す時間」の長さ。
			 *          0にすると散った直後にその場で入隊し直してしまい、谷が出ない。
			 *          親の再集合の呼びかけ（Yボタン）で打ち切れる。
			 */
			constexpr float FLEE_REJOIN_COOLDOWN = 2.0f;


			//============================================//
			// 親の察知（視界と音）
			//
			// 各数値の根拠と負荷の見積もりは docs/子ペンギンの察知モデル.md にある。
			// 触るときは必ずそちらの表も更新すること。
			//============================================//

			/**
			 * @brief タイプ別の察知性能テーブル
			 * @details 添字は EnChildPenguinType の値。
			 *
			 *          「まじめ＝よく気づく／やんちゃ＝気づかない」を作るのが目的。
			 *          まじめは配合でしか差が付けられず実測でも個性が見えていなかったため、
			 *          ここで差別化する（プレイログ計測基盤と実測結果 7-5）。
			 *
			 *          JSON に出していないのは、タイプ別パラメーターの
			 *          JSON→bin 変換が Tools/ParameterConvert.py にあり、
			 *          そこはストリーム C の占有ファイルのため。
			 *          調整が要るようなら C と話をつけてから JSON へ移す。
			 */
			constexpr DaddyPerceptionSpec DADDY_PERCEPTION_SPECS[] =
			{
				/** まじめ    ：一番よく気づく。この差が「まじめらしさ」になる */
				{ 420.0f, 120.0f, 0.2f, 1.20f },
				/** 甘えん坊  ：親を探しているので広い */
				{ 360.0f, 100.0f, 0.3f, 1.10f },
				/** やんちゃ  ：よそ見をしていて気づかない */
				{ 180.0f,  50.0f, 1.2f, 0.60f },
				/** おっちょこ：どんくさい */
				{ 240.0f,  70.0f, 0.8f, 0.85f },
				/** 世話焼き  ：標準（シロクマの視野角 70 の左右版にあたる 90 度） */
				{ 300.0f,  90.0f, 0.4f, 1.00f },
			};
			static_assert(
				sizeof(DADDY_PERCEPTION_SPECS) / sizeof(DADDY_PERCEPTION_SPECS[0])
					== static_cast<size_t>(EnChildPenguinType::Num),
				"DADDY_PERCEPTION_SPECS は EnChildPenguinType と同じ数だけ要素が要る");

			/**
			 * @brief 察知を忘れるまでの時間（秒）
			 * @details 知覚できない状態がこれだけ続くと、寄っていくのをやめてその場で待つ。
			 *          これが無いと、一度でも親を見た子が最後まで親を追い続けて
			 *          ステージ中の子が全員集まってしまう。
			 */
			constexpr float NOTICE_FORGET_TIME = 4.0f;

			/**
			 * @brief 遮蔽レイキャストを撃つ間隔（フレーム）
			 * @details 子ごとにスロットをずらしてあるので、
			 *          1フレームあたりのレイ本数の上限は「子の数 / この値」。
			 *          子100体なら最大12.5本／フレームで、
			 *          実際は距離と向きの足切りを通った子だけなのでさらに少ない。
			 */
			constexpr unsigned int OCCLUSION_CHECK_INTERVAL = 8;

			/**
			 * @brief 遮蔽レイキャストの視点の高さ
			 * @details 足元どうしを結ぶと、なだらかな凸斜面でも地面をかすめて
			 *          「見えていない」と誤判定する。ペンギンの背丈 70 の目の高さに合わせて浮かせる。
			 */
			constexpr float PERCEPTION_EYE_HEIGHT = 42.0f;

			/**
			 * @brief AIがスライドを諦める上り傾斜（符号つき傾斜 sinθ）
			 * @details -0.15 は約8.6度の上り。傾斜モデルの強化で上りのスライドは
			 *          走りより大幅に遅くなったため、これより急な上りでは走りに切り替える。
			 */
			constexpr float CHILD_SLIDE_UPHILL_LIMIT = -0.15f;

			/**
			 * @brief 移動手段を傾斜で選ぶときのしきい値（符号つき傾斜 sinθ。正が下り）
			 * @details 下り 0.06（約3.4度）を超えたらスライド、
			 *          上り -0.06 を超えたら歩き、その間は走り。
			 *          出る側のしきい値（EXIT）を緩くして境界でのばたつきを防ぐ。
			 */
			constexpr float CHILD_SLOPE_SLIDE_ENTER = 0.06f;
			constexpr float CHILD_SLOPE_SLIDE_EXIT  = 0.02f;
			constexpr float CHILD_SLOPE_WALK_ENTER  = -0.06f;
			constexpr float CHILD_SLOPE_WALK_EXIT   = -0.02f;

			/**
			 * @brief 渦潮回避の半径と反発の強さ
			 * @details 渦潮の吸い込み半径（whirlpoolRadius 200）に余白を足した距離から
			 *          反発をかけ、自分から吸い込み範囲へ入っていくのを防ぐ。
			 *          重さ2.0で、回避半径の中ほどでは進行方向より反発が勝つ。
			 */
			constexpr float WHIRLPOOL_AVOID_RADIUS = 320.0f;
			constexpr float WHIRLPOOL_AVOID_WEIGHT = 2.0f;

			/**
			 * @brief 音に気づいて振り向くときの旋回速度の倍率
			 * @details 「？」を出しながらゆっくり親のほうへ振り向くための値。
			 *          既定の旋回速度 8.0 に掛かる。1.0 だと一瞬で振り向いてしまい、
			 *          「音で気づく → 振り向く → 目で見つける」の段階が見えない
			 *          （2026-08-23 試遊フィードバック）。
			 */
			constexpr float INVESTIGATE_TURN_MULTIPLIER = 0.2f;

			/**
			 * @brief フローフィールド追従を使い始める目標までの距離
			 * @details これより近い移動は直進（隊列スロットへの精密な寄せを乱さないため）。
			 */
			constexpr float NAV_FLOW_MIN_DISTANCE = 200.0f;

			/**
			 * @brief 「親へ向かう移動」とみなす目標と親の距離
			 * @details 隊列スロットは親の周囲に並ぶため、この距離以内の目標への移動は
			 *          親へのフローフィールドで代用できる。徘徊先などはこれより遠いので直進のまま。
			 */
			constexpr float NAV_FLOW_TARGET_NEAR_DADDY = 400.0f;

			/** やんちゃのスタック判定：この距離（XZ）も動けていなければ停滞とみなす */
			constexpr float NAUGHTY_STUCK_MOVE_THRESHOLD = 10.0f;
			/** やんちゃのスタック判定：この時間（秒）停滞し続けたら行き先を変える */
			constexpr float NAUGHTY_STUCK_TIME_LIMIT = 1.5f;
			/** スタック脱出時に「進めなかった方向の反対」から振るランダム角の半角（ラジアン、約75度） */
			constexpr float NAUGHTY_STUCK_ESCAPE_HALF_ANGLE = 1.3f;

			/**
			 * @brief 再集合の呼びかけを無効にする「親とクマの距離」
			 * @details 親からこの距離以内に狩り中のクマがいる場合、呼びかけで子を
			 *          親のもとへ向かわせない（クマの口元へ呼び戻すことになるため）。
			 *          300だと「クマに追われている最中の呼びかけ」がほぼ常に無効になり
			 *          再集合が機能しなかったため、本当に至近のときだけ無効にする。
			 */
			constexpr float REGROUP_DADDY_SAFE_DISTANCE = 150.0f;

			/**
			 * @brief 呼びかけに応えた子が勇敢でいる時間（秒）
			 * @details 呼びかけの効果（2秒）が切れた瞬間にクマの300以内の子がまた散ると
			 *          「集めても集まらない」になるため、応えた子はこの時間だけ
			 *          「自分が狙われている」か「クマが至近」でない限り逃げない。
			 */
			constexpr float REGROUP_BRAVE_TIME = 6.0f;

			/**
			 * @brief 勇敢な子でも逃げ出す「クマとの距離」
			 * @details 呼びかけに応えた子も、狩り中のクマがこの距離まで来たら流石に逃げる。
			 */
			constexpr float BRAVE_PANIC_DISTANCE = 120.0f;


			/**
			 * @brief 範囲 [r.min, r.max] から一様乱数を生成する
			 * @param r 範囲
			 * @return 生成された乱数値
			 */
			float RollRange(const MasterChildPenguinTypeParameter::Range& r)
			{
				return util::RandomDevice::Random(r.min, r.max);
			}

			/**
			 * @brief [0, 1) の一様乱数を生成する
			 * @return 生成された乱数値
			 */
			float RollUnit()
			{
				return util::RandomDevice::Random(0.0f, 1.0f);
			}

			/**
			 * @brief タイプに対応する TypeData を取得する
			 * @details MasterChildPenguinTypeParameter はJSON配列の要素ごとに別インスタンスとして
			 * ロードされる。GetParameter() に type の値をそのまま渡すと、該当レコードへの
			 * ポインタが直接返ってくる。
			 * @param type 子ペンギンのタイプ
			 * @return タイプ別パラメーター
			 */
			const MasterChildPenguinTypeParameter& GetTypeData(EnChildPenguinType type)
			{
				const int index = static_cast<int>(type);
				const auto* param = core::ParameterManager::Get()->GetParameter<MasterChildPenguinTypeParameter>(index);
				return *param;
			}
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// ChildPenguinAIController（基底クラス）
		//--------------------------------------------------------------

		ChildPenguinAIController::ChildPenguinAIController(ChildPenguin* owner, EnChildPenguinType type)
			: m_owner(owner)
			, m_stateMachine(owner->GetStateMachine())
			, m_hartEffectHandle(INVALID_EFFECT_HANDLE)
		{
			const auto& td = GetTypeData(type);

			/** 速度系個体値を決定してStatusに反映する */
			const float runSpeed = RollRange(td.runSpeed);
			const float swimSpeed = RollRange(td.swimSpeed);
			const float sneakSpeed = RollRange(td.sneakSpeed);
			const float slideSpeed = RollRange(td.slideSpeed);
			const float jumpPower = RollRange(td.jumpPower);
			owner->GetStatus<ChildPenguinStatus>()->SetIndividualValues(
				runSpeed, swimSpeed, sneakSpeed, slideSpeed, jumpPower);

			/** 距離系個体値を決定する */
			m_stopDistance = RollRange(td.stopDistance);
			m_walkDistance = RollRange(td.walkDistance);
			m_runDistance = RollRange(td.runDistance);

			/** 制約補正：stopDistance < walkDistance < runDistance */
			m_walkDistance = max(m_walkDistance, m_stopDistance + 1.0f);
			m_runDistance = max(m_runDistance, m_walkDistance + 1.0f);

			/** タイプ別の察知性能と、遮蔽レイキャストを撃つフレームのスロットを確定させる */
			m_perceptionSpec = &DADDY_PERCEPTION_SPECS[static_cast<int>(type)];

			/**
			 * リプレイ再生（ReplayScene）は ChildPenguinManager を作らずに
			 * ChildPenguin を生成するため、ここだけは null を許容する。
			 * リプレイでは AI の Update() 自体が回らないのでスロットは使われない
			 */
			if (auto* manager = ChildPenguinManager::GetInstance())
			{
				m_perceptionSlot = manager->IssuePerceptionSlot();
			}
		}


		void ChildPenguinAIController::Update()
		{
			/** タイプによらず先に親の察知を更新する（入隊判定がこの結果を使う） */
			UpdatePerception();

			UpdateAI();
		}


		Vector3 ChildPenguinAIController::CalculateDirectionToDaddy() const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			const Vector3& daddyPos = ChildPenguinManager::GetInstance()->GetDaddyPosition();
			Vector3 direction = daddyPos - childPos;
			direction.y = 0.0f;
			if (direction.LengthSq() < FLT_EPSILON)
			{
				return Vector3::Zero;  // 呼び出し側の LengthSq チェックで弾かれる
			}
			direction.Normalize();
			return direction;
		}


		float ChildPenguinAIController::GetDistanceToDaddy() const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			const Vector3& daddyPos = ChildPenguinManager::GetInstance()->GetDaddyPosition();
			Vector3 diff = daddyPos - childPos;
			diff.y = 0.0f;
			return diff.Length();
		}


		Vector3 ChildPenguinAIController::CalculateDirectionToTarget(const Vector3& targetPos) const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			Vector3 direction = targetPos - childPos;
			direction.y = 0.0f;
			if (direction.LengthSq() < FLT_EPSILON)
			{
				return Vector3::Zero;
			}
			direction.Normalize();
			return direction;
		}


		float ChildPenguinAIController::GetDistanceToTarget(const Vector3& targetPos) const
		{
			const Vector3& childPos = m_owner->GetTransform().m_position;
			Vector3 diff = targetPos - childPos;
			diff.y = 0.0f;
			return diff.Length();
		}


		void ChildPenguinAIController::BuildInput()
		{
			// SlideEnd アニメーション中は入力をゼロにして自然に滑り止まるのを待つ
			if (m_stateMachine->IsEqualCurrentState(PenguinSlideEndState::ID()))
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			const Vector3 targetPos = m_owner->GetFormationTargetPosition();
			BuildInputToTarget(targetPos);
		}


		StageNavGrid* ChildPenguinAIController::GetStageNavGrid() const
		{
			auto* stageSystem = StageSystem::GetInstance();
			auto* terrain = (stageSystem != nullptr) ? stageSystem->GetTerrain() : nullptr;
			if (terrain == nullptr || !terrain->GetNavGrid().IsBuilt()) return nullptr;
			return &terrain->GetNavGrid();
		}


		Vector3 ChildPenguinAIController::CalculateMoveDirectionWithNav(const Vector3& targetPos, const float distToTarget)
		{
			/**
			 * 親（隊列）へ向かう遠距離移動だけ、親へのフローフィールドに沿って
			 * 水路や崖を回り込む。目標が親の近く（隊列スロット含む）で、
			 * かつ十分離れているときだけフローを使い、近距離はスロットへの
			 * 精密な寄せを乱さないよう直進にする
			 */
			auto* manager = ChildPenguinManager::GetInstance();
			if (distToTarget > NAV_FLOW_MIN_DISTANCE)
			{
				Vector3 targetToDaddy = targetPos - manager->GetDaddyPosition();
				targetToDaddy.y = 0.0f;
				if (targetToDaddy.LengthSq()
					<= NAV_FLOW_TARGET_NEAR_DADDY * NAV_FLOW_TARGET_NEAR_DADDY)
				{
					Vector3 flowDir;
					if (manager->GetDaddyFlowDirection(m_owner->GetTransform().m_position, flowDir))
					{
						return ApplyWhirlpoolAvoidance(flowDir);
					}
				}
			}

			return ApplyWhirlpoolAvoidance(CalculateDirectionToTarget(targetPos));
		}


		Vector3 ChildPenguinAIController::ApplyWhirlpoolAvoidance(const Vector3& moveDir) const
		{
			/** やんちゃのいたずらダイブなど、意図的に向かっている間は回避しない */
			if (IsHeadingIntoWhirlpoolOnPurpose()) return moveDir;

			auto* wpManager = nature::WhirlpoolManager::GetInstance();
			if (wpManager == nullptr) return moveDir;

			const Vector3 myPos = m_owner->GetTransform().m_position;
			Vector3 avoid = Vector3::Zero;
			wpManager->ForEach([&](nature::Whirlpool* wp)
				{
					Vector3 diff = myPos - wp->GetTransform().m_position;
					diff.y = 0.0f;
					const float dist = diff.Length();
					if (dist < 1.0f || dist >= WHIRLPOOL_AVOID_RADIUS) return;

					/** 近いほど強い反発（回避半径の縁で0、中心で1） */
					avoid += diff * (1.0f / dist) * (1.0f - dist / WHIRLPOOL_AVOID_RADIUS);
				});

			if (avoid.LengthSq() < FLT_EPSILON) return moveDir;

			Vector3 result = moveDir + avoid * WHIRLPOOL_AVOID_WEIGHT;
			result.y = 0.0f;
			if (result.LengthSq() < FLT_EPSILON) return moveDir;

			result.Normalize();
			return result;
		}


		void ChildPenguinAIController::BuildInputToTarget(const Vector3& targetPos)
		{
			const float distToTarget = GetDistanceToTarget(targetPos);


			// 目標付近（m_stopDistance + HYSTERESIS 以内）にいて、かつ物理的にほぼ停止しているなら、
			// 目標にピッタリ到達していなくても強制的に Stop フェーズにする。
			if (m_movePhase != MovePhase::Stop && distToTarget <= m_stopDistance + HYSTERESIS)
			{
				const Vector3& currentVel = m_stateMachine->GetCurrentVelocity();
				if (currentVel.LengthSq() < STOP_VELOCITY_THRESHOLD_SQ) // 速度がほぼゼロ
				{
					m_movePhase = MovePhase::Stop;
				}
			}


			/**
			 * フェーズ遷移。
			 * 遠距離の移動手段は距離ではなく「足元の傾斜」で選ぶ：
			 *   下り坂 → スライド（傾斜ゲインで加速して速い）
			 *   上り坂 → 歩き（スライドはずり落ち、確実に登れる歩幅で）
			 *   平地   → 走り
			 * 目標付近の減速（Walk→Stop）だけは従来どおり距離と速度で判断する。
			 * 傾斜は出入りで別しきい値のヒステリシスを持たせ、境界でばたつかないようにする。
			 */
			if (m_movePhase == MovePhase::Stop)
			{
				if (distToTarget > m_stopDistance + STOP_EXIT_MARGIN)
				{
					/** まず歩きで動き出し、次フレーム以降は傾斜で選び直す */
					m_movePhase = MovePhase::Walk;
				}
			}
			else if (distToTarget <= m_walkDistance - HYSTERESIS
				|| (m_movePhase == MovePhase::Walk
					&& distToTarget <= m_walkDistance + PHASE_UP_MARGIN))
			{
				/** 目標付近では傾斜に関係なく Walk へ落として減速する
				 *  （Stop への遷移はこの上の距離＋速度の判定が行う）。
				 *  入りは walkDistance-H、出は walkDistance+MARGIN の
				 *  ヒステリシスにしないと、境界上で Walk/Run が毎フレーム反転して
				 *  所定位置に着く直前の子がカクカクする */
				m_movePhase = MovePhase::Walk;
			}
			else
			{
				const float slope = m_stateMachine->GetSlideSlopeSigned();
				const bool wantSlide = (m_movePhase == MovePhase::Slide)
					? (slope > CHILD_SLOPE_SLIDE_EXIT)
					: (slope > CHILD_SLOPE_SLIDE_ENTER);
				const bool wantWalk = (m_movePhase == MovePhase::Walk)
					? (slope < CHILD_SLOPE_WALK_EXIT)
					: (slope < CHILD_SLOPE_WALK_ENTER);

				if (wantSlide)     { m_movePhase = MovePhase::Slide; }
				else if (wantWalk) { m_movePhase = MovePhase::Walk; }
				else               { m_movePhase = MovePhase::Run; }
			}

			/**
			 * 移動方向（常に正規化済みベクトルを渡す）
			 *
			 * Move() は moveDirection * (moveSpeed * speedMultiplier) で目標速度を計算するため、
			 * moveDirection のスケール（長さ）を変えると本来の速度設定が崩れてしまう。
			 * 基本速度の調整は SetMoveSpeed()、目標手前での減速やブレーキは SetSpeedMultiplier() 経由で行う。
			 */
			const Vector3 moveDirection = CalculateMoveDirectionWithNav(targetPos, distToTarget);

			float speedMultiplier = 1.0f;

			// 目標までの距離に完全比例で減速する（停止距離の2倍で全速→目標中心で0のバネ）。
			// 旧実装は「停止距離の内側=全速」だったため、親が止まってスロットが足元に来た子や、
			// Y再集合の一斉入隊でスロットが数ユニット動いた子が、目標点を全速で
			// 往復し続けてカクつく（2026-08-26 実バグ）。
			// 倍率0固定は渦潮吸引に無抵抗になるため不可（同日ボット実測でHard飲まれ激増）。
			// 距離比例なら静止時はそっと着地し、流されたら距離に応じて踏ん張る
			if (m_stopDistance > 0.0f && m_movePhase != MovePhase::Stop)
			{
				const float brakeRange = m_stopDistance * 2.0f;
				speedMultiplier = min(distToTarget / brakeRange, 1.0f);
			}

			// 計算した倍率をステートマシンに渡し、物理処理(Lerp)の目標速度を落とす
			m_stateMachine->SetSpeedMultiplier(speedMultiplier);

			// 陣形のパッシブ/ウルト速度倍率を反映する（1.0f超もありうるためクランプなしの別枠に渡す）
			m_stateMachine->SetExternalSpeedMultiplier(ChildPenguinManager::GetInstance()->GetFormationSpeedMultiplier());

			/** フェーズに応じてAIControllerInputを組み立てる */
			switch (m_movePhase)
			{
			case MovePhase::Stop:
				/** 停止 */
				// 目標座標の近くで自然に止まるため、向きの調整は不要
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				break;

			case MovePhase::Walk:
				/** 歩き：isSneak=true, isDash=false, isSlide=false */
				m_stateMachine->SetActionInput(moveDirection, true, false, false, false);
				break;

			case MovePhase::Run:
				/** 走り：isSneak=false, isDash=true, isSlide=false */
				m_stateMachine->SetActionInput(moveDirection, false, true, false, false);
				break;

			case MovePhase::Slide:
				/**
				 * 滑り：isSneak=false, isDash=true, isSlide=true。
				 * ただし上り坂では滑らず走る。傾斜モデルの強化（GAIN_UP 3.5）で
				 * 上りのスライドは走りより大幅に遅くなるため、AIには選ばせない
				 */
				if (m_stateMachine->GetSlideSlopeSigned() < CHILD_SLIDE_UPHILL_LIMIT)
				{
					m_stateMachine->SetActionInput(moveDirection, false, true, false, false);
				}
				else
				{
					m_stateMachine->SetActionInput(moveDirection, false, true, false, true);
				}
				break;
			}
		}


		void ChildPenguinAIController::UpdateIglooEvent()
		{
			if (m_isInsideIgloo)
			{
				// ★ リーダーの助言通り、かまくらの中にいる間は「毎フレーム」座標をセットし続ける
				m_owner->GetCharacterController()->SetPosition(m_iglooTargetPos);
				m_owner->GetCharacterController()->RequestTeleport();

				// ★ 渦潮の処理と同じように、ステートマシン（モデルの見た目位置）も毎フレーム同期する！
				// これがないと見た目だけ屋根の上に押し出されてしまいます
				m_stateMachine->SetPosition(m_iglooTargetPos);

				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			Vector3 myPos = m_owner->GetTransform().m_position;

			Vector3 dirToTarget = m_iglooTargetPos - myPos;
			dirToTarget.y = 0.0f;

			// 定数化して判定を明確にする
			constexpr float ENTER_DISTANCE = 150.0f;

			// 青い円に十分近づいたらワープ発動
			if (dirToTarget.Length() < ENTER_DISTANCE)
			{
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);

				// 子ペンギンの現在位置を基準に最も近いイグルーの中心座標を取得
				Vector3 iglooPos = StageSystem::GetInstance()->GetNearestIglooPosition(myPos);
				Vector3 insidePos = iglooPos;

				constexpr float IGLOO_INSIDE_CIRCLE = 360.0f;
				constexpr float IGLOO_INSIDE_RADIUS = 60.0f; // かまくらの中に収まる半径


				const float angle = util::RandomDevice::Random(0.0f, IGLOO_INSIDE_CIRCLE);
				const float radius = util::RandomDevice::Random(0.0f, IGLOO_INSIDE_RADIUS);

				constexpr float IGLOO_INSIDE_HALF_CIRCLE = 180.0f;

				float angleRad = angle * (Math::PI / IGLOO_INSIDE_HALF_CIRCLE);
				float r = radius;

				insidePos.x += r * cosf(angleRad);
				insidePos.z += r * sinf(angleRad);

				m_iglooTargetPos = insidePos;
				m_owner->SetIglooFixedPos(m_iglooTargetPos);
				m_owner->SetInsideIgloo(true);

				// 最初のワープ時も両方の座標をセットする
				m_owner->GetCharacterController()->SetPosition(m_iglooTargetPos);
				m_owner->GetCharacterController()->RequestTeleport();
				m_stateMachine->SetPosition(m_iglooTargetPos);

				// 隊列リストからの離脱
				if (m_isFollowing)
				{
					ChildPenguinManager::GetInstance()->RemoveFollower(m_owner);
					m_isFollowing = false;
				}

				m_isInsideIgloo = true;


				// =========================================================
				// ★ かまくらマネージャーにこの子ペンギンを格納（登録）する！
				// =========================================================
				IglooManager::GetInstance().AddPenguin(m_owner);

				// マネージャーに入室完了を報告（イベント終了判定用）
				ChildPenguinManager::GetInstance()->FinishEnterIglooOne();
			}
			else
			{
				// まだ遠い場合は青い円に向かって歩く
				if (dirToTarget.LengthSq() > FLT_EPSILON)
				{
					dirToTarget.Normalize();
				}
				m_stateMachine->SetActionInput(dirToTarget, true, false, false, false);
			}
		}


		void ChildPenguinAIController::EndEnterIglooEvent(const Vector3& exitPos)
		{
			// 1. 各種イベントフラグを解除（これで通常の追従AIに戻る）
			m_isEnterIglooMode = false;
			m_isInsideIgloo = false;
			m_owner->SetInsideIgloo(false);

			// 2. 出現座標を少しばらけさせる
			Vector3 spawnPos = exitPos;
			spawnPos.x += (float)(std::rand() % 60) - 30.0f;
			spawnPos.z += (float)(std::rand() % 60) - 30.0f;

			// 3. キャラクターコントローラーとステートマシン両方をワープ！
			m_owner->GetCharacterController()->SetPosition(spawnPos);
			m_stateMachine->SetPosition(spawnPos);
		}


		bool ChildPenguinAIController::IsDaddyWithinJoinRadius() const
		{
			return GetDistanceToDaddy() <= ChildPenguinManager::GetInstance()->GetJoinRadius();
		}


		bool ChildPenguinAIController::CanJoinFormation() const
		{
			/** 逃げた直後は隊列へ戻さない（散開に「集め直す時間」を持たせるため） */
			if (m_rejoinCooldown > 0.0f) return false;

			/**
			 * 入隊条件は「親を察知している」のみ（2026-08-23 試遊フィードバック）。
			 *
			 * 以前は「隊列参加距離の内側にいる」も条件で、察知した子が寄ってきて
			 * 距離内に入った時点で入隊していたが、視界で付いてくるようになったため
			 * 「見つけた瞬間に♪を出して入隊カウント」へ簡略化した。
			 * 入隊した子は陣形コントローラーの隊列スロットへ自分で移動してくる。
			 *
			 * 逃走後の復帰猶予（m_rejoinCooldown）はこの関数の頭で効いているので、
			 * クマに散らされた子がその場で即復帰することはない
			 */
			return m_hasNoticedDaddy;
		}


		bool ChildPenguinAIController::TryJoinFormation()
		{
			/** すでに隊列にいるなら何もしない */
			if (m_isFollowing) return true;

			if (!CanJoinFormation()) return false;

			ChildPenguinManager::GetInstance()->AddFollower(m_owner);
			m_isFollowing = true;
			return true;
		}


		void ChildPenguinAIController::BuildInputWhenNotFollowing()
		{
			auto* manager = ChildPenguinManager::GetInstance();

			/**
			 * 待機命令中は気づいていても動かない。
			 * 「待て」と言われた子が勝手に寄ってきては待機命令の意味が無くなる
			 */
			const bool isFollowCmd =
				manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			if (m_hasNoticedDaddy && isFollowCmd)
			{
				/** 親に気づいたので自分から寄っていく */
				m_stateMachine->SetTurnSpeedMultiplier(1.0f);
				BuildInputToTarget(manager->GetDaddyPosition());
				return;
			}

			if (m_hasHeardDaddy && isFollowCmd)
			{
				/**
				 * 音だけ聞こえている：その場でゆっくり親のほうへ振り向く。
				 * 振り向いた結果、親が視界に入れば UpdatePerception() が察知（「！」）へ進める。
				 * 速度は0にして動かず、旋回だけを遅くする
				 */
				const Vector3 toDaddy = CalculateDirectionToDaddy();
				if (toDaddy.LengthSq() > FLT_EPSILON)
				{
					m_stateMachine->SetSpeedMultiplier(0.0f);
					m_stateMachine->SetTurnSpeedMultiplier(INVESTIGATE_TURN_MULTIPLIER);
					m_stateMachine->SetActionInput(toDaddy, true, false, false, false);
					return;
				}
			}

			/** 気づいていなければその場で待つ */
			m_stateMachine->SetTurnSpeedMultiplier(1.0f);
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
		}


		void ChildPenguinAIController::UpdatePerception()
		{
			/**
			 * 旋回倍率を毎フレーム既定値へ戻す（AIの経路によらず必ず戻す）。
			 * 振り向き（？）中だけ BuildInputWhenNotFollowing() がこの後で 0.2 に落とす。
			 * ここで戻さないと、振り向き中に徘徊などへ移ったタイプが遅い旋回のまま動き続ける
			 */
			m_stateMachine->SetTurnSpeedMultiplier(1.0f);

			/** 隊列へ戻れるようになるまでの猶予を進める（AIの経路によらず必ず進める） */
			if (m_rejoinCooldown > 0.0f)
			{
				m_rejoinCooldown -= g_gameTime->GetFrameDeltaTime();
			}

			/** 呼びかけに応えた子の勇敢タイマーを進める */
			if (m_braveTimer > 0.0f)
			{
				m_braveTimer -= g_gameTime->GetFrameDeltaTime();
			}

			/** 隊列にいる間は当然親を察知している（判定そのものを省ける） */
			if (m_isFollowing)
			{
				m_hasNoticedDaddy = true;
				m_hasHeardDaddy = false;
				m_noticeTimer = m_perceptionSpec->noticeTime;
				m_forgetTimer = 0.0f;
				return;
			}

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			/**
			 * 察知は2段階（2026-08-23 試遊フィードバック）。
			 *   音が聞こえる  → 「？」を出してゆっくり振り向く（m_hasHeardDaddy）
			 *   視界に入った  → 「！」を出して察知、自分から寄っていく（m_hasNoticedDaddy）
			 * 至近距離と再集合の呼びかけだけは段階を踏まず即座に察知する
			 * （この保険が無いと「親に背を向けた子が永久に入隊できない」が起きる）
			 */
			if (PerceiveDaddyStrongThisFrame())
			{
				m_forgetTimer = 0.0f;
				NoticeDaddy();
				return;
			}

			if (PerceiveDaddySightThisFrame())
			{
				m_forgetTimer = 0.0f;
				m_noticeTimer += deltaTime;

				/** 視界に入り続けた時間が「気づく速さ」を超えたら察知に変わる */
				if (m_noticeTimer >= m_perceptionSpec->noticeTime)
				{
					NoticeDaddy();
				}
				return;
			}

			m_noticeTimer = 0.0f;

			if (PerceiveDaddySoundThisFrame())
			{
				m_forgetTimer = 0.0f;

				/** 音だけでは察知にならない。「聞こえた」段階に入って振り向き始める */
				if (!m_hasNoticedDaddy && !m_hasHeardDaddy)
				{
					m_hasHeardDaddy = true;
					BattleManager::GetInstance().NotifyCPReactionChanged(
						m_owner, ui::EnCPReactionType::Question);
				}
				return;
			}

			/** 何も知覚できない：察知も「聞こえた」も時間経過で忘れる */
			if (!m_hasNoticedDaddy && !m_hasHeardDaddy) return;

			m_forgetTimer += deltaTime;
			if (m_forgetTimer >= NOTICE_FORGET_TIME)
			{
				m_hasNoticedDaddy = false;
				m_hasHeardDaddy = false;
				m_forgetTimer = 0.0f;
			}
		}


		void ChildPenguinAIController::NoticeDaddy()
		{
			m_noticeTimer = m_perceptionSpec->noticeTime;

			/**
			 * 「！」マークは出さない。察知＝入隊なので、入隊の♪吹き出しが
			 * そのまま「見つけた」の合図になる（2026-08-23 試遊フィードバック）。
			 * 「？」を出していた場合は見つけた瞬間に消す
			 */
			if (m_hasHeardDaddy)
			{
				BattleManager::GetInstance().NotifyCPReactionChanged(
					m_owner, ui::EnCPReactionType::None);
			}
			m_hasNoticedDaddy = true;
			m_hasHeardDaddy = false;
		}


		bool ChildPenguinAIController::PerceiveDaddyStrongThisFrame()
		{
			auto* manager = ChildPenguinManager::GetInstance();
			const float distDaddy = GetDistanceToDaddy();

			/**
			 * 1. 至近距離：隊列参加距離の内側なら向きも遮蔽も問わず気づく。
			 *    すぐ目の前にいる親を見落とすことは無い
			 */
			if (distDaddy <= manager->GetJoinRadius()) return true;

			/**
			 * 2. 再集合の呼びかけ（Yボタン）：親が大声で呼んでいる間は、
			 *    向きも遮蔽も耳のよさも問わず気づく。
			 *    散った群れを集め直すための手段なので、ここで性格差は付けない
			 */
			if (manager->IsRegroupCallActive()
				&& distDaddy <= manager->GetRegroupCallRadius())
			{
				return true;
			}

			return false;
		}


		bool ChildPenguinAIController::PerceiveDaddySoundThisFrame()
		{
			auto* manager = ChildPenguinManager::GetInstance();

			/** 親が立てている音が届いているか（向きも遮蔽も問わない） */
			const float hearingRadius = manager->GetDaddyNoiseRadius() * m_perceptionSpec->hearingScale;
			return GetDistanceToDaddy() <= hearingRadius;
		}


		bool ChildPenguinAIController::PerceiveDaddySightThisFrame()
		{
			/** 距離 → 向き → 遮蔽 の順に、軽い判定から足切りする */
			if (GetDistanceToDaddy() > m_perceptionSpec->sightDistance) return false;

			/** 向き：自分の正面ベクトルと親への方向の角度を見る */
			Vector3 forward = Vector3::Front;
			m_owner->GetTransform().m_rotation.Apply(forward);
			forward.y = 0.0f;
			if (forward.LengthSq() <= FLT_EPSILON) return false;
			forward.Normalize();

			const Vector3 toDaddy = CalculateDirectionToDaddy();
			if (toDaddy.LengthSq() <= FLT_EPSILON) return false;

			const float cosHalfAngle = cosf(Math::DegToRad(m_perceptionSpec->sightHalfAngleDeg));
			if (forward.Dot(toDaddy) < cosHalfAngle) return false;

			/** 遮蔽：ここまで通った子だけがレイキャストの対象になる */
			return !IsDaddyOccluded();
		}


		bool ChildPenguinAIController::IsDaddyOccluded()
		{
			auto* manager = ChildPenguinManager::GetInstance();

			/**
			 * レイキャストは OCCLUSION_CHECK_INTERVAL フレームに1回だけ。
			 * 子ごとにスロットをずらしてあるので、負荷はフレーム間で平らになる
			 */
			const unsigned int frame = manager->GetPerceptionFrame() + m_perceptionSlot;
			if (frame % OCCLUSION_CHECK_INTERVAL != 0)
			{
				return m_isDaddyOccludedCache;
			}

			Vector3 eyePos = m_owner->GetTransform().m_position;
			eyePos.y += PERCEPTION_EYE_HEIGHT;

			Vector3 daddyPos = manager->GetDaddyPosition();
			daddyPos.y += PERCEPTION_EYE_HEIGHT;

			/**
			 * 地形だけを遮蔽物として扱う。
			 * フィルタを掛けずに撃つと、間に立っている別の子ペンギンが最近接ヒットになり
			 * 地形の判定にならない。フィルタはブロードフェーズで効くのでコストも下がる
			 */
			nsBeastEngine::nsCollision::RaycastHit hit;
			m_isDaddyOccludedCache = nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(
				eyePos,
				daddyPos,
				hit,
				nsBeastEngine::nsCollision::ALL_COLLISION_ATTRIBUTE_MASK,
				[](const btCollisionObject& obj)
				{
					return (obj.getUserIndex() & nsBeastEngine::nsCollision::CollisionAttribute::Ground) != 0;
				});

			return m_isDaddyOccludedCache;
		}


		bool ChildPenguinAIController::CheckAndFlee()
		{
			auto* manager = ChildPenguinManager::GetInstance();
			const Vector3 myPos = m_owner->GetTransform().m_position;

			// 「自分を追っているクマ」ではなく「誰かを追っているクマ」を見る。
			// クマが群れに突っ込んだとき、狙われた1体だけでなく周りの子もまとめて散る。
			// これが無いと隊列人数に「群れごと失う」谷が出ない
			Vector3 threatPos;
			bool hasThreat = manager->FindNearestBearThreat(myPos, FLEE_DETECTION_DISTANCE, threatPos);

			// ウルト発動中の隊列の子は逃げない（全陣形共通）。
			// ウルト中に群れが散ると、せっかくの強化時間が集め直しに消えて
			// 爽快感よりストレスが勝つ（2026-08-24 試遊フィードバック）。
			// 密集陣では逃げないことが攻撃無効化＋反撃の前提でもある
			// （逃走は RemoveFollower で隊列から抜けるため、逃げると無効化の対象外になる）。
			// 隊列外の子はウルトの恩恵を受けていないので、今までどおり逃げる
			if (m_isFollowing && manager->IsUltActive())
			{
				m_fleeHoldTimer = 0.0f;
				if (m_isFleeing)
				{
					m_isFleeing = false;
					if (auto* lm = GameLogManager::GetInstance())
					{
						lm->QueueEvent({
							{"ev", "child_flee_end"},
							{"penguin_id", m_owner->GetLogId()}
						});
					}
				}
				return false;
			}

			// 再集合の呼びかけ（Yボタン）中は、クマが近くにいても「クマから逃げる」のをやめて
			// 親のもとへ逃げ込ませる。以前は「クマの近くの子には呼びかけが効かない」仕様だったが、
			// クマがうろついている間は再集合がまったく機能しなかった（2026-08-23 試遊フィードバック）。
			// ただし親自身がクマの至近にいる場合だけは、クマの口元へ呼び戻すことになるので効かせない
			if (manager->IsRegroupCallActive()
				&& GetDistanceToDaddy() <= manager->GetRegroupCallRadius())
			{
				Vector3 dummyPos;
				const bool isDaddyInDanger = manager->FindNearestBearThreat(
					manager->GetDaddyPosition(), REGROUP_DADDY_SAFE_DISTANCE, dummyPos);

				if (!isDaddyInDanger)
				{
					// 逃走を打ち切り、察知経路（呼びかけ）で親へ向かわせる。
					// 復帰の猶予も0にして、親に着いたらすぐ隊列へ入れるようにする。
					// 呼びかけの効果（2秒）が切れた直後にまた散らないよう、しばらく勇敢にする
					m_fleeHoldTimer = 0.0f;
					m_rejoinCooldown = 0.0f;
					m_braveTimer = REGROUP_BRAVE_TIME;
					if (m_isFleeing)
					{
						m_isFleeing = false;
						if (auto* lm = GameLogManager::GetInstance())
						{
							lm->QueueEvent({
								{"ev", "child_flee_end"},
								{"penguin_id", m_owner->GetLogId()}
							});
						}
					}
					return false;
				}
			}

			// 呼びかけに応えた直後の子は勇敢：クマが「他の子」を追っているだけなら逃げない。
			// これが無いと、呼びかけで集めてもクマが300以内にいる限り2秒後にまた散ってしまう。
			// 自分が狙われているとき・クマが至近まで来たときは今までどおり逃げる
			if (hasThreat && m_braveTimer > 0.0f)
			{
				Vector3 chaserPos;
				bool isChasedMyself = false;
				if (auto* enemyManager = EnemyManager::GetInstance())
				{
					isChasedMyself = enemyManager->FindNearestChaserOf(m_owner, chaserPos)
						&& (chaserPos - myPos).LengthSq()
							<= FLEE_DETECTION_DISTANCE * FLEE_DETECTION_DISTANCE;
				}

				Vector3 toThreat = threatPos - myPos;
				toThreat.y = 0.0f;
				const bool isThreatVeryClose =
					toThreat.LengthSq() <= BRAVE_PANIC_DISTANCE * BRAVE_PANIC_DISTANCE;

				if (!isChasedMyself && !isThreatVeryClose)
				{
					hasThreat = false;
				}
			}

			if (hasThreat)
			{
				// クマが目の前にいる間は逃げ続ける
				m_fleeHoldTimer = FLEE_HOLD_TIME;
				m_lastThreatPos = threatPos;
			}
			else
			{
				// クマが離れてもすぐには落ち着かない。この時間だけ逃げ続ける
				if (m_fleeHoldTimer > 0.0f)
				{
					m_fleeHoldTimer -= g_gameTime->GetFrameDeltaTime();
				}

				// 親の呼びかけが届いていればパニックを打ち切って戻れるようにする
				if (manager->IsRegroupCallActive()
					&& GetDistanceToDaddy() <= manager->GetRegroupCallRadius())
				{
					m_fleeHoldTimer = 0.0f;
					m_rejoinCooldown = 0.0f;
				}
			}

			if (m_fleeHoldTimer <= 0.0f)
			{
				// 逃げ終わった：状態をリセットして次回の逃走に備える
				if (m_isFleeing)
				{
					m_isFleeing = false;
					if (auto* lm = GameLogManager::GetInstance())
					{
						lm->QueueEvent({
							{"ev", "child_flee_end"},
							{"penguin_id", m_owner->GetLogId()}
						});
					}
				}
				m_fleeDirChangeTimer = 0.0f;
				m_fleeAngleOffset = 0.0f;
				return false;
			}

			// ここから逃走行動。
			//
			// 以前はここに「親が隊列参加距離以内にいる場合は逃走より隊列を優先する」
			// という分岐があり、隊列にいる子はクマに追われても逃げなかった。
			// 新ステージではクマの被害が 4〜5 件から 11 件に増えて
			// この分岐が頻繁に踏まれるようになり、
			// 「追われても誰も逃げずそのまま食べられる絵」になっていたため反転した
			// （プレイログ計測基盤と実測結果 7-1 / ステージ再設計_設計仕様 10節）。
			const bool wasFollower = m_isFollowing;
			if (m_isFollowing)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			// 逃げ終わってもすぐには隊列へ戻さない。ここが「集め直す時間」の長さを決めるつまみ。
			// 逃げている間ずっと張り直すことで、逃げ終わった瞬間から数え始める
			// （1回だけ入れると、長く逃げたぶんだけ猶予が食い潰されてしまう）
			m_rejoinCooldown = FLEE_REJOIN_COOLDOWN;

			if (!m_isFleeing)
			{
				m_isFleeing = true;
				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->QueueEvent({
						{"ev", "child_flee_start"},
						{"penguin_id", m_owner->GetLogId()},
						{"penguin_type", m_owner->GetChildPenguinTypeStr()},
						{"was_follower", wasFollower}
					});
				}
			}

			Vector3 fleeDir = myPos - m_lastThreatPos;
			fleeDir.y = 0.0f;

			if (fleeDir.LengthSq() > FLT_EPSILON)
			{
				fleeDir.Normalize();
			}

			// ── 逃走方向の揺らぎ（たまに横方向へ逃げる）──
			// タイマーがゼロ以下になるたびに方向を再抽選する
			m_fleeDirChangeTimer -= g_gameTime->GetFrameDeltaTime();
			if (m_fleeDirChangeTimer <= 0.0f)
			{
				// 次の方向変更まで 0.5～1.5 秒保持する
				const float newInterval = util::RandomDevice::Random(FLEE_DIR_HOLD_MIN, FLEE_DIR_HOLD_MAX);
				m_fleeDirChangeTimer = newInterval;

				if (RollUnit() >= FLEE_STRAIGHT_CHANCE)
				{
					// 30%の確率で横方向（45～90度）へ回避する
					const float angle = util::RandomDevice::Random(FLEE_DODGE_ANGLE_MIN, FLEE_DODGE_ANGLE_MAX);
					const float sign = (RollUnit() < FLEE_SIGN_FLIP_CHANCE) ? 1.0f : -1.0f;
					m_fleeAngleOffset = angle * sign;
				}
				else
				{
					// 70%の確率で直進
					m_fleeAngleOffset = 0.0f;
				}
			}

			// オフセット角度を X-Z 平面上で flee 方向に適用（Y軸回転）
			if (fabsf(m_fleeAngleOffset) > FLT_EPSILON)
			{
				const float cosA = cosf(m_fleeAngleOffset);
				const float sinA = sinf(m_fleeAngleOffset);
				const float newX = fleeDir.x * cosA - fleeDir.z * sinA;
				const float newZ = fleeDir.x * sinA + fleeDir.z * cosA;
				fleeDir.x = newX;
				fleeDir.z = newZ;
			}

			// エネミーから離れる方向へダッシュ。
			// 振り向き（？）中に落としていた速度・旋回の倍率が残らないよう明示的に戻す
			m_stateMachine->SetSpeedMultiplier(1.0f);
			m_stateMachine->SetTurnSpeedMultiplier(1.0f);
			m_stateMachine->SetActionInput(fleeDir, false, true, false, false);
			return true;
		}


		void ChildPenguinAIController::ForceEjectFromIgloo(const Vector3& iglooPos)
		{
			// 1. 各種イベントフラグを強制解除（通常のAI処理に戻すため）
			m_isEnterIglooMode = false;
			m_isInsideIgloo = false;
			m_owner->SetInsideIgloo(false);

			constexpr float EJECT_OFFSET_RANGE = 60.0f; // かまくらの中心から弾き出される範囲（半径）
			constexpr float EJECT_UP_OFFSET = 50.0f; // 弾き出される際の上方向のオフセット

			// 2. 弾き出される座標をランダムに散らす
			// かまくらの中心(iglooPos)を基準に、周囲にランダムに配置します
			const float offset = util::RandomDevice::Random(-EJECT_OFFSET_RANGE, EJECT_OFFSET_RANGE);

			Vector3 spawnPos = iglooPos;
			spawnPos.x += offset;
			spawnPos.z += offset;

			// 少し上空から落とすことで、弾き飛ばされた感を演出します
			//spawnPos.y +=EJECT_UP_OFFSET;

			// 3. キャラクターコントローラーとステートマシン両方をワープ
			m_owner->GetCharacterController()->SetPosition(spawnPos);
			m_owner->GetCharacterController()->RequestTeleport(); // ワープを確定
			m_stateMachine->SetPosition(spawnPos);

			// 4. 強制的に空中に放り出された判定にするため、入力をゼロにする
			// （これによって自動的にジャンプ・落下ステート等へ遷移します）
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
		}



		/**************************************************************/


		//--------------------------------------------------------------
		// SeriousChildPenguinAI（まじめペンギン）
		//--------------------------------------------------------------

		SeriousChildPenguinAI::SeriousChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Serious)
		{}


		void SeriousChildPenguinAI::UpdateAI()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}

			/** シロクマ逃走チェック（かまくら > 逃走 > 通常AI の優先順） */
			if (CheckAndFlee()) return;

			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();

			/** 渦潮に飲まれている間は隊を抜けて入力をゼロにする */
			if (m_owner->GetStateMachine()->GetIsInWhirlpool())
			{
				if (m_isFollowing) { m_isFollowing = false; }
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			/** 待機命令のとき */
			if (manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Wait)
			{
				/** 隊列から離脱 */
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				/** その場で待機 */
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			/** 入隊判定（条件は CanJoinFormation() に集約してある） */
			/** 隊列に参加できていなければ、親に気づいていれば寄っていき、いなければ待つ */
			if (!TryJoinFormation()) {
				BuildInputWhenNotFollowing();
				return;
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// ClingyChildPenguinAI（甘えん坊ペンギン）
		//--------------------------------------------------------------

		ClingyChildPenguinAI::ClingyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Clingy)
			, m_clingyEffectHandle(INVALID_EFFECT_HANDLE)
		{}


		ClingyChildPenguinAI::~ClingyChildPenguinAI()
		{
			if (m_clingyEffectHandle != INVALID_EFFECT_HANDLE)
			{
				EffectManager::Get().StopEffect(m_clingyEffectHandle);
			}
		}


		void ClingyChildPenguinAI::SetRestrained(const bool isRestrained)
		{
			// 制止された瞬間(false→true)だけリアクションを要求する
			// （ApplyIntervention()は介入中毎フレーム呼ばれるため、ここでエッジ検出する）
			if (isRestrained && !m_isRestrained)
			{
				BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Trouble, ui::EnCPReactionPriority::High);
			}

			m_isRestrained = isRestrained;
		}


		void ClingyChildPenguinAI::UpdateAI()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}

			/** シロクマ逃走チェック（かまくら > 逃走 > 通常AI の優先順） */
			if (CheckAndFlee()) return;

			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 渦潮に飲まれている間は隊を抜けて入力をゼロにする */
			if (m_owner->GetStateMachine()->GetIsInWhirlpool())
			{
				if (m_isFollowing) { m_isFollowing = false; }
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			// エフェクトの再生と位置更新を行うラムダ。
			auto updateClingyEffect = [&]()
				{
					// 計測時間。
					m_effectInterval += g_gameTime->GetFrameDeltaTime();

					// 1秒を超えたらエフェクトを再生。
					if (m_effectInterval > 1.0f)
					{
						// 古いエフェクトを停止してから新しいものを生成する（取り残し防止）。
						if (m_clingyEffectHandle != INVALID_EFFECT_HANDLE)
						{
							EffectManager::Get().StopEffect(m_clingyEffectHandle);
							m_clingyEffectHandle = INVALID_EFFECT_HANDLE;
						}

						const Vector3 hartPos = m_owner->GetTransform().m_position + CLINGY_HART_EFFECT_POSITION;
						const Vector3 hartScl = m_owner->GetTransform().m_scale + CLINGY_HART_EFFECT_SCALE;
						m_clingyEffectHandle = EffectManager::Get().PlayEffect(
							EnEffectKind::ClingyPenguinHart
							, hartPos
							, Quaternion::Identity
							, hartScl
						);
						// キャラクターに追従させる。
						EffectManager::Get().AttachEffect(
							m_clingyEffectHandle,
							&m_owner->GetTransform().m_position,
							CLINGY_HART_EFFECT_POSITION
						);
						// インターバルをリセット。
						m_effectInterval = 0.0f;
					}
				};

			// エフェクトを停止するラムダ。
			auto stopEffect = [&]()
				{
					if (m_clingyEffectHandle != INVALID_EFFECT_HANDLE)
					{
						// エフェクトを停止。
						EffectManager::Get().StopEffect(m_clingyEffectHandle);
						// ハンドルを無効化。
						m_clingyEffectHandle = INVALID_EFFECT_HANDLE;
						// インターバルをリセット。
						m_effectInterval = 0.0f;
					}
				};

			/** 追従命令のとき：制止・登録を解除して通常追従する */
			if (isFollowCmd)
			{
				m_isRestrained = false;
				manager->UnregisterAttempting(m_owner);

				/** 入隊判定（条件は CanJoinFormation() に集約してある） */
				if (!TryJoinFormation())
				{
					BuildInputWhenNotFollowing();
					return;
				}

				updateClingyEffect();

				BuildInput();
				return;
			}

			/** 待機命令のとき */

			/** 世話焼きペンギンに制止されているときはその場で待機する */
			if (m_isRestrained)
			{
				/** 制止中は追従しようとしている登録を解除する */
				manager->UnregisterAttempting(m_owner);
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				stopEffect();
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			/** 待機命令中に追従しようとしていることをManagerに登録する */
			manager->RegisterAttempting(m_owner);

			/** 入隊判定（条件は CanJoinFormation() に集約してある） */
			/** 待機命令中なので寄ってはいかず、その場で待つ */
			if (!TryJoinFormation())
			{
				BuildInputWhenNotFollowing();
				return;
			}

			// エフェクト再生。
			if (m_isFollowing && !m_isRestrained)
			{
				updateClingyEffect();
			}

			// エフェクト停止。
			else
			{
				/** 追従から外れた、または制止された場合はエフェクトを即座に停止する */
				if (m_clingyEffectHandle != INVALID_EFFECT_HANDLE)
				{
					stopEffect();
				}
			}

			/** 隊列参加中：距離だけで移動手段を決定する */
			BuildInput();
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// NaughtyChildPenguinAI（やんちゃペンギン）
		//--------------------------------------------------------------

		NaughtyChildPenguinAI::NaughtyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Naughty)
			, m_naughtyStateMachine(static_cast<NaughtyChildPenguinStateMachine*>(owner->GetStateMachine()))
		{
			const auto& td = GetTypeData(EnChildPenguinType::Naughty);
			m_roamTriggerDistance = RollRange(td.roamTriggerDistance);
			m_roamRadius = RollRange(td.roamRadius);
		}


		NaughtyChildPenguinAI::~NaughtyChildPenguinAI()
		{
			StopLivelyEffect();
		}


		bool NaughtyChildPenguinAI::IsHeadingIntoWhirlpoolOnPurpose() const
		{
			return m_naughtyStateMachine != nullptr
				&& m_naughtyStateMachine->GetIsGoingToWhirlpool();
		}


		void NaughtyChildPenguinAI::SetRestrained(const bool isRestrained)
		{
			// 制止された瞬間(false→true)だけリアクションを要求する
			// （ApplyIntervention()は介入中毎フレーム呼ばれるため、ここでエッジ検出する）
			if (isRestrained && !m_isRestrained)
			{
				BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Trouble, ui::EnCPReactionPriority::High);
			}

			m_isRestrained = isRestrained;
		}


		void NaughtyChildPenguinAI::UpdateAI()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}

			/** シロクマ逃走チェック（かまくら > 逃走 > 通常AI の優先順） */
			if (CheckAndFlee()) return;

			/** 急斜面などで動けなくなっていたら行き先を変える */
			UpdateStuckWatch();

			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 親との距離を取得 */
			const float distDaddy = GetDistanceToDaddy();

			/** 意図せず渦潮に飲まれている間は隊を抜けて入力をゼロにする
			 *  （意図的な場合は後続の GetIsGoingToWhirlpool() ブロックで処理） */
			if (m_owner->GetStateMachine()->GetIsInWhirlpool() && !m_naughtyStateMachine->GetIsGoingToWhirlpool())
			{
				if (m_isFollowing) { m_isFollowing = false; }
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			/** 世話焼きペンギンに制止されているときはその場で待機する */
			/** （命令に関わらず最優先で制止を適用する） */
			if (m_isRestrained)
			{
				// 制止されたらシロクマや渦潮に向かう行動もやめる
				if (m_naughtyStateMachine->GetIsGoingToWakeBear() || m_naughtyStateMachine->GetIsGoingToWhirlpool())
				{
					m_naughtyStateMachine->SetIsGoingToWakeBear(false);
					m_naughtyStateMachine->SetIsAtBear(false);

					m_naughtyStateMachine->SetIsGoingToWhirlpool(false);
					m_naughtyStateMachine->SetIsAtWhirlpool(false);

					m_wasSwallowedByWhirlpool = false;

					manager->UnregisterAttempting(m_owner);
					m_scoldCooldown = SCOLD_COOLDOWN_DURATION;

					StopLivelyEffect();
				}

				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				if (manager->IsRoaming(m_owner))
				{
					manager->UnregisterRoaming(m_owner);
				}

				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			// シロクマを起こし終わった時
			if (m_naughtyStateMachine->GetHasFinishedWaking())
			{
				m_naughtyStateMachine->SetHasFinishedWaking(false);
				manager->UnregisterAttempting(m_owner); // 問題行動リストから外れる
				m_scoldCooldown = SCOLD_COOLDOWN_DURATION; // SCOLD_COOLDOWN_DURATION秒間は満足してシロクマを無視する
				StopLivelyEffect();
				manager->AddFollower(m_owner);
			}

			if (m_scoldCooldown > 0.0f)
			{
				m_scoldCooldown -= g_gameTime->GetFrameDeltaTime();
			}

			// ==========================================================
			// 索敵処理：シロクマと渦潮の両方を探す
			// ==========================================================
			// まだどちらにも向かっていない場合
			if (!m_naughtyStateMachine->GetIsGoingToWakeBear() && !m_naughtyStateMachine->GetIsGoingToWhirlpool() && m_scoldCooldown <= 0.0f)
			{
				const Vector3& myPos = m_owner->GetTransform().m_position;

				// マネージャーの機能を使って、指定距離内にいる一番近い「寝ているシロクマ」を取得
				Enemy* targetBear = EnemyManager::GetInstance()->GetNearestSleepingEnemy(myPos, WAKE_BEAR_TRIGGER_DISTANCE);

				Vector3 whirlpoolPos = Vector3::Zero;
				bool foundWhirlpool = false;
				float minDistSq = FLT_MAX;

				nature::WhirlpoolManager::GetInstance()->ForEach([&](nature::Whirlpool* wp)
					{
						if (wp->GetState() == nature::Whirlpool::EnWhirlpoolState::None) return;

						const float wpRadius = wp->GetMaxRadius();
						const float triggerDistSq = (WHIRLPOOL_TRIGGER_DISTANCE + wpRadius) * (WHIRLPOOL_TRIGGER_DISTANCE + wpRadius);

						const Vector3& pos = wp->GetTransform().m_position;
						float distSq = (pos - myPos).LengthSq();

						if (distSq <= minDistSq && distSq <= triggerDistSq)
						{
							minDistSq = distSq;
							whirlpoolPos = pos;
							foundWhirlpool = true;
						}
					});

				if (targetBear != nullptr && foundWhirlpool)
				{
					float bearDistSq = (targetBear->GetTransform().m_position - myPos).LengthSq();
					if (bearDistSq < minDistSq) {
						foundWhirlpool = false; // シロクマ優先
					}
					else {
						targetBear = nullptr;   // 渦潮優先
					}
				}

				// 崖の向こうなど、歩いて到達できないいたずら先は最初から諦める
				// （行けない目的地へ向かって壁を押し続けるスタックの予防）
				if (const StageNavGrid* navGrid = GetStageNavGrid())
				{
					const Vector3& myPos = m_owner->GetTransform().m_position;
					if (targetBear != nullptr
						&& !navGrid->IsReachable(myPos, targetBear->GetTransform().m_position))
					{
						targetBear = nullptr;
					}
					if (foundWhirlpool && !navGrid->IsReachable(myPos, whirlpoolPos))
					{
						foundWhirlpool = false;
					}
				}

				// 近くに寝ているシロクマが見つかったらフラグをONにして向かう！
				if (targetBear != nullptr)
				{
					m_naughtyStateMachine->SetIsGoingToWakeBear(true);
					m_naughtyStateMachine->SetTargetBear(targetBear);
					m_naughtyStateMachine->SetBearTargetPos(targetBear->GetTransform().m_position);
					if (auto* lm = GameLogManager::GetInstance())
						lm->QueueEvent({ {"ev", "naughty_disobey"}, {"penguin_id", m_owner->GetLogId()}, {"toward", "bear"}, {"bear_id", targetBear->GetLogId()} });

					// いたずらを決意した瞬間にリアクションを要求する
					// 優先度Highのため、この後のRemoveFollower()が発行するTrouble(Normal)には上書きされない
					BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Happy, ui::EnCPReactionPriority::High);

					// シロクマに向かうため、隊列や徘徊からは離脱する
					if (m_isFollowing)
					{
						manager->RemoveFollower(m_owner);
						m_isFollowing = false;
					}
					if (manager->IsRoaming(m_owner))
					{
						manager->UnregisterRoaming(m_owner);
					}

					manager->RegisterAttempting(m_owner);
				}
				// 近くに寝ているシロクマはいないけど、渦潮が見つかったら向かう
				else if (foundWhirlpool)
				{
					m_naughtyStateMachine->SetIsGoingToWhirlpool(true);
					m_naughtyStateMachine->SetWhirlpoolTargetPos(whirlpoolPos);
					if (auto* lm = GameLogManager::GetInstance())
						lm->QueueEvent({ {"ev", "naughty_disobey"}, {"penguin_id", m_owner->GetLogId()}, {"toward", "whirlpool"} });

					// いたずらを決意した瞬間にリアクションを要求する
					// 優先度Highのため、この後のRemoveFollower()が発行するTrouble(Normal)には上書きされない
					BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Happy, ui::EnCPReactionPriority::High);

					if (m_isFollowing) { manager->RemoveFollower(m_owner); m_isFollowing = false; }
					if (manager->IsRoaming(m_owner)) { manager->UnregisterRoaming(m_owner); }
					manager->RegisterAttempting(m_owner);
				}
			}

			// ==========================================================
			// 渦潮に向かっている最中の処理
			// ==========================================================
			if (m_naughtyStateMachine->GetIsGoingToWhirlpool())
			{
				// 1. 現在渦潮に巻き込まれているかチェック
				if (m_owner->GetStateMachine()->GetIsInWhirlpool())
				{
					// TODO: 渦潮に入った瞬間のSEを鳴らす場合はここに実装する


					// 巻き込まれたフラグを立てて、入力はゼロにしてシステムに身を任せる
					m_wasSwallowedByWhirlpool = true;
					if (m_isFollowing) { m_isFollowing = false; }
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
					PlayLivelyEffect();
					return;
				}
				else
				{
					// 2. 巻き込まれていない場合
					if (m_wasSwallowedByWhirlpool)
					{
						// 一度巻き込まれた後なら、渦潮から吐き出された（遊び終わった）ということ！
						m_naughtyStateMachine->SetIsGoingToWhirlpool(false);
						m_wasSwallowedByWhirlpool = false;
						manager->UnregisterAttempting(m_owner);
						m_scoldCooldown = SCOLD_COOLDOWN_DURATION; // 満足して親の元へ帰る
						StopLivelyEffect();
						return;
					}

					// 3. まだ巻き込まれていない場合は、目標地点へ向かって走る
					const Vector3& targetPos = m_naughtyStateMachine->GetWhirlpoolTargetPos();
					const float distToWhirlpool = GetDistanceToTarget(targetPos);

					// 渦潮の中心付近に到達したのに巻き込まれない場合は、渦潮が既に消滅していると判断
					if (distToWhirlpool <= REACH_WHIRLPOOL_DISTANCE)
					{
						m_naughtyStateMachine->SetIsGoingToWhirlpool(false);
						manager->UnregisterAttempting(m_owner);
						m_scoldCooldown = WHIRLPOOL_MISS_COOLDOWN_DURATION; // 少し反省して帰る
						StopLivelyEffect();
						return;
					}
					else
					{
						Vector3 dir = CalculateDirectionToTarget(targetPos);
						m_stateMachine->SetActionInput(dir, false, true, false, false); // ダッシュで向かう
					}

					PlayLivelyEffect();
					return;
				}
			}

			// すでにシロクマに向かっている最中（または上で向かう決定をした直後）の処理
			if (m_naughtyStateMachine->GetIsGoingToWakeBear())
			{
				// ==========================================================
				// 向かっている途中でシロクマが起きてしまったら行動をキャンセルする
				// ==========================================================
				Enemy* bear = m_naughtyStateMachine->GetTargetBear();
				if (bear == nullptr || !bear->GetEnemyStateMachine()->IsCoolDown())
				{
					m_naughtyStateMachine->SetIsGoingToWakeBear(false);
					m_naughtyStateMachine->SetIsAtBear(false);
					manager->UnregisterAttempting(m_owner);
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);

					StopLivelyEffect();
					return;
				}

				const Vector3& bearPos = m_naughtyStateMachine->GetBearTargetPos();
				const float distToBear = GetDistanceToTarget(bearPos);

				// シロクマに到達したか判定（停止距離を利用）
				if (distToBear <= REACH_BEAR_DISTANCE)
				{
					m_naughtyStateMachine->SetIsAtBear(true);
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				}
				else
				{
					// 到達していなければシロクマに向かって移動
					Vector3 dir = CalculateDirectionToTarget(bearPos);

					m_stateMachine->SetActionInput(dir, true, false, false, false);
				}

				// エフェクトを再生する。
				PlayLivelyEffect();
				// シロクマに対処している間は、この後の「追従・待機の通常ロジック」を無視する
				return;
			}

			/** 追従命令のとき */
			if (isFollowCmd)
			{
				/** 入隊条件を満たしたら徘徊を終了して隊列に参加する */
				/** （条件は CanJoinFormation() に集約してある） */
				if (CanJoinFormation())
				{
					/** 徘徊登録を解除する */
					manager->UnregisterRoaming(m_owner);

					StopLivelyEffect();

					TryJoinFormation();
				}

				/** 隊列に参加していない状態（＝まだ遠くにいる）なら徘徊を継続する */
				if (!m_isFollowing)
				{
					/** 徘徊中でなければ新しい目標を選ぶ */
					if (!manager->IsRoaming(m_owner))
					{
						manager->RegisterRoaming(m_owner);
						PickNewRoamTarget();
					}

					const float distToRoamTarget = GetDistanceToTarget(m_roamTarget);

					/** 目標地点に到達したら次の目標を選ぶ */
					if (distToRoamTarget <= m_stopDistance)
					{
						PickNewRoamTarget();
					}

					BuildInputToTarget(m_roamTarget);
					PlayLivelyEffect();
					return;
				}

				/** 隊列参加中：通常の追従入力 */
				BuildInput();
				return;
			}

			/** 待機命令のとき */

			/** 隊列から離脱する */
			if (m_isFollowing)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** 親が一定距離以上離れたら徘徊を開始する */
			if (!manager->IsRoaming(m_owner) && distDaddy >= m_roamTriggerDistance)
			{
				manager->RegisterRoaming(m_owner);
				PickNewRoamTarget();
			}

			/** 徘徊中 */
			if (manager->IsRoaming(m_owner))
			{
				const float distToRoamTarget = GetDistanceToTarget(m_roamTarget);

				/** 目標地点に到達したら次の目標を選ぶ */
				if (distToRoamTarget <= m_stopDistance)
				{
					PickNewRoamTarget();
				}

				BuildInputToTarget(m_roamTarget);
				PlayLivelyEffect();
				return;
			}

			/** 待機命令中かつ親が近い場合はその場で待機する */
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
		}


		void NaughtyChildPenguinAI::PickNewRoamTarget()
		{
			/**
			 * 円内のランダムな座標を選ぶ（拒絶サンプリング）。
			 * ナビグリッドで到達できる目的地だけを採用することで、
			 * 崖の上や絶壁の向こうを引いてスタックするのを未然に防ぐ
			 */
			const Vector3& currentPos = m_owner->GetTransform().m_position;
			const StageNavGrid* navGrid = GetStageNavGrid();

			for (int i = 0; i < 16; i++)
			{
				const float x = util::RandomDevice::Random(-m_roamRadius, m_roamRadius);
				const float z = util::RandomDevice::Random(-m_roamRadius, m_roamRadius);
				if ((x * x + z * z) > (m_roamRadius * m_roamRadius)) continue;

				const Vector3 candidate(currentPos.x + x, currentPos.y, currentPos.z + z);
				if (navGrid != nullptr && !navGrid->IsReachable(currentPos, candidate)) continue;

				m_roamTarget = candidate;
				return;
			}

			/** 最大試行回数を超えた場合は現在地をそのまま目標にする */
			m_roamTarget = currentPos;
		}


		void NaughtyChildPenguinAI::UpdateStuckWatch()
		{
			const Vector3 currentPos = m_owner->GetTransform().m_position;

			// 「移動する意思があるのに動けていない」時間を測る。
			// 急斜面（接地限界63度超）へ向かって歩き続けると、押し戻されて足踏みになる
			const Vector3& moveDir = m_stateMachine->GetMoveDirection();
			Vector3 moved = currentPos - m_stuckCheckPos;
			moved.y = 0.0f;

			const bool wantsToMove = moveDir.LengthSq() > FLT_EPSILON;
			if (!wantsToMove
				|| moved.LengthSq() >= NAUGHTY_STUCK_MOVE_THRESHOLD * NAUGHTY_STUCK_MOVE_THRESHOLD)
			{
				m_stuckTimer = 0.0f;
				m_stuckCheckPos = currentPos;
				return;
			}

			m_stuckTimer += g_gameTime->GetFrameDeltaTime();
			if (m_stuckTimer < NAUGHTY_STUCK_TIME_LIMIT) return;

			m_stuckTimer = 0.0f;
			m_stuckCheckPos = currentPos;

			auto* manager = ChildPenguinManager::GetInstance();

			// いたずら（クマ起こし・渦潮）へ向かう途中なら諦める。壁の向こうには行けない
			if (m_naughtyStateMachine->GetIsGoingToWakeBear()
				|| m_naughtyStateMachine->GetIsGoingToWhirlpool())
			{
				m_naughtyStateMachine->SetIsGoingToWakeBear(false);
				m_naughtyStateMachine->SetIsAtBear(false);
				m_naughtyStateMachine->SetIsGoingToWhirlpool(false);
				m_naughtyStateMachine->SetIsAtWhirlpool(false);
				m_wasSwallowedByWhirlpool = false;
				manager->UnregisterAttempting(m_owner);
				StopLivelyEffect();
			}

			// 徘徊中なら、進めなかった方向の反対側から行き先を選び直す
			if (manager->IsRoaming(m_owner))
			{
				PickNewRoamTargetAwayFromBlocked(moveDir);
			}
		}


		void NaughtyChildPenguinAI::PickNewRoamTargetAwayFromBlocked(const Vector3& blockedDir)
		{
			const Vector3& currentPos = m_owner->GetTransform().m_position;

			Vector3 back = blockedDir;
			back.y = 0.0f;
			if (back.LengthSq() <= FLT_EPSILON)
			{
				PickNewRoamTarget();
				return;
			}
			back.Normalize();
			back *= -1.0f;

			// 反対方向を中心に±75度のランダムな向きへ、半径の40〜100%の距離を選ぶ。
			// こちらもナビグリッドで到達できる目的地だけを採用する
			const StageNavGrid* navGrid = GetStageNavGrid();
			for (int i = 0; i < 8; i++)
			{
				const float angle = util::RandomDevice::Random(
					-NAUGHTY_STUCK_ESCAPE_HALF_ANGLE, NAUGHTY_STUCK_ESCAPE_HALF_ANGLE);
				const float cosA = cosf(angle);
				const float sinA = sinf(angle);

				Vector3 dir;
				dir.x = back.x * cosA - back.z * sinA;
				dir.y = 0.0f;
				dir.z = back.x * sinA + back.z * cosA;

				const float dist = util::RandomDevice::Random(m_roamRadius * 0.4f, m_roamRadius);
				const Vector3 candidate = currentPos + dir * dist;

				if (navGrid != nullptr && !navGrid->IsReachable(currentPos, candidate)) continue;

				m_roamTarget = candidate;
				return;
			}

			/** 見つからない場合は現在地を目標にする（到達扱いになり、次フレームで選び直される） */
			m_roamTarget = currentPos;
		}


		void NaughtyChildPenguinAI::PlayLivelyEffect()
		{
			// 時間計測。
			m_livelyInterval += g_gameTime->GetFrameDeltaTime();

			if (m_livelyInterval > LIVELY_INTERVAL)
			{
				const Vector3 effectPos = m_owner->GetTransform().m_position;
				const Vector3 effectScl = m_owner->GetTransform().m_scale + NAUGHTY_LIVELY_EFFECT_SCALE;

				// エフェクトの再生。
				m_livelyEffectHandle = EffectManager::Get().PlayEffect(
					EnEffectKind::NaughtyPenguinLively
					, effectPos
					, Quaternion::Identity
					, effectScl
				);
				// キャラクターに追従させる。
				EffectManager::Get().AttachEffect(
					m_livelyEffectHandle,
					&m_owner->GetTransform().m_position
				);

				// インターバルをリセット。
				m_livelyInterval = 0.0f;
			}
		}


		void NaughtyChildPenguinAI::StopLivelyEffect()
		{
			// エフェクトハンドルが有効なら
			if (m_livelyEffectHandle != INVALID_EFFECT_HANDLE)
			{
				// エフェクトを停止。
				EffectManager::Get().StopEffect(m_livelyEffectHandle);
				// ハンドルを無効化。
				m_livelyEffectHandle = INVALID_EFFECT_HANDLE;
				// インターバルをリセット。
				m_livelyInterval = 0.0f;
			}
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// ClumsyChildPenguinAI（おっちょこちょいペンギン）
		//--------------------------------------------------------------

		ClumsyChildPenguinAI::ClumsyChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Clumsy)
			, m_clumsyStateMachine(static_cast<ClumsyChildPenguinStateMachine*>(owner->GetStateMachine()))
		{
			const auto& td = GetTypeData(EnChildPenguinType::Clumsy);
			m_tripChancePerSec = td.tripChancePerSec;
			m_slipChance = td.slipChance;
		}


		void ClumsyChildPenguinAI::UpdateAI()
		{
			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}
			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();
			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** Managerに登録されている転倒・スリップ中フラグで判定する */
			const bool isDowningNow = manager->IsDowning(m_owner);

			/** 転倒・スリップから起き上がった瞬間を検出する（前フレームDowning中→今フレームDowningでない） */
			if (m_wasDowning && !isDowningNow)
			{
				if (m_wasHelpedThisDowning)
				{
					// 世話焼きペンギンに助けてもらって起き上がった瞬間にリアクションを要求する
					BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Happy, ui::EnCPReactionPriority::High);
				}
				m_wasHelpedThisDowning = false;
			}
			m_wasDowning = isDowningNow;

			if (isDowningNow)
			{
				/** 転倒・スリップ中は移動入力をゼロにして固有ステートの評価を妨げないようにする */
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				m_wasSliding = false;
				return;
			}

			/** シロクマ逃走チェック（かまくら > 逃走 > 通常AI の優先順） */
			if (CheckAndFlee()) return;

			/** 渦潮に飲まれている間は隊を抜けて入力をゼロにする */
			if (m_owner->GetStateMachine()->GetIsInWhirlpool())
			{
				if (m_isFollowing) { m_isFollowing = false; }
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				m_wasSliding = false;
				return;
			}

			/** 待機命令のとき */
			if (!isFollowCmd)
			{
				if (m_isFollowing)
				{
					manager->RemoveFollower(m_owner);
					m_isFollowing = false;
				}
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				m_wasSliding = false;
				return;
			}

			/** 入隊判定（条件は CanJoinFormation() に集約してある） */
			if (!TryJoinFormation())
			{
				BuildInputWhenNotFollowing();
				m_wasSliding = false;
				return;
			}

			/** スライド解除検出：前フレームがスライド中（PenguinSlidingStateのみ）で */
			/** 今フレームがスライド中でなく、かつ泳ぎ中でもなければスリップ判定を行う */
			const bool isCurrentlySliding = m_stateMachine->IsEqualCurrentState(PenguinSlidingState::ID());
			const bool isCurrentlySwimming = m_stateMachine->IsEqualCurrentState(PenguinSwimmingState::ID());

			if (m_wasSliding && !isCurrentlySliding && !isCurrentlySwimming)
			{
				if (RollUnit() < m_slipChance)
				{
					m_clumsyStateMachine->SetIsSlipped(true);
					m_wasSliding = false;

					// 転んだ瞬間にリアクションを要求する
					m_wasHelpedThisDowning = false;
					BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Trouble, ui::EnCPReactionPriority::High);
					return;
				}
			}
			m_wasSliding = isCurrentlySliding;

			/** 泳ぎ中は転倒判定をしない */
			if (isCurrentlySwimming)
			{
				BuildInput();
				return;
			}

			/** 隊列参加中の通常移動入力 */
			BuildInput();

			/** 歩き・走り中のみ転倒判定を行う（秒あたりの確率をフレーム確率に変換） */
			const bool isWalking = m_stateMachine->IsEqualCurrentState(PenguinSneakState::ID());
			const bool isRunning = m_stateMachine->IsEqualCurrentState(PenguinRunState::ID());

			if (isWalking || isRunning)
			{
				const float tripChancePerFrame = m_tripChancePerSec * g_gameTime->GetFrameDeltaTime();
				if (RollUnit() < tripChancePerFrame)
				{
					m_clumsyStateMachine->SetIsTripped(true);

					// 転んだ瞬間にリアクションを要求する
					m_wasHelpedThisDowning = false;
					BattleManager::GetInstance().NotifyCPReactionChanged(m_owner, ui::EnCPReactionType::Trouble, ui::EnCPReactionPriority::High);
				}
			}
		}


		void ClumsyChildPenguinAI::HelpedByCaringPenguin()
		{
			m_clumsyStateMachine->SetIsHelped(true);

			// 起き上がった瞬間にHappyを出すためのフラグ（ApplyIntervention()から毎フレーム呼ばれるため、ここでは単純にtrueを立てるだけでよい）
			m_wasHelpedThisDowning = true;
		}




		/**************************************************************/


		//--------------------------------------------------------------
		// CaringChildPenguinAI（世話焼きペンギン）
		//--------------------------------------------------------------

		CaringChildPenguinAI::CaringChildPenguinAI(ChildPenguin* owner)
			: ChildPenguinAIController(owner, EnChildPenguinType::Caring)
			, m_caringEffectHandles({ INVALID_EFFECT_HANDLE, INVALID_EFFECT_HANDLE, INVALID_EFFECT_HANDLE })
		{
			const auto& td = GetTypeData(EnChildPenguinType::Caring);
			m_interventionRange = td.interventionRange;
		}


		CaringChildPenguinAI::~CaringChildPenguinAI()
		{
			StopAllCaringEffects();
		}


		void CaringChildPenguinAI::UpdateInterventionLog()
		{
			if (m_interventionTarget == m_loggedInterventionTarget) return;

			auto* lm = GameLogManager::GetInstance();
			const float now = TimeManager::GetInstance().GetCurTime();

			// 対象が入れ替わるときは、直前の対象の介入をまず閉じる
			if (m_loggedInterventionTarget != nullptr && lm != nullptr)
			{
				// 残り時間は減っていくので、開始時刻から現在時刻を引いたものが経過秒数になる
				lm->QueueEvent({
					{ "ev",         "caring_help_end" },
					{ "penguin_id", m_owner->GetLogId() },
					{ "target_id",  m_loggedInterventionTarget->GetLogId() },
					{ "sec",        m_interventionStartTime - now },
					{ "reached",    m_hasReachedInterventionTarget }
				});
			}

			if (m_interventionTarget != nullptr && lm != nullptr)
			{
				lm->QueueEvent({
					{ "ev",          "caring_help_start" },
					{ "penguin_id",  m_owner->GetLogId() },
					{ "target_id",   m_interventionTarget->GetLogId() },
					{ "target_type", m_interventionTarget->GetChildPenguinTypeStr() }
				});
			}

			m_interventionStartTime = now;
			m_hasReachedInterventionTarget = false;
			m_loggedInterventionTarget = m_interventionTarget;
		}


		void CaringChildPenguinAI::UpdateAI()
		{
			// 介入対象は以降の処理の複数の経路で差し替わるため、前フレームの結果をここで拾って記録する
			UpdateInterventionLog();

			if (m_isEnterIglooMode) {
				UpdateIglooEvent();
				return;
			}

			/** シロクマ逃走チェック（かまくら > 逃走 > 通常AI の優先順） */
			if (CheckAndFlee()) return;

			/** 子ペンギンマネージャーのインスタンスを取得 */
			auto* manager = ChildPenguinManager::GetInstance();

			if (m_owner->GetStateMachine()->GetIsInWhirlpool())
			{
				if (m_interventionTarget != nullptr)
				{
					// 手を離してあげる
					ReleaseSuppression(m_interventionTarget);
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
				}
				if (m_isFollowing) { m_isFollowing = false; }
				// 自分の入力もゼロにしてシステムに身を任せる
				m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				return;
			}

			const bool isFollowCmd = manager->GetCommand() == ChildPenguinManager::EnPenguinCommand::Follow;

			/** 追従命令のとき */
			if (isFollowCmd)
			{
				/** 担当対象が消えていたら（死亡など）クリアする */
				if (m_interventionTarget != nullptr)
				{
					const auto& childList = manager->GetChildPenguin();
					const bool exists = std::find(childList.begin(), childList.end(), m_interventionTarget) != childList.end();
					if (!exists)
					{
						manager->UnregisterAssigned(m_interventionTarget);
						m_interventionTarget = nullptr;
					}
				}

				/** 助け終わったらターゲットをクリアして次を探す */
				if (m_interventionTarget != nullptr &&
					m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Clumsy)
				{
					if (!manager->IsDowning(m_interventionTarget))
					{
						/** 起き上がり完了 → 介入終了 */
						ReleaseSuppression(m_interventionTarget);
						manager->UnregisterAssigned(m_interventionTarget);
						m_interventionTarget = nullptr;

						StopAllCaringEffects();
					}
				}
				else if (m_interventionTarget != nullptr)
				{
					bool shouldRelease = true;
					if (m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Naughty)
					{
						auto* naughtySM = static_cast<NaughtyChildPenguinStateMachine*>(m_interventionTarget->GetStateMachine());
						if (naughtySM && (naughtySM->GetIsGoingToWakeBear() || naughtySM->GetIsGoingToWhirlpool()))
						{
							shouldRelease = false; // シロクマに対処中なので離さない
						}

						if (naughtySM && naughtySM->GetIsInWhirlpool())
						{
							shouldRelease = true;
						}
					}

					if (shouldRelease)
					{
						/** 制止対象の命令が Follow になったら制止を解除する */
						ReleaseSuppression(m_interventionTarget);
						manager->UnregisterAssigned(m_interventionTarget);
						m_interventionTarget = nullptr;
					}
				}

				/** 担当がいなければ新たに探す */
				if (m_interventionTarget == nullptr)
				{
					const auto& assigned = manager->GetAssignedTargets();
					const Vector3& myPos = m_owner->GetTransform().m_position;

					ChildPenguin* target = manager->FindNearestDowning(myPos, assigned, m_interventionRange);

					if (target == nullptr)
					{
						ChildPenguin* supervisionTarget = manager->FindNearestNeedingSupervision(myPos, assigned, m_interventionRange);

						if (supervisionTarget != nullptr && supervisionTarget->GetChildPenguinType() == EnChildPenguinType::Naughty)
						{
							auto* naughtySM = static_cast<NaughtyChildPenguinStateMachine*>(supervisionTarget->GetStateMachine());
							// シロクマに向かっている場合のみターゲットにする
							if (naughtySM && (naughtySM->GetIsGoingToWakeBear() || (naughtySM->GetIsGoingToWhirlpool() && !naughtySM->GetIsInWhirlpool())))
							{
								target = supervisionTarget;
							}
						}
					}

					if (target != nullptr)
					{
						m_interventionTarget = target;
						manager->RegisterAssigned(m_interventionTarget);
					}
				}

				/** 担当のおっちょこちょいがいる場合 */
				if (m_interventionTarget != nullptr)
				{
					/** 助けに向かう間は隊列から外れる */
					if (m_isFollowing)
					{
						manager->RemoveFollower(m_owner);
						m_isFollowing = false;
					}

					if (IsCloseEnoughTo(m_interventionTarget))
					{
						/** 十分近づいたら介入処理を適用してその場で待機する */
						ApplyIntervention(m_interventionTarget);
						m_hasReachedInterventionTarget = true;
						m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
					}
					else
					{
						/** ターゲットの座標へ向かって移動する */
						if (m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Naughty)
						{
							Vector3 dir = CalculateDirectionToTarget(m_interventionTarget->GetTransform().m_position);

							m_stateMachine->SetActionInput(dir, false, true, false, true);
						}
						else
						{
							BuildInputToTarget(m_interventionTarget->GetTransform().m_position);
						}
					}
					return;
				}

				/** 介入対象がいなければ通常の追従行動 */
				/** 入隊判定（条件は CanJoinFormation() に集約してある） */
				if (!TryJoinFormation())
				{
					BuildInputWhenNotFollowing();
					return;
				}

				BuildInput();
				return;
			}

			/** 待機命令のとき */
			if (m_isFollowing)
			{
				manager->RemoveFollower(m_owner);
				m_isFollowing = false;
			}

			/** おっちょこちょいへの介入：助け終わったらターゲットをクリアして次を探す */
			if (m_interventionTarget != nullptr &&
				m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Clumsy)
			{
				if (!manager->IsDowning(m_interventionTarget))
				{
					/** 起き上がり完了 → 介入終了 */
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
					StopAllCaringEffects();
				}
			}
			else if (m_interventionTarget != nullptr &&
				m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Naughty)
			{
				auto* naughtySM = static_cast<NaughtyChildPenguinStateMachine*>(m_interventionTarget->GetStateMachine());
				// シロクマにも渦潮にも向かっていなければ（反省していれば）手を離す
				if (naughtySM && ((!naughtySM->GetIsGoingToWakeBear() && !naughtySM->GetIsGoingToWhirlpool()) || naughtySM->GetIsInWhirlpool()))
				{
					ReleaseSuppression(m_interventionTarget);
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
				}
			}

			/** 担当対象が消えていたら（死亡など）クリアする */
			if (m_interventionTarget != nullptr)
			{
				const auto& childList = manager->GetChildPenguin();
				const bool exists = std::find(childList.begin(), childList.end(), m_interventionTarget) != childList.end();
				if (!exists)
				{
					manager->UnregisterAssigned(m_interventionTarget);
					m_interventionTarget = nullptr;
				}
			}

			/** 担当がいなければ新たに探す */
			if (m_interventionTarget == nullptr)
			{
				const auto& assigned = manager->GetAssignedTargets();
				const Vector3& myPos = m_owner->GetTransform().m_position;

				/** 優先①：倒れているおっちょこちょい */
				ChildPenguin* target = manager->FindNearestDowning(myPos, assigned, m_interventionRange);

				/** 優先②：問題行動中の甘えん坊・やんちゃ */
				if (target == nullptr)
				{
					ChildPenguin* supervisionTarget = manager->FindNearestNeedingSupervision(myPos, assigned, m_interventionRange);

					// ==========================================================
					// 渦潮に飲まれているやんちゃはターゲットから除外する
					// ==========================================================
					if (supervisionTarget != nullptr && supervisionTarget->GetChildPenguinType() == EnChildPenguinType::Naughty)
					{
						auto* naughtySM = static_cast<NaughtyChildPenguinStateMachine*>(supervisionTarget->GetStateMachine());
						if (naughtySM && naughtySM->GetIsInWhirlpool())
						{
							supervisionTarget = nullptr;
						}
					}

					target = supervisionTarget;
				}

				if (target != nullptr)
				{
					m_interventionTarget = target;
					manager->RegisterAssigned(m_interventionTarget);
				}
			}

			/** 担当対象がいる場合 */
			if (m_interventionTarget != nullptr)
			{
				if (IsCloseEnoughTo(m_interventionTarget))
				{
					/** 十分近づいたら介入処理を適用してその場で待機する */
					ApplyIntervention(m_interventionTarget);
					m_hasReachedInterventionTarget = true;
					m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
				}
				else
				{
					/** ターゲットの座標へ向かって移動する */
					if (m_interventionTarget->GetChildPenguinType() == EnChildPenguinType::Naughty)
					{
						Vector3 dir = CalculateDirectionToTarget(m_interventionTarget->GetTransform().m_position);

						m_stateMachine->SetActionInput(dir, false, true, false, true);
					}
					else
					{
						BuildInputToTarget(m_interventionTarget->GetTransform().m_position);
					}
				}
				return;
			}

			/** 介入対象がいなければその場で待機する */
			m_stateMachine->SetActionInput(Vector3::Zero, false, false, false, false);
		}


		bool CaringChildPenguinAI::IsCloseEnoughTo(const ChildPenguin* target) const
		{
			return GetDistanceToTarget(target->GetTransform().m_position) <= INTERVENTION_REACH_DISTANCE;
		}


		void CaringChildPenguinAI::PlayCaringEffect() const
		{
			// カウント数が最大カウント数より大きい場合
			if (m_sweatEffectCount >= MAX_SWEAT_COUNT) return;

			// 汗エフェクトの連続再生制御のための時間計測。
			m_sweatEffectCoolTime += g_gameTime->GetFrameDeltaTime();

			// カウントゼロじゃないかつインターバル未満の場合エフェクトを再生しない。
			if (m_sweatEffectCount != 0 && m_sweatEffectCoolTime < SWEAT_INTERVAL) return;

			// 汗エフェクトが生み出される位置。
			const Vector3 sweatPos = m_owner->GetTransform().m_position + CARING_SWEAT_EFFECT_POSITION;
			// 汗エフェクトの大きさ。
			const Vector3 sweatScl = m_owner->GetTransform().m_scale + CARING_SWEAT_EFFECT_SCALE;
			// 汗エフェクトの再生。インデックスはカウント増加前の値を使う。
			m_caringEffectHandles[m_sweatEffectCount] = EffectManager::Get().PlayEffect(
				EnEffectKind::CaringPenguinSweat
				, sweatPos
				, Quaternion::Identity
				, sweatScl
			);
			// キャラクターに追従させる。
			EffectManager::Get().AttachEffect(
				m_caringEffectHandles[m_sweatEffectCount],
				&m_owner->GetTransform().m_position,
				CARING_SWEAT_EFFECT_POSITION
			);
			// カウントを増やす。
			m_sweatEffectCount++;
			// 値のリセット。
			m_sweatEffectCoolTime = 0.0f;
		}


		void CaringChildPenguinAI::StopAllCaringEffects() const
		{
			for (auto& handle : m_caringEffectHandles)
			{
				if (handle != INVALID_EFFECT_HANDLE)
				{
					EffectManager::Get().StopEffect(handle);
					handle = INVALID_EFFECT_HANDLE;
				}
			}
			m_sweatEffectCount = 0;
			m_sweatEffectCoolTime = 0.0f;
		}


		void CaringChildPenguinAI::ApplyIntervention(ChildPenguin* target) const
		{
			switch (target->GetChildPenguinType())
			{
			case EnChildPenguinType::Clumsy:
			{
				/** おっちょこちょいを助けて即座に起き上がらせる */
				auto* ai = static_cast<ClumsyChildPenguinAI*>(target->GetAIController());

				if (ai)
				{
					ai->HelpedByCaringPenguin();

					PlayCaringEffect();
				}
				break;
			}
			case EnChildPenguinType::Clingy:
			{
				/** 甘えん坊を制止する */
				auto* ai = static_cast<ClingyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(true);
				}
				break;
			}
			case EnChildPenguinType::Naughty:
			{
				/** やんちゃを制止する */
				auto* ai = static_cast<NaughtyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(true);
				}
				break;
			}
			default:
				break;
			}
		}


		void CaringChildPenguinAI::ReleaseSuppression(ChildPenguin* target) const
		{
			switch (target->GetChildPenguinType())
			{
			case EnChildPenguinType::Clingy:
			{
				/** 甘えん坊の制止を解除する */
				auto* ai = static_cast<ClingyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(false);
				}
				break;
			}
			case EnChildPenguinType::Naughty:
			{
				/** やんちゃの制止を解除する */
				auto* ai = static_cast<NaughtyChildPenguinAI*>(target->GetAIController());
				if (ai)
				{
					ai->SetRestrained(false);
				}
				break;
			}
			default:
				break;
			}
		}
	}
}