/**
 * @file GameLogManager.cpp
 * @brief ゲームプレイログの記録・出力管理クラス
 */
#include "stdafx.h"
#include "GameLogManager.h"
#include "LogCompression.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinController.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Manager/TimeManager.h"
#include "Source/Camera/CameraManager.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>


namespace app
{
	namespace
	{
		/**
		 * @brief 記録用に数値を丸める（ログファイルサイズ削減）
		 * @details float値をnlohmann::jsonへ代入するとnumber_float_t（double）へ暗黙昇格するため、
		 *          json側のシリアライズ（丸め誤差なく復元できる最短の10進表現を選ぶ方式）が、
		 *          元のfloatの精度ではなくdoubleの精度で桁数を決めてしまい、
		 *          "1572.363037109375" のように再現に不要な桁まで書き出されてしまう。
		 *          丸め計算自体をfloat精度で行うと、丸めた"つもり"の値がdoubleへ昇格する際に
		 *          誤差が復活し "0.09100000560283661" のようになってしまうため、
		 *          丸め計算はdouble精度で行い、結果もdoubleのまま返す（floatへ戻さない）
		 * @param v         丸める値
		 * @param precision 丸めの単位（例: 0.01なら小数第2位まで）
		 */
		double RoundForLog(float v, double precision)
		{
			return std::round(static_cast<double>(v) / precision) * precision;
		}

		/** Quaternion の Apply で進行方向を取得し Yaw 角度（度）を返す（整数度に丸める。1度未満は見た目に影響しない） */
		double GetYawDeg(const Quaternion& rot)
		{
			Vector3 forward = Vector3::AxisZ;
			rot.Apply(forward);
			const float deg = atan2f(forward.x, forward.z) * (180.0f / 3.14159265f);
			return RoundForLog(deg, 1.0);
		}

		/** Vector3 を json 配列に変換（座標は数百〜数千単位のため整数に丸めても見た目に影響しない） */
		nlohmann::json V3toJson(const Vector3& v)
		{
			return nlohmann::json::array({
				RoundForLog(v.x, 1.0),
				RoundForLog(v.y, 1.0),
				RoundForLog(v.z, 1.0)
			});
		}

		/** 渦潮の状態名を返す */
		const char* WhirlpoolStateName(nature::Whirlpool::EnWhirlpoolState s)
		{
			switch (s)
			{
			case nature::Whirlpool::EnWhirlpoolState::Bigger:  return "Bigger";
			case nature::Whirlpool::EnWhirlpoolState::Stay:    return "Stay";
			case nature::Whirlpool::EnWhirlpoolState::Smaller: return "Smaller";
			default:                                            return "None";
			}
		}
	}


	GameLogManager* GameLogManager::m_instance = nullptr;


	GameLogManager::GameLogManager()
		: m_frameCount(0)
	{
		m_sessionId = MakeSessionId();
	}


	void GameLogManager::RecordTick(actor::DaddyPenguin* daddy)
	{
		if (m_ticks.size() >= MAX_TICKS) return;

		nlohmann::json tick = BuildTickSnapshot(daddy);
		tick["type"]   = "tick";
		tick["frame"]  = m_frameCount;
		tick["t"]      = GetGameTime();
		tick["events"] = std::move(m_pendingEvents);
		m_pendingEvents.clear();

		m_ticks.push_back(std::move(tick));
		m_frameCount++;
	}


	void GameLogManager::QueueEvent(nlohmann::json event)
	{
		event["t"] = GetGameTime();
		m_pendingEvents.push_back(std::move(event));
	}


	void GameLogManager::RecordSpawn(const std::string& entity, int id,
		nlohmann::json extraData)
	{
		extraData["type"]   = "spawn";
		extraData["t"]      = GetGameTime();
		extraData["entity"] = entity;
		extraData["id"]     = id;
		m_ticks.push_back(std::move(extraData));
	}


	void GameLogManager::RecordDespawn(const std::string& entity, int id,
		const std::string& cause)
	{
		nlohmann::json rec;
		rec["type"]   = "despawn";
		rec["t"]      = GetGameTime();
		rec["entity"] = entity;
		rec["id"]     = id;
		if (!cause.empty()) rec["cause"] = cause;
		m_ticks.push_back(std::move(rec));
	}


