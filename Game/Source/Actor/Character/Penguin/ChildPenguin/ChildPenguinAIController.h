/**
 * @file ChildPenguinAIController.h
 * @brief 子ペンギンのAIコントローラー
 */
#pragma once
#include <array>
#include "ChildPenguinTypes.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStateMachine;
		class ClumsyChildPenguinStateMachine;
		class NaughtyChildPenguinStateMachine;
		class StageNavGrid;


		/**
		 * @brief タイプごとの「親の察知」性能
		 * @details 実際の値は ChildPenguinAIController.cpp の
		 *          DADDY_PERCEPTION_SPECS テーブルにある。
		 */
		struct DaddyPerceptionSpec
		{
			/** 視界が届く距離 */
			float sightDistance;
			/** 視野角の半角（度）。シロクマの VIEW_ANGLE = 70 と同じ持ち方 */
			float sightHalfAngleDeg;
			/** 視界に入り続けてから察知に変わるまでの時間（秒） */
			float noticeTime;
			/** 親の音が届く距離への倍率（耳のよさ） */
			float hearingScale;
		};


		/**
		 * @brief 子ペンギンのAIコントローラー基底クラス
		 */
		class ChildPenguinAIController
		{
		public:
			/**
			 * @brief 更新処理
			 * @details 全タイプ共通の前処理（親の察知）を行ってから UpdateAI() を呼ぶ。
			 *          タイプごとの処理は UpdateAI() 側に書くこと。
			 */
			void Update();

		public:
			/**
			 * @brief 再集合の呼びかけに応えた「勇敢」時間中かどうか
			 * @details 勇敢中はシロクマから逃げない。うっすら明滅の演出判定に使う。
			 * @return 勇敢時間が残っていればtrue
			 */
			bool IsBraveFromRegroup() const { return m_braveTimer > 0.0f; }

		protected:
			/**
			 * @brief タイプごとの更新処理
			 */
			virtual void UpdateAI() = 0;


		public:
			ChildPenguinAIController(ChildPenguin* owner, EnChildPenguinType type);
			virtual ~ChildPenguinAIController() = default;


		public:
			/**
			 * @brief かまくらに入るイベントを開始する
			 * @param targetPos 向かうべき入り口の座標（青い円）
			 */
			void StartEnterIglooEvent(const Vector3& targetPos)
			{
				m_iglooTargetPos = targetPos;
				m_isEnterIglooMode = true;
				m_isInsideIgloo = false;
			}


			// ★ 追加：イベント中かどうかを取得する関数
			bool IsEnterIglooMode() const { return m_isEnterIglooMode; }


			/**
			 * @brief かまくらから出てくるイベントを開始する
			 * @param exitPos 出てくる座標（青い円）
			 */
			void EndEnterIglooEvent(const Vector3& exitPos);


			/**
			 * @brief かまくらが壊された時の強制排出処理（★ 追加）
			 * @param iglooPos 壊されたかまくらの中心座標
			 */
			void ForceEjectFromIgloo(const Vector3& iglooPos);


		protected:
			/**
			 * @brief DaddyPenguinへの方向ベクトルを計算
			 * @return DaddyPenguinへの正規化された方向ベクトル
			 */
			Vector3 CalculateDirectionToDaddy() const;

			/**
			 * @brief DaddyPenguinまでの距離を取得
			 * @return DaddyPenguinまでの距離
			 */
			float GetDistanceToDaddy() const;

			/**
			 * @brief 任意の目標座標への正規化された方向ベクトルを計算
			 * @param targetPos 目標座標
			 * @return 正規化された方向ベクトル
			 */
			Vector3 CalculateDirectionToTarget(const Vector3& targetPos) const;

			/**
			 * @brief 任意の目標座標までの距離を取得
			 * @param targetPos 目標座標
			 * @return 距離
			 */
			float GetDistanceToTarget(const Vector3& targetPos) const;

			/**
			 * @brief 陣形ポジションまでの距離に応じてAIControllerInputを組み立てる
			 *
			 * @details
			 * 【移動手段の切り替え（3段階）】
			 *   距離が大きいほど速い移動手段を使う。親の移動状態には依存しない。
			 *   distance <= stopDistance : 停止
			 *   distance <= walkDistance : 歩き
			 *   distance <= runDistance  : 走り
			 *   distance >  runDistance  : 滑り
			 *
			 * 【ヒステリシスによるチラつき防止】
			 *   フェーズを「上げる閾値」と「下げる閾値」を分けている。
			 *   下げる閾値 = 上げる閾値 - HYSTERESIS
			 *   これにより、閾値付近での高速な状態切り替えを防ぐ。
			 *   ※ m_stopDistance > HYSTERESIS となる値を設定すること。
			 *
			 * 【目標手前での減速（アプローチ減速）】
			 *   stopDistance の2倍以内に入ると moveDirection をスケールダウンする。
			 *   目標に近づくほど入力が弱まり行き過ぎを抑制する。
			 */
			void BuildInput();

			/**
			 * @brief 任意の目標座標へ向かう入力を組み立てる（BuildInputの座標指定版）
			 * @param targetPos 移動先の座標
			 * @note 世話焼きペンギンの介入移動などに使用する
			 */
			void BuildInputToTarget(const Vector3& targetPos);

			/**
			 * @brief ナビゲーションを織り込んだ移動方向を求める
			 * @details 親（隊列）へ向かう遠距離移動はフローフィールドに沿って
			 *          水路や崖を回り込む。それ以外は目標への直進方向を返す。
			 * @param targetPos    移動先の座標
			 * @param distToTarget 目標までの距離（呼び出し側で計算済みの値）
			 * @return 移動方向（正規化済み）
			 */
			Vector3 CalculateMoveDirectionWithNav(const Vector3& targetPos, const float distToTarget);

			/**
			 * @brief 移動方向にアクティブな渦潮からの反発を織り込む
			 * @details 渦潮の回避半径内では渦潮から離れる向きの力を加算し、
			 *          子ペンギンが自分から吸い込み範囲へ入っていくのを防ぐ。
			 *          意図的に渦潮へ向かうとき（やんちゃのいたずら）は素通しする。
			 * @param moveDir 元の移動方向（正規化済み）
			 * @return 反発を織り込んだ移動方向（正規化済み）
			 */
			Vector3 ApplyWhirlpoolAvoidance(const Vector3& moveDir) const;

			/**
			 * @brief 意図的に渦潮へ向かっている最中かどうか
			 * @details やんちゃAIがいたずらダイブ中にtrueを返し、渦潮回避を無効化する。
			 * @return 意図的に渦潮へ向かっていればtrue
			 */
			virtual bool IsHeadingIntoWhirlpoolOnPurpose() const { return false; }

			/**
			 * @brief ステージの歩行可否グリッドを取得する
			 * @return 歩行可否グリッド。地形が無い（未構築の）ステージではnullptr
			 */
			StageNavGrid* GetStageNavGrid() const;


			/** かまくらイベントの更新処理 */
			void UpdateIglooEvent();

			/**
			 * @brief シロクマ逃走チェックと移動入力設定
			 * @details 自分を追跡中のエネミーが FLEE_DETECTION_DISTANCE 以内にいれば
			 *          エネミーと反対方向へダッシュ入力を設定してtrueを返す。
			 * @return 逃走行動中ならtrue（このフレームの通常AIをスキップする）
			 */
			bool CheckAndFlee();


			//------------------------------------------------------------//
			// 入隊判定
			// 以前は同じ距離判定が5タイプのAIに散らばっていた。
			// 判定を変えるときにここだけ直せば済むよう1箇所へ集約している。
			//------------------------------------------------------------//

			/**
			 * @brief 親を察知して隊列へ加われる状態かどうか
			 * @details 入隊条件はこの関数だけが持つ。各タイプのAIはここを呼ぶこと。
			 * @return 加われるならtrue
			 */
			bool CanJoinFormation() const;

			/**
			 * @brief 入隊条件を満たしていれば隊列へ加わる
			 * @details すでに隊列にいる場合は何もせずtrueを返す。
			 * @return このフレームの終わりに隊列にいるならtrue
			 */
			bool TryJoinFormation();

			/**
			 * @brief 親の隊列参加距離の内側にいるかどうか（入隊済みかは問わない）
			 * @details 「親のそばにいるか」を距離だけで見たいとき用。
			 *          CanJoinFormation() と違い、視界や向きの条件は入らない。
			 * @return 内側にいるならtrue
			 */
			bool IsDaddyWithinJoinRadius() const;

			/**
			 * @brief 隊列に入っていないときの移動入力を組み立てる
			 * @details 親に気づいていれば自分から寄っていき、気づいていなければその場で待つ。
			 *          待機命令中は気づいていても動かない（待機の意味を壊さないため）。
			 */
			void BuildInputWhenNotFollowing();


			//------------------------------------------------------------//
			// 親の察知（視界と音）
			//------------------------------------------------------------//

			/**
			 * @brief 親を察知しているかどうか
			 * @return 察知していればtrue
			 */
			bool HasNoticedDaddy() const { return m_hasNoticedDaddy; }

		private:
			/**
			 * @brief 親の察知状態を毎フレーム更新する
			 * @details Update() の先頭から呼ばれる。
			 *          距離 → 向き → 遮蔽 の順に重い判定へ進むので、
			 *          ほとんどの子は最初の距離判定で足切りされる。
			 */
			void UpdatePerception();

			/**
			 * @brief このフレームに親を「確定的に」知覚できているかを判定する
			 * @details 至近距離・再集合の呼びかけのどちらか。ここが真なら段階を踏まず即座に察知する。
			 * @return 知覚できていればtrue
			 */
			bool PerceiveDaddyStrongThisFrame();

			/**
			 * @brief このフレームに親の音が聞こえているかを判定する
			 * @details 音だけでは察知にならず、「？」を出して振り向く段階に入る。
			 * @return 聞こえていればtrue
			 */
			bool PerceiveDaddySoundThisFrame();

			/**
			 * @brief このフレームに親が視界に入っているかを判定する
			 * @details 距離 → 向き → 遮蔽 の順に、軽い判定から足切りする。
			 * @return 見えていればtrue
			 */
			bool PerceiveDaddySightThisFrame();

			/**
			 * @brief 察知の成立処理
			 * @details 未察知からの遷移なら「！」のリアクションを出す。
			 */
			void NoticeDaddy();

			/**
			 * @brief 親との間が地形で遮られているかを判定する
			 * @details レイキャストは OCCLUSION_CHECK_INTERVAL フレームに1回しか撃たず、
			 *          間のフレームは前回の結果を使い回す。
			 *          子ごとに実行フレームをずらしてあるので、
			 *          1フレームあたりのレイ本数は「子の数 / 間隔」が上限になる。
			 * @return 遮られていればtrue
			 */
			bool IsDaddyOccluded();


		protected:
			/** 子ペンギンのポインタ */
			ChildPenguin* m_owner;
			/** ステートマシンへの参照 */
			ChildPenguinStateMachine* m_stateMachine;
			/** 隊列（陣形）に参加しているかどうか */
			bool m_isFollowing = false;
			/** 停止とみなす距離（HYSTERESIS より十分大きい値にすること） */
			float m_stopDistance;
			/** 歩きの上限距離 */
			float m_walkDistance;
			/** 走りの上限距離（これを超えると滑りで追う） */
			float m_runDistance;
			/** かまくらイベント中かどうか */
			bool m_isEnterIglooMode = false;
			/** すでにかまくらの中に入ったか */
			bool m_isInsideIgloo = false;
			/** かまくらの入り口（青い円）の目標座標 */
			Vector3 m_iglooTargetPos = Vector3::Zero;
			/** エフェクトハンドル */
			EffectHandle m_hartEffectHandle;


		private:
			//------------------------------------------------------------//
			// 親の察知（視界と音）の状態
			//------------------------------------------------------------//

			/** タイプごとの察知性能（生成時に決まる。以降変わらない） */
			const DaddyPerceptionSpec* m_perceptionSpec = nullptr;
			/** 親を察知しているかどうか */
			bool m_hasNoticedDaddy = false;
			/**
			 * 親の音だけ聞こえている（まだ見つけてはいない）かどうか。
			 * この間は「？」を出しながらゆっくり親のほうへ振り向き、
			 * 視界に入った時点で察知（＝入隊）に変わる
			 */
			bool m_hasHeardDaddy = false;
			/**
			 * 再集合の呼びかけに応えてからの勇敢時間（秒）。
			 * 残っている間は「自分が狙われている」か「クマが至近」でない限り逃げない。
			 * 呼びかけの効果が切れた直後に群れがまた散るのを防ぐ
			 */
			float m_braveTimer = 0.0f;
			/** 知覚し続けている時間（noticeTime を超えると察知に変わる） */
			float m_noticeTimer = 0.0f;
			/** 知覚できなくなってからの時間（NOTICE_FORGET_TIME を超えると察知を忘れる） */
			float m_forgetTimer = 0.0f;
			/** 遮蔽レイキャストを撃つフレームをずらすためのスロット番号 */
			unsigned int m_perceptionSlot = 0;
			/** 前回の遮蔽レイキャストの結果（間のフレームで使い回す） */
			bool m_isDaddyOccludedCache = false;


		private:
			/**
			 * @brief 移動手段の内部フェーズ
			 * @details ヒステリシス制御のために保持する
			 */
			enum class MovePhase
			{
				Stop,    ///< 停止
				Walk,    ///< 歩き（Sneak）
				Run,     ///< 走り
				Slide,   ///< 滑り
			};

			/** 現在の移動フェーズ */
			MovePhase m_movePhase = MovePhase::Stop;

			/** 逃走検知距離（この距離以内の追跡エネミーがいると逃走行動に入る） */
			static constexpr float FLEE_DETECTION_DISTANCE = 300.0f;

			/**
			 * @brief 逃走方向を切り替えるまでの残り時間（秒）
			 * @details 0以下になると次の方向が抽選される
			 */
			float m_fleeDirChangeTimer = 0.0f;

			/**
			 * @brief 逃走方向に加えるY軸回転オフセット（ラジアン）
			 * @details 0のとき直進、±値のとき左右に振れる
			 */
			float m_fleeAngleOffset = 0.0f;

			/** 現在逃走中かどうか（プレイログの開始・終了を1回ずつ出すためのエッジ検出用） */
			bool m_isFleeing = false;
			/**
			 * @brief 逃走を続ける残り時間（秒）
			 * @details クマが近くにいる間は FLEE_HOLD_TIME で上書きされ続ける。
			 *          クマが離れてから0になるまでの間も逃げ続けるので、
			 *          「クマの脇をすり抜けた瞬間に立ち止まる」が起きない。
			 */
			float m_fleeHoldTimer = 0.0f;
			/**
			 * @brief 逃走後、隊列へ戻れるようになるまでの残り時間（秒）
			 * @details 散った直後にその場で入隊し直すと「集め直す時間」が生まれない。
			 *          親の再集合の呼びかけ（Yボタン）で0にできる。
			 */
			float m_rejoinCooldown = 0.0f;
			/** 最後に逃げる根拠にしたシロクマの座標（見失った後もこの向きへ逃げ続ける） */
			Vector3 m_lastThreatPos = Vector3::Zero;
		};




		/***********************************************
		 * 派生クラス
		 ***********************************************/


		 /**
		  * @brief まじめタイプの子ペンギンAI
		  * @details 追従命令→ついてくる、待機命令→その場待機
		  */
		class SeriousChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void UpdateAI() override;

		public:
			SeriousChildPenguinAI(ChildPenguin* owner);
			~SeriousChildPenguinAI() override = default;
		};




		/**************************************************************/


		/**
		 * @brief 甘えん坊タイプの子ペンギンAI
		 * @details 命令に関わらず常にDaddyに追従しようとする。制止されている間は待機する
		 */
		class ClingyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void UpdateAI() override;

			/**
			 * @brief 世話焼きペンギンによる制止フラグを設定する
			 * @detail 制止された瞬間(false→true)にリアクションUIへの通知を行う
			 * @param isRestrained 制止フラグ
			 */
			void SetRestrained(const bool isRestrained);

			/**
			 * @brief 制止中かどうかを取得する
			 * @return 制止中ならtrue
			 */
			inline bool IsRestrained() const
			{
				return m_isRestrained;
			}

		public:
			ClingyChildPenguinAI(ChildPenguin* owner);
			~ClingyChildPenguinAI() override;

		private:
			/** 世話焼きペンギンに制止されているかどうか */
			bool m_isRestrained = false;
			/** 甘えん坊専用エフェクトハンドル */
			EffectHandle m_clingyEffectHandle;
			/** エフェクトの再生時間 */
			float m_effectInterval = 0.0f;
		};




		/**************************************************************/


		/**
		 * @brief やんちゃタイプの子ペンギンAI
		 * @details 追従命令→ついてくる、待機命令かつ親が一定距離以上離れたら→徘徊する
		 */
		class NaughtyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void UpdateAI() override;

			/**
			 * @brief 世話焼きペンギンによる制止フラグを設定する
			 * @detail 制止された瞬間(false→true)にリアクションUIへの通知を行う
			 * @param isRestrained 制止フラグ
			 */
			void SetRestrained(const bool isRestrained);

			/**
			 * @brief 制止中かどうかを取得する
			 * @return 制止中ならtrue
			 */
			inline bool IsRestrained() const
			{
				return m_isRestrained;
			}

		public:
			NaughtyChildPenguinAI(ChildPenguin* owner);
			~NaughtyChildPenguinAI() override;

			/**
			 * @brief 意図的に渦潮へ向かっている最中かどうか
			 * @details いたずらダイブ中は基底の渦潮回避を無効化する。
			 * @return 渦潮いたずらへ向かっていればtrue
			 */
			bool IsHeadingIntoWhirlpoolOnPurpose() const override;

		private:
			/**
			 * @brief 次の徘徊目標座標をランダムに選ぶ
			 */
			void PickNewRoamTarget();

			/**
			 * @brief スタック（移動の意思があるのに動けていない）を検出して解消する
			 * @details 急斜面（接地限界63度超）へ向かって歩き続けると押し戻されて
			 *          足踏みになるため、一定時間動けていなければ行き先を変える。
			 *          いたずら（クマ起こし・渦潮）へ向かう途中なら諦めさせる。
			 */
			void UpdateStuckWatch();

			/**
			 * @brief 進めなかった方向の反対側から徘徊目標を選び直す
			 * @details PickNewRoamTarget() だと同じ斜面方向を引き直す可能性があるため、
			 *          スタック解消時はこちらを使う。
			 * @param blockedDir 進めなかった移動方向
			 */
			void PickNewRoamTargetAwayFromBlocked(const Vector3& blockedDir);

			/**
			 * @brief わいわいエフェクトの再生
			 */
			void PlayLivelyEffect();

			/**
			 * @brief わいわいエフェクトの停止
			 */
			void StopLivelyEffect();

		private:
			/** やんちゃ固有ステートマシンへのポインタ（キャスト済みのキャッシュ） */
			NaughtyChildPenguinStateMachine* m_naughtyStateMachine = nullptr;
			/** 世話焼きペンギンに制止されているかどうか */
			bool m_isRestrained = false;
			/** 徘徊先の目標座標 */
			Vector3 m_roamTarget = Vector3::Zero;
			/** 待機命令中に徘徊を開始する親との距離 */
			float m_roamTriggerDistance = 0.0f;
			/** 徘徊先を選ぶ現在地からの半径 */
			float m_roamRadius = 0.0f;
			/** スタック検出：動けていない時間（秒） */
			float m_stuckTimer = 0.0f;
			/** スタック検出：前回チェック時の座標 */
			Vector3 m_stuckCheckPos = Vector3::Zero;
			/** 反省時間 */
			float m_scoldCooldown = 0.0f;
			/** 渦潮に飲み込まれたかどうかのフラグ */
			bool m_wasSwallowedByWhirlpool = false;
			/** わいわいエフェクトのインターバル */
			float m_livelyInterval = 0.0f;
			/** わいわいエフェクト */
			EffectHandle m_livelyEffectHandle = INVALID_EFFECT_HANDLE;
		};




		/**************************************************************/


		/**
		 * @brief おっちょこちょいタイプの子ペンギンAI
		 * @details 歩き・走り中に確率で転倒し、スライド解除時に確率でスリップする
		 */
		class ClumsyChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void UpdateAI() override;

			/**
			 * @brief 世話焼きペンギンに助けてもらう
			 * @note 転倒・スリップ中に呼ばれるとアニメ終了を待たず即座に起き上がる
			 */
			void HelpedByCaringPenguin();

		public:
			ClumsyChildPenguinAI(ChildPenguin* owner);
			~ClumsyChildPenguinAI() override = default;

		private:
			/** おっちょこちょい固有ステートマシンへのポインタ（キャスト済みのキャッシュ） */
			ClumsyChildPenguinStateMachine* m_clumsyStateMachine = nullptr;
			/** 歩き・走り中の転倒確率（秒あたり） */
			float m_tripChancePerSec = 0.0f;
			/** スライド解除時のスリップ確率 */
			float m_slipChance = 0.0f;
			/** 前フレームにスライド中だったかどうか（スライド解除の検出に使う） */
			bool m_wasSliding = false;
			/** 前フレームに転倒・スリップ中(Downing)だったかどうか（起き上がりの検出に使う） */
			bool m_wasDowning = false;
			/** 今回の転倒・スリップ中に世話焼きペンギンに助けてもらったかどうか */
			bool m_wasHelpedThisDowning = false;
		};




		/**************************************************************/


		/**
		 * @brief 世話焼きタイプの子ペンギンAI
		 * @details 待機命令中に問題を起こしている子ペンギンに近づいて制止・助けを行う
		 */
		class CaringChildPenguinAI : public ChildPenguinAIController
		{
		public:
			void UpdateAI() override;

		public:
			CaringChildPenguinAI(ChildPenguin* owner);
			~CaringChildPenguinAI() override;

		private:
			/**
			 * @brief 介入対象への到達判定
			 * @param target 介入対象
			 * @return 十分近ければtrue
			 */
			bool IsCloseEnoughTo(const ChildPenguin* target) const;

			/**
			 * @brief 介入対象に制止・助けの処理を適用する
			 * @param target 介入対象
			 */
			void ApplyIntervention(ChildPenguin* target) const;

			/**
			 * @brief 制止していた対象への制止を解除する
			 * @param target 制止を解除する対象
			 */
			void ReleaseSuppression(ChildPenguin* target) const;


		private:
			/**
			 * @brief 介入時にエフェクトを再生する
			 * @details 対象ペンギンの頭上に汗エフェクト
			 */
			void PlayCaringEffect() const;

			/**
			 * @brief 再生中の汗エフェクトをすべて停止し、カウンターをリセットする
			 */
			void StopAllCaringEffects() const;

			/**
			 * @brief 介入対象の変化を検出してプレイログへイベントを出す
			 * @details 介入対象は Update() の複数の経路で設定・解除されるため、
			 *          各所に記録を仕込まず、前フレームの対象と比較するエッジ検出で拾う。
			 *          世話焼きが実際に働いているかを測るために使う
			 */
			void UpdateInterventionLog();

			/** 現在介入中の対象ペンギン */
			ChildPenguin* m_interventionTarget = nullptr;
			/** 前フレームにログへ記録した介入対象（エッジ検出用） */
			ChildPenguin* m_loggedInterventionTarget = nullptr;
			/** 介入を開始した時刻（残り秒数） */
			float m_interventionStartTime = 0.0f;
			/** 現在の介入で対象まで到達できたか（空振りとの区別に使う） */
			bool m_hasReachedInterventionTarget = false;
			/**
			 * @brief 介入対象を探す最大距離（JSONパラメーターから読み込む）
			 * @details この距離より遠いペンギンには介入しない
			 */
			float m_interventionRange = 0.0f;
			/** 介入到達とみなす距離 */
			static constexpr float INTERVENTION_REACH_DISTANCE = 25.0f;
			/** 複数回連続で流すための制御値 */
			mutable float m_sweatEffectCoolTime = 0.0f;
			/** 汗エフェクトハンドル（MAX_SWEAT_COUNT 本分を保持） */
			mutable std::array<EffectHandle, 3> m_caringEffectHandles;
			/** 汗エフェクトの回数制御値 */
			mutable int m_sweatEffectCount = 0;
		};
	}
}