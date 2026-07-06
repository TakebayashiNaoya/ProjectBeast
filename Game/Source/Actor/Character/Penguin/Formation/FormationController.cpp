/**
 * @file FormationController.cpp
 * @brief 陣形の切り替えと座標計算を管理するコントローラー
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationController.h"
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
					p.ultDuration                 = util::JsonConverter::ToFloat(j, "ultDuration");
					p.ultCooldown                 = util::JsonConverter::ToFloat(j, "ultCooldown");
					p.passiveSpeedMultiplier      = util::JsonConverter::ToFloat(j, "passiveSpeedMultiplier");
					p.passiveSpeedBase            = util::JsonConverter::ToFloat(j, "passiveSpeedBase");
					p.passiveSpeedPerLevel        = util::JsonConverter::ToFloat(j, "passiveSpeedPerLevel");
					p.passiveWhirlpoolResistance  = util::JsonConverter::ToBool(j, "passiveWhirlpoolResistance");
					p.ultSpeedMultiplier          = util::JsonConverter::ToFloat(j, "ultSpeedMultiplier");
					p.ultWhirlpoolResistance      = util::JsonConverter::ToBool(j, "ultWhirlpoolResistance");
					p.ultWhirlpoolBoostMultiplier = util::JsonConverter::ToFloat(j, "ultWhirlpoolBoostMultiplier");
					p.ultBearAttackNullify        = util::JsonConverter::ToBool(j, "ultBearAttackNullify");
					p.ultCallDistance             = util::JsonConverter::ToFloat(j, "ultCallDistance");
					p.radiusPerRing               = util::JsonConverter::ToFloat(j, "radiusPerRing");
					p.joinMargin                  = util::JsonConverter::ToFloat(j, "joinMargin");
					p.rowSpacing                  = util::JsonConverter::ToFloat(j, "rowSpacing");
					p.colSpacing                  = util::JsonConverter::ToFloat(j, "colSpacing");
					p.baseFollowers               = util::JsonConverter::ToInt(j, "baseFollowers");
				}
			);
#else
			// リリースビルドは変換済みバイナリを読み込む（軽量・高速。ホットリロードは非対応）
			core::ParameterManager::Get()->LoadParameterBinary<MasterFormationParameter>(PARAMETER_BINARY_FILE_PATH);
#endif

			auto* parameterManager = core::ParameterManager::Get();
			const auto* circleParam   = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Circle));
			const auto* triangleParam = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Triangle));
			const auto* clusterParam  = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Cluster));
			const auto* scatterParam  = parameterManager->GetParameter<MasterFormationParameter>(static_cast<int>(EnFormationType::Scatter));

			m_formations[static_cast<size_t>(EnFormationType::Circle)]   = std::make_unique<CircleFormation>(*circleParam);
			m_formations[static_cast<size_t>(EnFormationType::Triangle)] = std::make_unique<TriangleFormation>(*triangleParam);
			m_formations[static_cast<size_t>(EnFormationType::Cluster)]  = std::make_unique<ClusterFormation>(*clusterParam);
			m_formations[static_cast<size_t>(EnFormationType::Scatter)]  = std::make_unique<ScatterFormation>(*scatterParam);

			SwitchFormation(EnFormationType::Circle);

			// ゲーム開始直後はウルトが貯まっていない状態から始める
			m_ultController.ResetCooldown();

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
			const int newLevel = effective / FOLLOWERS_PER_LEVEL;
			if (newLevel > m_formationLevel && m_onLevelUp){
				m_onLevelUp(newLevel);
			}
			m_formationLevel = newLevel;
		}


		void FormationController::SwitchFormation(EnFormationType type)
		{
			m_currentType      = type;
			m_currentFormation = m_formations[static_cast<size_t>(type)].get();

			// 陣形切り替えと同時にウルトチェーンも差し替える
			m_ultController.SetUlt(
				&m_currentFormation->GetUlt(),
				m_currentFormation->GetUltDuration(),
				m_currentFormation->GetUltCooldown()
			);
		}


		float FormationController::GetSpeedMultiplier() const
		{
			if (!m_currentFormation) return 1.0f;

			// パッシブ倍率 × (ウルト中ならウルト倍率)
			float speed = m_currentFormation->GetPassive().GetSpeedMultiplier(m_formationLevel);
			if (m_ultController.IsActive())
			{
				speed *= m_currentFormation->GetUlt().GetSpeedMultiplier(m_formationLevel);
			}
			return speed;
		}


		bool FormationController::HasWhirlpoolResistance() const
		{
			if (!m_currentFormation) return false;

			// パッシブ耐性 OR (ウルト中かつウルト耐性)
			if (m_currentFormation->GetPassive().HasWhirlpoolResistance()) return true;
			return m_ultController.IsActive()
				&& m_currentFormation->GetUlt().HasWhirlpoolResistance();
		}


		float FormationController::GetJoinRadius(int count) const
		{
			return m_currentFormation ? m_currentFormation->GetJoinRadius(count) : 0.0f;
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

			// 現在充填中のリング k の末尾インデックスを求める
			// リング k のスロット数 = k * FOLLOWERS_PER_LEVEL
			// リング 1〜k-1 の累計 = 1+2+...+(k-1) のスロット数 = ringStart
			int ringStart = 0;
			int k = 1;
			while (ringStart + k * FOLLOWERS_PER_LEVEL <= occupied)
			{
				ringStart += k * FOLLOWERS_PER_LEVEL;
				k++;
			}
			const int nextCount = ringStart + k * FOLLOWERS_PER_LEVEL;

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
			return m_currentFormation
				? m_currentFormation->GetJoinRadius()
				: 0.0f;
		}
	}
}
