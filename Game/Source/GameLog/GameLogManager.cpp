/**
 * @file GameLogManager.cpp
 * @brief ゲームプレイログの記録・出力管理クラス
 * @author 竹林
 */
#include "stdafx.h"
#include "GameLogManager.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Nature/WhirlpoolManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Manager/TimeManager.h"
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
        /** Quaternion の Apply で進行方向を取得し Yaw 角度（度）を返す */
        float GetYawDeg(const Quaternion& rot)
        {
            Vector3 forward = Vector3::AxisZ;
            rot.Apply(forward);
            return atan2f(forward.x, forward.z) * (180.0f / 3.14159265f);
        }

        /** Vector3 を json 配列に変換 */
        nlohmann::json V3toJson(const Vector3& v)
        {
            return nlohmann::json::array({ v.x, v.y, v.z });
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
            session["session_id"]   = m_sessionId;
            session["stage"]        = stage;
            session["duration_sec"] = durationSec;
            session["result"]["rescued"] = rescued;
            session["result"]["score"]   = score;

            std::ofstream ofs(dir + "/session.json");
            ofs << session.dump(2) << "\n";
        }

        // --- ticks.jsonl ---
        {
            std::ofstream ofs(dir + "/ticks.jsonl");
            for (const auto& rec : m_ticks)
            {
                ofs << rec.dump() << "\n";
            }
        }

        m_ticks.clear();
        m_pendingEvents.clear();
        m_frameCount = 0;
    }


    float GameLogManager::GetGameTime() const
    {
        return TimeManager::GetInstance().GetCurTime();
    }


    nlohmann::json GameLogManager::BuildTickSnapshot(actor::DaddyPenguin* daddy) const
    {
        nlohmann::json snap;

        // ------ 親ペンギン ------
        if (daddy)
        {
            auto* sm = daddy->GetStateMachine();
            const auto& tf = daddy->GetTransform();
            snap["parent"] = {
                { "pos",   V3toJson(tf.m_position)          },
                { "rot_y", GetYawDeg(tf.m_rotation)         },
                { "state", sm ? sm->GetStateNameForLog() : "Unknown" }
            };
        }

        // ------ シロクマ ------
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
                bearsArr.push_back({
                    { "id",         static_cast<int>(i)             },
                    { "pos",        V3toJson(tf.m_position)         },
                    { "rot_y",      GetYawDeg(tf.m_rotation)        },
                    { "state",      sm ? sm->GetStateNameForLog() : "Unknown" },
                    { "sleep_timer", sm ? sm->GetSleepTimer()    : 0.0f      }
                });
            }
        }
        snap["bears"] = std::move(bearsArr);

        // ------ 子ペンギン ------
        auto* cpm = actor::ChildPenguinManager::GetInstance();
        nlohmann::json penguinsArr = nlohmann::json::array();
        if (cpm)
        {
            for (auto* child : cpm->GetChildPenguin())
            {
                if (!child) continue;
                auto* sm = child->GetStateMachine();
                const auto& tf = child->GetTransform();
                penguinsArr.push_back({
                    { "id",           child->GetLogId()                           },
                    { "type",         child->GetChildPenguinTypeStr()              },
                    { "pos",          V3toJson(tf.m_position)                     },
                    { "rot_y",        GetYawDeg(tf.m_rotation)                    },
                    { "state",        sm ? sm->GetStateNameForLog() : "Unknown"          },
                    { "in_formation", cpm->IsFollower(child)                            },
                    { "is_alive",     sm ? !sm->GetPenguinStatus()->IsDead() : false    }
                });
            }
        }
        snap["penguins"] = std::move(penguinsArr);

        // ------ 渦潮 ------
        auto* wm = nature::WhirlpoolManager::GetInstance();
        nlohmann::json whirlpoolsArr = nlohmann::json::array();
        if (wm)
        {
            wm->ForEach([&](nature::Whirlpool* wp)
            {
                if (!wp) return;
                const auto& tf = wp->GetTransform();
                whirlpoolsArr.push_back({
                    { "id",       static_cast<int>(wp->GetIndex())   },
                    { "pos",      V3toJson(tf.m_position)            },
                    { "state",    WhirlpoolStateName(wp->GetState())  },
                    { "scale_xz", tf.m_scale.x                       }
                });
            });
        }
        snap["whirlpools"] = std::move(whirlpoolsArr);

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