	void GameLogManager::SetStageConfig(nlohmann::json config)
	{
		m_stageConfig = std::move(config);
	}


	void GameLogManager::SetResultDetail(nlohmann::json detail)
	{
		m_resultDetail = std::move(detail);
	}


	void GameLogManager::Flush(const std::string& stage,
		float durationSec,
		int   rescued,
		float score)
	{
		// --- ディレクトリ作成 ---
		const std::string dir = "Logs/" + m_sessionId;
		std::filesystem::create_directories(dir);

		// --- session.json ---
		{
			nlohmann::json session;
			session["session_id"]            = m_sessionId;
			session["stage"]                 = stage;
			session["duration_sec"]          = durationSec;

			// どの配合で取ったログかをログ自身に残す。これが無いと配合違いの比較ができない
			if (!m_stageConfig.is_null())
			{
				session["stage_config"] = m_stageConfig;
			}

			session["result"]["rescued"]     = rescued;
			session["result"]["score"]       = score;

			// アチーブメントの達成状況などをリザルトへ統合する
			if (m_resultDetail.is_object())
			{
				session["result"].update(m_resultDetail);
			}

			std::ofstream ofs(dir + "/session.json");
			ofs << session.dump(2) << "\n";
		}

		// --- ticks.jsonl（圧縮して .cmp として書き出す） ---
		// 記録件数が多いログは非圧縮で数十MBになるため、書き出し時に丸ごと圧縮する。
		// ReplayScene側は選んだセッションを読み込む瞬間に1回展開するだけで、
		// 再生中はメモリ上に展開済みのデータを使うため負荷への影響はない
		{
			std::string jsonl;
			for (const auto& rec : m_ticks)
			{
				jsonl += rec.dump();
				jsonl += '\n';
			}

			const std::vector<uint8_t> compressed = CompressLogData(jsonl);
			std::ofstream ofs(dir + "/ticks.jsonl.cmp", std::ios::binary);
			ofs.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
		}

		m_ticks.clear();
		m_pendingEvents.clear();
		m_stageConfig = nullptr;
		m_resultDetail = nullptr;
		m_frameCount = 0;
	}


	float GameLogManager::GetGameTime() const
	{
		// "t"は全レコード（tick・spawn・despawn・event）に付与されファイル全体で頻出するため、
		// 整数秒に丸めてログサイズを抑える（HUDのタイマー表示・タイムライン算出には十分な精度）。
		// 整数値はfloatでも誤差なく表現できるため、floatに戻してもdouble昇格時の桁化けは起きない
		return static_cast<float>(RoundForLog(TimeManager::GetInstance().GetCurTime(), 1.0));
	}


