/**
 * @file PlayerFront.h
 * @brief プレイヤー（親ペンギン）の「正面」の計算と、敵の回り込みの計算
 */
#pragma once


namespace app
{
	namespace actor
	{
		/**
		 * @brief プレイヤーの正面の計算と、敵（シロクマ・シャチ）の回り込みの計算
		 * @details 「正面」はカメラ前方のXZ成分（カメラが真上・真下を向いているときは親の向き）、起点は親の位置。
		 *          敵は親を中心とした円軌道を回り、正面コーンに入ってから狙った子へ突進する。
		 *          StepApproachなどの計算は純関数にしてあり、SelfTestで約束を確かめる。
		 *          親とカメラを読むのはGetFront / IsVisible / IsPointVisibleだけ。
		 *          クマ（EnemyController）・シャチ（OrcaController）・渦潮（WhirlpoolManager）で共用する。
		 */
		class PlayerFront
		{
		public:
			/** 回り込みの状態 */
			enum class EnApproachPhase : uint8_t
			{
				Idle,		/** 回り込まずに標的へ直行する（狙った子が群れから離れている） */
				Flanking,	/** 正面へ回り込んでいる（この間は攻撃しない） */
				Done,		/** 回り込みを終えた（正面に入った・時間切れ・地形など）。以後は直行する */
			};

			/** 直前の更新でFlankingを抜けた理由（ログ用） */
			enum class EnApproachEndReason : uint8_t
			{
				None,		/** Flankingを抜けていない */
				Front,		/** 正面コーンに入った */
				Timeout,	/** 時間切れ */
				Far,		/** 狙った子が群れから離れた */
			};

			/** 回り込みの状態（敵ごとに持つ） */
			struct ApproachState
			{
				EnApproachPhase phase = EnApproachPhase::Idle;
				/** Flankingに入ってからの時間（秒） */
				float flankTimer = 0.0f;
				/** 回り込み中に向かう点 */
				Vector3 steerPoint = Vector3::Zero;
				/** 回る向き（+1 / -1。0は未決定）。真後ろで左右が毎フレーム入れ替わらないように保持する */
				float orbitSign = 0.0f;
				/** 直前の更新でFlankingを抜けた理由 */
				EnApproachEndReason endReason = EnApproachEndReason::None;
			};

			/** StepApproachの入力（1フレーム分） */
			struct ApproachInput
			{
				/** 敵の位置 */
				Vector3 enemyPosition = Vector3::Zero;
				/** 狙った子の位置 */
				Vector3 targetPosition = Vector3::Zero;
				/** 親の位置（正面の起点） */
				Vector3 parentPosition = Vector3::Zero;
				/** 正面の向き（XZの単位ベクトル。CalcFrontDirXZの結果） */
				Vector3 frontDirection = Vector3::Front;
				/** フレーム時間（秒） */
				float deltaTime = 0.0f;
			};

			/** 回り込みの調整値（敵の種類ごと） */
			struct ApproachParams
			{
				/** 親を中心に回る半径 */
				float orbitRadius = 400.0f;
				/** 回り込みを打ち切るまでの時間（秒） */
				float timeLimit = 5.0f;
				/** 狙った子が親からこれより遠ければ回り込まずに直行する（群れから外れた子には正面が定義できないため） */
				float directDistance = 450.0f;
				/** 回り込み中に狙った子が親からこれより遠くなったら直行へ戻す */
				float releaseDistance = 900.0f;
				/** 正面コーンの半角（度） */
				float frontHalfAngleDeg = 40.0f;
				/** 敵の方位角から正面寄りにずらす角度（度） */
				float leadAngleDeg = 45.0f;
			};


		public:
			/**
			 * @brief 正面の向きを求める
			 * @param cameraForward カメラの前方向
			 * @param parentRotation 親の回転（カメラが真上・真下を向いているときに使う）
			 * @return XZ平面上の単位ベクトル
			 */
			static Vector3 CalcFrontDirXZ(const Vector3& cameraForward, const Quaternion& parentRotation);

			/**
			 * @brief targetPosがbasePosから見て正面側にあるか（XZ平面で判定）
			 * @param basePos 起点
			 * @param frontDir 正面の向き（XZの単位ベクトル）
			 * @param targetPos 判定する位置
			 * @param cosThreshold 内積の閾値（これ以上なら正面）
			 * @return 正面側ならtrue。起点と同じ位置ならtrue
			 */
			static bool IsInFrontXZ(const Vector3& basePos, const Vector3& frontDir, const Vector3& targetPos, float cosThreshold);

			/**
			 * @brief 回り込みの状態を1フレーム進める（純関数）
			 * @param state 現在の状態
			 * @param input 入力
			 * @param params 調整値
			 * @return 次の状態
			 */
			static ApproachState StepApproach(const ApproachState& state, const ApproachInput& input, const ApproachParams& params);

			/**
			 * @brief 親の位置と正面の向きを取得する（親とカメラを読む）
			 * @param outParentPos 親の位置
			 * @param outFrontDir 正面の向き（XZの単位ベクトル）
			 * @return 親がいなければfalse
			 */
			static bool GetFront(Vector3& outParentPos, Vector3& outFrontDir);

			/**
			 * @brief 球が画面（メインカメラの視錐台）に入っているか
			 * @details RenderingEngineのフラスタムは前フレームの値なので使わず、
			 *          呼び出し時点のメインカメラのビュー×プロジェクション行列から組み直して判定する
			 * @param center 球の中心
			 * @param radius 球の半径
			 * @return 一部でも入っていればtrue
			 */
			static bool IsVisible(const Vector3& center, float radius);

			/**
			 * @brief 点が画面（メインカメラの視錐台）に入っているか（判定方法はIsVisibleと同じ）
			 * @param point 判定する点
			 * @return 入っていればtrue
			 */
			static bool IsPointVisible(const Vector3& point);

			/**
			 * @brief 純関数の約束を確かめる（Debugのロード時に呼ぶ。失敗するとK2_ASSERTで止まる）
			 * @return すべて通ればtrue
			 */
			static bool SelfTest();
		};
	}
}