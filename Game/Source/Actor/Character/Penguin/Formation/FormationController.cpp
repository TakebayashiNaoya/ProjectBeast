/**
 * @file FormationController.cpp
 * @brief 陣形の切り替えと座標計算を管理するコントローラー
 */
#include "stdafx.h"
#include "FormationController.h"
#include "FormationContext.h"
#include "Formations.h"
#include "MasterFormationParameter.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** 陣形パラメーターのファイルパス（ホットリロード時: JSONを直接読み込む） */
			constexpr const char* PARAMETER_JSON_FILE_PATH = "Assets/parameter/character/penguin/formation/FormationParameter.json";
			/** 陣形パラメーターのファイルパス（リリース時: 変換済みバイナリを読み込む） */
			constexpr const char* PARAMETER_BINARY_FILE_PATH = "Assets/parameter/character/penguin/formation/FormationParameter.bin";
			/** 陣形切り替え演出（スライド）時間のチューニングファイルパス（ホットリロード対応） */
			constexpr const char* SWITCH_TUNING_JSON_PATH = "Assets/parameter/character/penguin/formation/FormationSwitchTuning.json";

			/**
			 * @brief フォロワー数から陣形レベルと現在進行中のリングの内訳を求める
			 * @details リング k（1始まり）は followersPerLevel*k 人で満員になる。
			 *          レベルは満員になったリングの数（= 完了したリング数）。
			 * @param count 現在のフォロワー数
			 * @param followersPerLevel リング1個あたりの基準人数
			 * @param outRingStart 現在進行中のリングが始まった時点の累計人数
			 * @param outRingSize 現在進行中のリングの満員人数
			 * @return 陣形レベル（0始まり）
			 */
			int ComputeFormationLevel(int count, int followersPerLevel, int& outRingStart, int& outRingSize)
			{
				int ringStart = 0;
				int k = 1;
				while (ringStart + k * followersPerLevel <= count)
				{
					ringStart += k * followersPerLevel;
					k++;
				}
				outRingStart = ringStart;
				outRingSize  = k * followersPerLevel;
				return k - 1;
			}
		}


		FormationController::~FormationController()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterFormationParameter>();
		}


		FormationController::FormationController()
		{
			// 外部ファイルを読み込み（インデックスは EnFormationType の値と対応する）
#if defined(APP_PARAM_HOT_RELOAD)
			// ホットリロード有効時はJSONを直接読み込む。保存するだけで即座に反映される（変換不要）
			core::ParameterManager::Get()->LoadParameter<MasterFormationParameter>(
				PARAMETER_JSON_FILE_PATH,
				[](const nlohmann::json& j, MasterFormationParameter& p)
				{
					p.passiveSpeedMultiplier      = util::JsonConverter::ToFloat(j, "passiveSpeedMultiplier");
					p.passiveSpeedBase            = util::JsonConverter::ToFloat(j, "passiveSpeedBase");
					p.passiveSpeedPerLevel        = util::JsonConverter::ToFloat(j, "passiveSpeedPerLevel");
					p.passiveWhirlpoolResistance  = util::JsonConverter::ToBool(j, "passiveWhirlpoolResistance");
					p.passiveBearAttackNullify    = util::JsonConverter::ToBool(j, "passiveBearAttackNullify");
					p.radiusPerRing               = util::JsonConverter::ToFloat(j, "radiusPerRing");
					p.joinMargin                  = util::JsonConverter::ToFloat(j, "joinMargin");
					p.rowSpacing                  = util::JsonConverter::ToFloat(j, "rowSpacing");
					p.colSpacing                  = util::JsonConverter::ToFloat(j, "colSpacing");
					p.passiveFetchRadiusBase      = util::JsonConverter::ToFloat(j, "passiveFetchRadiusBase");
					p.passiveFetchRadiusPerLevel  = util::JsonConverter::ToFloat(j, "passiveFetchRadiusPerLevel");
					p.passiveFetchSpeedMultiplier = util::JsonConverter::ToFloat(j, "passiveFetchSpeedMultiplier");
					p.passiveFetchTimeLimit       = util::JsonConverter::ToFloat(j, "passiveFetchTimeLimit");
					p.baseFollowers               = util::JsonConverter::ToInt(j, "baseFollowers");
				}
			);
#else
			// リリースビルドは変換済みバイナリを読み込む（軽量・高速。ホットリロードは非対応）
			core::ParameterManager::Get()->LoadParameterBinary<MasterFormationParameter>(PARAMETER_BINARY_FILE_PATH);
#endif

			auto* parameterManager = core::ParameterManager::Get();
			const auto* speedParam   = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Speed));
			const auto* defenseParam = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Defense));
			const auto* gatherParam  = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Gather));

			m_formations[static_cast<size_t>(EnFormationType::Speed)]   = std::make_unique<SpeedFormation>(*speedParam);
			m_formations[static_cast<size_t>(EnFormationType::Defense)] = std::make_unique<DefenseFormation>(*defenseParam);
			m_formations[static_cast<size_t>(EnFormationType::Gather)]  = std::make_unique<GatherFormation>(*gatherParam);

			SwitchFormation(INITIAL_FORMATION_TYPE);

			// 陣形切り替え演出時間の初期読み込み
			{
				nlohmann::json tuningJson;
				if (util::JsonConverter::IsLoadJsonFile(tuningJson, SWITCH_TUNING_JSON_PATH))
				{
					m_switchLockDuration = util::JsonConverter::ToFloat(tuningJson, "switchDuration", m_switchLockDuration);
				}
#if defined(APP_DEBUG)
				m_tuningLastWriteTime = util::JsonConverter::GetFileLastWriteTime(SWITCH_TUNING_JSON_PATH);
#endif
			}
		}


		void FormationController::ReloadSwitchTuningIfChanged()
		{
#if defined(APP_DEBUG)
			if (!util::JsonConverter::CheckFileModified(SWITCH_TUNING_JSON_PATH, m_tuningLastWriteTime)) return;

			nlohmann::json tuningJson;
			if (!util::JsonConverter::IsLoadJsonFile(tuningJson, SWITCH_TUNING_JSON_PATH)) return;

			m_switchLockDuration  = util::JsonConverter::ToFloat(tuningJson, "switchDuration", m_switchLockDuration);
			m_tuningLastWriteTime = util::JsonConverter::GetFileLastWriteTime(SWITCH_TUNING_JSON_PATH);
#endif
		}


		void FormationController::CalculatePositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int count,
			int countForLevel
		)
		{
			if (m_currentFormation == nullptr) return;

			// 現在の陣形に座標計算を委譲（count は m_outerRadius の算出に使われる）
			m_currentFormation->CalculatePositions(center, forward, out, count);

			// レベル判定は countForLevel で行う（-1 の場合は count を使う）
			const int effective = (countForLevel < 0) ? count : countForLevel;
			int ringStart = 0;
			int ringSize  = 0;
			const int newLevel = ComputeFormationLevel(effective, FOLLOWERS_PER_LEVEL, ringStart, ringSize);
			if (newLevel > m_formationLevel)
			{
				if (m_onLevelUp)
				{
					m_onLevelUp(newLevel);
				}

				// 序盤の空白（初回レベルアップまでの時間）を実測するために記録する
				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->QueueEvent({
						{ "ev",        "formation_levelup" },
						{ "level",     newLevel },
						{ "followers", effective }
					});
				}
			}
			m_formationLevel   = newLevel;
			m_ringProgress     = effective - ringStart;
			m_ringRequirement  = ringSize;
		}


		void FormationController::SwitchFormation(EnFormationType type)
		{
			// 実際に切り替わったときだけ記録する（初期化時の同一陣形へのセットは対象外）
			const bool isChanged = (m_currentFormation != nullptr) && (m_currentType != type);
			const EnFormationType prevType = m_currentType;

			m_currentType      = type;
			m_currentFormation = m_formations[static_cast<size_t>(type)].get();

			if (isChanged)
			{
				if (auto* lm = GameLogManager::GetInstance())
				{
					lm->QueueEvent({
						{ "ev",   "formation_switch" },
						{ "from", FormationTypeName(prevType) },
						{ "to",   FormationTypeName(type) }
					});
				}
			}
		}


		void FormationController::UpdatePassive(float dt, const FormationContext& ctx)
		{
			if (!m_currentFormation) return;

			// 陣形が変わったら、旧陣形のパッシブを抜けてから新陣形のパッシブに入る
			if (m_passiveFormation != m_currentFormation)
			{
				if (m_passiveFormation)
				{
					m_passiveFormation->GetPassive().Exit(ctx);
					if (IFormationVisual* visual = m_passiveFormation->GetPassiveVisual()) visual->Exit(ctx);
				}
				m_passiveFormation = m_currentFormation;
				m_passiveFormation->GetPassive().Enter(ctx);
				if (IFormationVisual* visual = m_passiveFormation->GetPassiveVisual()) visual->Enter(ctx);
			}

			m_passiveFormation->GetPassive().Update(dt, ctx);
			if (IFormationVisual* visual = m_passiveFormation->GetPassiveVisual()) visual->Update(dt, ctx);
		}


		float FormationController::GetSpeedMultiplier() const
		{
			if (!m_currentFormation) return 1.0f;

			return m_currentFormation->GetPassive().GetSpeedMultiplier(m_formationLevel);
		}


		bool FormationController::HasWhirlpoolResistance() const
		{
			if (!m_currentFormation) return false;

			return m_currentFormation->GetPassive().HasWhirlpoolResistance();
		}


		float FormationController::GetJoinRadius(int count) const
		{
			if (!m_currentFormation) return 0.0f;

			return m_currentFormation->GetJoinRadius(count);
		}


		void FormationController::CalculateNextLevelPositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int occupied
		)
		{
			if (!m_currentFormation) return;

			// 現在の m_outerRadius を保存（呼び出し後に復元する）
			const float savedRadius = m_currentFormation->GetOuterRadius();

			// 現在充填中のリングの末尾インデックス（= リングが満員になったときの累計人数）を求める
			int ringStart = 0;
			int ringSize  = 0;
			ComputeFormationLevel(occupied, FOLLOWERS_PER_LEVEL, ringStart, ringSize);
			const int nextCount = ringStart + ringSize;

			out.clear();
			m_currentFormation->CalculatePositions(center, forward, out, nextCount);
			m_currentFormation->SetOuterRadius(savedRadius);
		}


		float FormationController::GetOuterRadius() const
		{
			return m_currentFormation
				? m_currentFormation->GetOuterRadius()
				: 0.0f;
		}


		float FormationController::GetJoinRadius() const
		{
			if (!m_currentFormation) return 0.0f;

			return m_currentFormation->GetJoinRadius();
		}
	}
}