	nlohmann::json GameLogManager::BuildTickSnapshot(actor::DaddyPenguin* daddy) const
	{
		nlohmann::json snap;

		// ------ 親ペンギン ------
		// フィールド名をキーとして毎tick書き出すとログサイズが膨れ上がる（数百体分×数千tick）ため、
		// 固定順の配列で記録する。読み込み側（ReplayScene）はオブジェクト形式の旧ログも
		// 判別して読めるようにしてあるため、過去に記録したログも引き続き再生できる
		// 配列レイアウト: [pos, rot_y, state, clingy_count, speed_mul]
		// clingy_count/speed_mul は後から足した要素。読み込み側は範囲チェックしてから読むため、
		// これらを持たない旧ログもそのまま再生できる
		if (daddy)
		{
			auto* sm = daddy->GetStateMachine();
			auto* ctrl = daddy->GetController();
			const auto& tf = daddy->GetTransform();
			snap["parent"] = nlohmann::json::array({
				V3toJson(tf.m_position),
				GetYawDeg(tf.m_rotation),
				sm ? sm->GetStateNameForLog() : "Unknown",
				ctrl ? ctrl->GetClingyCount() : 0,
				RoundForLog(ctrl ? ctrl->GetSpeedMultiplier() : 1.0f, 0.01)
			});
		}

		// ------ シロクマ ------
		// 配列レイアウト: [id, pos, rot_y, state, sleep_timer]
		auto* em = actor::EnemyManager::GetInstance();
		nlohmann::json bearsArr = nlohmann::json::array();
		if (em)
		{
			const auto& enemies = em->GetEnemies();
			for (size_t i = 0; i < enemies.size(); i++)
			{
				auto* enemy = enemies[i];
				if (!enemy) continue;
				auto* sm = enemy->GetEnemyStateMachine();
				const auto& tf = enemy->GetTransform();
				bearsArr.push_back(nlohmann::json::array({
					static_cast<int>(i),
					V3toJson(tf.m_position),
					GetYawDeg(tf.m_rotation),
					sm ? sm->GetStateNameForLog() : "Unknown",
					RoundForLog(sm ? sm->GetSleepTimer() : 0.0f, 1.0)
				}));
			}
		}
		snap["bears"] = std::move(bearsArr);

		// ------ 子ペンギン ------
		// 配列レイアウト: [id, type, pos, rot_y, state, in_formation, is_alive]
		// in_formation/is_aliveはJSONのtrue/false（4〜5文字）ではなく0/1（1文字）で書き出す。
		// 子ペンギン1体につき2個×最大200体×数千tick分積み重なるため、この差だけでもログ全体で
		// 1割前後サイズが変わる。読み込み側（ReplayScene::ArrGetBool）は数値・真偽値のどちらでも読める
		auto* cpm = actor::ChildPenguinManager::GetInstance();
		nlohmann::json penguinsArr = nlohmann::json::array();
		if (cpm)
		{
			for (auto* child : cpm->GetChildPenguin())
			{
				if (!child) continue;
				auto* sm = child->GetStateMachine();
				const auto& tf = child->GetTransform();
				penguinsArr.push_back(nlohmann::json::array({
					child->GetLogId(),
					child->GetChildPenguinTypeStr(),
					V3toJson(tf.m_position),
					GetYawDeg(tf.m_rotation),
					sm ? sm->GetStateNameForLog() : "Unknown",
					cpm->IsFollower(child) ? 1 : 0,
					(sm ? !sm->GetPenguinStatus()->IsDead() : false) ? 1 : 0
				}));
			}
		}
		snap["penguins"] = std::move(penguinsArr);

		// ------ 渦潮 ------
		// 配列レイアウト: [id, pos, state, scale_xz]
		auto* wm = nature::WhirlpoolManager::GetInstance();
		nlohmann::json whirlpoolsArr = nlohmann::json::array();
		if (wm)
		{
			wm->ForEach([&](nature::Whirlpool* wp)
			{
				if (!wp) return;
				const auto& tf = wp->GetTransform();
				whirlpoolsArr.push_back(nlohmann::json::array({
					static_cast<int>(wp->GetIndex()),
					V3toJson(tf.m_position),
					WhirlpoolStateName(wp->GetState()),
					RoundForLog(tf.m_scale.x, 0.001)
				}));
			});
		}
		snap["whirlpools"] = std::move(whirlpoolsArr);

		// ------ カメラ ------
		// 配列レイアウト: [pos, target, fov]
		// scale_xz・fovはpos/rot_yと違って値の範囲自体が小さい（scale_xzは実測で0〜0.12程度、
		// fovもラジアン単位で1前後）ため、整数に丸めると渦潮が消えたりFOVが崩れたりしてしまう。
		// そのため意味のある変化が残る精度のまま丸める
		{
			const auto& camData = camera::CameraManager::Get().GetCurrentCameraData();
			snap["camera"] = nlohmann::json::array({
				V3toJson(camData.position),
				V3toJson(camData.target),
				RoundForLog(camData.fov, 0.01)
			});
		}

		return snap;
	}


	float GameLogManager::QuatToYawDeg(float qx, float qy, float qz, float qw)
	{
		float yaw = atan2f(2.0f * (qy * qw + qx * qz),
			1.0f - 2.0f * (qy * qy + qz * qz));
		return yaw * (180.0f / 3.14159265f);
	}


	std::string GameLogManager::MakeSessionId()
	{
		auto now = std::chrono::system_clock::now();
		auto t   = std::chrono::system_clock::to_time_t(now);
		std::tm tm{};
#ifdef _WIN32
		localtime_s(&tm, &t);
#else
		localtime_r(&t, &tm);
#endif
		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
		return oss.str();
	}
}
