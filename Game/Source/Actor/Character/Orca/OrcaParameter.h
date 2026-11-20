/**
 * @file OrcaParameter.h
 * @brief シャチのパラメーター
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief シャチのパラメーター
		 * @details OrcaParameter.json を Tools/ParameterConvert.py の pack_orca_parameter で
		 *          バイナリにしたものを、メンバーの並び順のまま読み込む。
		 *          並びを変えたら pack_orca_parameter も同じ順に直すこと。
		 *          全項目 float、項目数は偶数（構造体末尾の詰め物でずれないようにするため）。
		 *          カッコ内はシロクマの値と、その出どころ。
		 */
		struct MasterOrcaParameter : public MasterCharacterParameter
		{
			appParameter(MasterOrcaParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// 基底: runSpeed（追跡の速さ）, swimSpeed（巡回・探索の速さ）, radius, height（物理ボディを持たないので未使用）

			/** 見た目の大きさ（モデルは原寸で読み込み、この倍率で縮める） */
			float modelScale;
			/** 旋回の速さ（度/秒） */
			float turnSpeedDeg;
			/** 速さの追従の早さ */
			float acceleration;
			/** 探索中の旋回の倍率（0.25。EnemyController.cpp SEARCH_TURN_MULTIPLIER） */
			float searchTurnMultiplier;
			/** 視野の半角（70度。EnemyController.cpp VIEW_ANGLE） */
			float viewAngleDeg;
			/** 視界の距離（600。EnemyController.cpp DIST_TO_IDLE_SQ） */
			float viewDistance;
			/** 音に気づくしきい値（15。EnemyController.cpp SEARCH_THRESHOLD） */
			float noiseThreshold;
			/** 口先からの攻撃距離（80。EnemyController.cpp NORMAL_ATTACK_DIST） */
			float attackDistance;
			/** 原点から口先までの前方距離（modelScale を掛ける前の値ではなく、縮めた後の値） */
			float mouthOffset;
			/** 攻撃開始から食べた数を増やすまで（0.5。EnemyIState.cpp ATTACK_IMPACT_TIME） */
			float attackImpactTime;
			/** 「!」から追跡開始まで（シロクマは咆哮アニメの長さ） */
			float roarDuration;
			/** 見失う距離（700。EnemyParameter.json） */
			float lostChaseDistance;
			/** 満腹になる数（3。使うときに整数へ直す） */
			float maxEat;
			/** スタミナの上限（30） */
			float maxStamina;
			/** 追跡中に毎秒減るスタミナ（1.0） */
			float staminaDrainRate;
			/** 待機時間の上限。0〜この値の乱数（5秒。EnemyController.cpp RANDOM_IDLE_TIME_*） */
			float idleTimeMax;
			/** 巡回点の到達判定（120。EnemyController.cpp ARRIVE_DIST_WANDERING） */
			float patrolArriveDistance;
			/** 探索を諦める秒数（3.0。EnemyController.cpp GIVE_UP_SEARCH_TIME） */
			float searchGiveUpTime;
			/** 待機・巡回・探索・咆哮・追跡の深さ（背びれだけ出る） */
			float swimDepth;
			/** 噛みつきの深さ（attackPitchDeg と合わせて口が水面に出るようにする） */
			float attackDepth;
			/** 気絶の秒数（2.0。EnemyIState.cpp STUN_DURATION） */
			float stunDuration;
			/** 攻撃を無効化されたときに弾かれる距離（150。EnemyController.cpp NULLIFY_KNOCKBACK_DISTANCE） */
			float nullifyKnockbackDistance;
			/** 噛みつくときに頭を持ち上げる角度（度。口を水面の上に出すため） */
			float attackPitchDeg;
			/** 噛みついたあと、水面に出たまま止まる時間（秒）。過ぎたら沈み始める */
			float attackHoldTime;
			/** 噛みつくときに浮上して頭を持ち上げきるまでの時間（秒。イーズインアウト。attackImpactTime と合わせる） */
			float attackRiseTime;
			/** 噛みついたあと、背びれだけの深さに戻るまでの時間（秒。イーズインアウト） */
			float attackSinkTime;
		};

		/** バイナリの並びと構造体の大きさが合っているかの確認（基底4 + シャチ26 = 30項目 × 4バイト） */
		static_assert(sizeof(MasterOrcaParameter) - sizeof(core::IMasterParameter) == 120,
			"MasterOrcaParameter の大きさが OrcaParameter.bin と合いません。pack_orca_parameter と並びを確認してください");
	}
}
