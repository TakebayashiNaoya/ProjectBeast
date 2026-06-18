/**
 * @file ParticleEffectLoader.h
 * @brief JSONからパーティクルエフェクトを構築するローダー
 * @author 忽那
 */
#pragma once
#include "ParticleEmitter.h"
#include <fstream>
#include <string>
#include "Json/json.hpp"


using json = nlohmann::json;


namespace ParticleJsonHelper
{
    /** EasingType文字列変換 */
    inline app::util::EasingType ParseEasingType(const std::string& str)
    {
        if (str == "EaseIn")    return app::util::EasingType::EaseIn;
        if (str == "EaseOut")   return app::util::EasingType::EaseOut;
        if (str == "EaseInOut") return app::util::EasingType::EaseInOut;
        return app::util::EasingType::Linear;
    }

    /** FloatValueProviderをJSONから設定 */
    inline void LoadFloatProvider(app::FloatValueProvider& provider, const json& j)
    {
        if (!j.is_object()) {
            // 数値直指定 → Fixed
            provider.SetFixed(j.get<float>());
            return;
        }

        std::string mode = j.value("mode", "fixed");

        if (mode == "fixed") {
            provider.SetFixed(j.value("value", 0.0f));
        }
        else if (mode == "random") {
            provider.SetRandom(
                j.value("min", 0.0f),
                j.value("max", 1.0f)
            );
        }
        else if (mode == "curve") {
            app::util::EasingType easing = ParseEasingType(j.value("easing", "Linear"));
            provider.SetCurve(
                j.value("start", 0.0f),
                j.value("end", 1.0f),
                easing
            );
        }
        else if (mode == "randomCurve") {
            app::util::EasingType easing = ParseEasingType(j.value("easing", "Linear"));
            provider.SetRandomCurve(
                j.value("minStart", 0.0f),
                j.value("maxStart", 1.0f),
                j.value("minEnd", 0.0f),
                j.value("maxEnd", 1.0f),
                easing
            );
        }
    }

    /** Vector3ValueProviderをJSONから設定 */
    inline void LoadVector3Provider(app::Vector3ValueProvider& provider, const json& j)
    {
        if (!j.is_object()) return;

        // uniform対応: Vector3の中で指定可能
        if (j.count("uniform")) {
            provider.SetUniform(j["uniform"].get<bool>());
        }

        std::string mode = j.value("mode", "fixed");

        if (mode == "fixed") {
            Vector3 val;
            val.x = j.value("x", 0.0f);
            val.y = j.value("y", 0.0f);
            val.z = j.value("z", 0.0f);
            provider.SetFixed(val);
        }

        else if (mode == "random") {
            Vector3 minVal, maxVal;
            minVal.x = j["min"].value("x", 0.0f);
            minVal.y = j["min"].value("y", 0.0f);
            minVal.z = j["min"].value("z", 0.0f);
            maxVal.x = j["max"].value("x", 0.0f);
            maxVal.y = j["max"].value("y", 0.0f);
            maxVal.z = j["max"].value("z", 0.0f);
            provider.SetRandom(minVal, maxVal);
        }

        else if (mode == "perAxis") {
            // 軸ごとに個別設定
            if (j.count("x")) ParticleJsonHelper::LoadFloatProvider(provider.X(), j["x"]);
            if (j.count("y")) ParticleJsonHelper::LoadFloatProvider(provider.Y(), j["y"]);
            if (j.count("z")) ParticleJsonHelper::LoadFloatProvider(provider.Z(), j["z"]);
        }
    }
}


namespace app
{
    /**
     * パーティクルエフェクトローダー
     */
    class ParticleEffectLoader
    {
    public:
        /**
         * JSONファイルからエミッターを構築
         * @param filePath JSONファイルパス
         * @param emitter  構築先のエミッター
         * @return 成功したらtrue
         */
        static bool LoadFromFile(const char* filePath, ParticleEmitter& emitter)
        {
            std::ifstream file(filePath);
            if (!file.is_open()) {
                return false;
            }

            json root;
            try {
                file >> root;
            }
            catch (...) {
                return false;
            }

            return LoadFromJson(root, emitter);
        }

        /**
         * JSON文字列からエミッターを構築
         */
        static bool LoadFromString(const std::string& jsonStr, ParticleEmitter& emitter)
        {
            json root;
            try {
                root = json::parse(jsonStr);
            }
            catch (...) {
                return false;
            }

            return LoadFromJson(root, emitter);
        }

        /**
         * JSONオブジェクトからエミッターを構築
         */
        static bool LoadFromJson(const json& root, ParticleEmitter& emitter)
        {
            using namespace ParticleJsonHelper;

            // エミッター基本設定
            if (root.count("duration")) {
                emitter.SetDuration(root["duration"].get<float>());
            }

            // スポーンモジュール
            if (root.count("spawn")) {
                const json& sp = root["spawn"];
                SpawnModule& spawn = emitter.GetSpawnModule();
                if (sp.count("rate"))         spawn.SetSpawnRate(sp["rate"].get<float>());
                if (sp.count("burstCount"))   spawn.SetBurst(
                    sp["burstCount"].get<int>(),
                    sp.value("burstInterval", 0.0f)
                );
                if (sp.count("maxParticles")) spawn.SetMaxParticles(sp["maxParticles"].get<int>());
                if (sp.count("looping"))      spawn.SetLooping(sp["looping"].get<bool>());
                emitter.ResizePool();
            }

            // モジュール群
            if (root.count("modules") && root["modules"].is_array()) {
                const json& modules = root["modules"];

                for (size_t i = 0; i < modules.size(); ++i) {
                    const json& mod = modules[i];
                    std::string type = mod.value("type", "");

                    if (type == "InitLifetime") {
                        auto* m = emitter.AddModule<InitLifeTimeModule>();
                        if (mod.count("lifetime")) LoadFloatProvider(m->LifeTime(), mod["lifetime"]);
                    }
                    else if (type == "InitPosition") {
                        auto* m = emitter.AddModule<InitPositionModule>();
                        if (mod.count("position")) LoadVector3Provider(m->Position(), mod["position"]);
                    }
                    else if (type == "InitVelocity") {
                        auto* m = emitter.AddModule<InitVelocityModule>();
                        if (mod.count("velocity")) LoadVector3Provider(m->Velocity(), mod["velocity"]);
                    }
                    else if (type == "InitScale") {
                        auto* m = emitter.AddModule<InitScaleModule>();
                        if (mod.count("scale")) LoadVector3Provider(m->Scale(), mod["scale"]);
                    }
                    else if (type == "InitRotation") {
                        auto* m = emitter.AddModule<InitRotationModule>();
                        if (mod.count("angle"))           LoadFloatProvider(m->Angle(), mod["angle"]);
                        if (mod.count("angularVelocity")) LoadFloatProvider(m->AngularVelocity(), mod["angularVelocity"]);
                    }
                    else if (type == "InitColor") {
                        auto* m = emitter.AddModule<InitColorModule>();
                        if (mod.count("r")) LoadFloatProvider(m->R(), mod["r"]);
                        if (mod.count("g")) LoadFloatProvider(m->G(), mod["g"]);
                        if (mod.count("b")) LoadFloatProvider(m->B(), mod["b"]);
                        if (mod.count("a")) LoadFloatProvider(m->A(), mod["a"]);
                    }
                    else if (type == "ScaleOverLife") {
                        auto* m = emitter.AddModule<ScaleOverLifeModule>();
                        if (mod.count("easing")) m->SetEasing(ParseEasingType(mod["easing"].get<std::string>()));

                        bool uniform = mod.value("uniform", false);
                        if (uniform) {
                            // uniform: true → startScale/endScale でXY同値
                            m->SetUniform(true);
                            if (mod.count("startScale")) LoadFloatProvider(m->StartScaleX(), mod["startScale"]);
                            if (mod.count("endScale"))   LoadFloatProvider(m->EndScaleX(), mod["endScale"]);
                        }
                        else {
                            // 従来のXY個別指定
                            if (mod.count("startScaleX")) LoadFloatProvider(m->StartScaleX(), mod["startScaleX"]);
                            if (mod.count("endScaleX"))   LoadFloatProvider(m->EndScaleX(), mod["endScaleX"]);
                            if (mod.count("startScaleY")) LoadFloatProvider(m->StartScaleY(), mod["startScaleY"]);
                            if (mod.count("endScaleY"))   LoadFloatProvider(m->EndScaleY(), mod["endScaleY"]);
                        }
                    }
                    else if (type == "RotationOverLife") {
                        auto* m = emitter.AddModule<RotationOverLifeModule>();
                        if (mod.count("startAngle")) LoadFloatProvider(m->StartAngle(), mod["startAngle"]);
                        if (mod.count("endAngle"))   LoadFloatProvider(m->EndAngle(), mod["endAngle"]);
                        if (mod.count("easing"))     m->SetEasing(ParseEasingType(mod["easing"].get<std::string>()));
                    }
                    else if (type == "AlphaOverLife") {
                        auto* m = emitter.AddModule<AlphaOverLifeModule>();
                        if (mod.count("startAlpha")) LoadFloatProvider(m->StartAlpha(), mod["startAlpha"]);
                        if (mod.count("endAlpha"))   LoadFloatProvider(m->EndAlpha(), mod["endAlpha"]);
                        if (mod.count("easing"))     m->SetEasing(ParseEasingType(mod["easing"].get<std::string>()));
                    }
                    else if (type == "SpeedOverLife") {
                        auto* m = emitter.AddModule<SpeedOverLifeModule>();
                        if (mod.count("startMultiplier")) LoadFloatProvider(m->StartMultiplier(), mod["startMultiplier"]);
                        if (mod.count("endMultiplier"))   LoadFloatProvider(m->EndMultiplier(), mod["endMultiplier"]);
                        if (mod.count("easing"))          m->SetEasing(ParseEasingType(mod["easing"].get<std::string>()));
                    }
                    else if (type == "ConvergenceOverLife") {
                        auto* m = emitter.AddModule<ConvergenceOverLifeModule>();
                        if (mod.count("startRatio")) LoadFloatProvider(m->StartRatio(), mod["startRatio"]);
						if (mod.count("endRatio"))   LoadFloatProvider(m->EndRatio(), mod["endRatio"]);
						if (mod.count("easing"))        m->SetEasing(ParseEasingType(mod["easing"].get<std::string>()));
                    }
                    else if (type == "Acceleration")
                    {
                        auto* m = emitter.AddModule<AccelerationModule>();
                        Vector3 accel;
                        accel.x = mod.value("x", 0.0f);
                        accel.y = mod.value("y", 0.0f);
                        accel.z = mod.value("z", 0.0f);
                        m->SetAcceleration(accel);
                    }
                    else if (type == "VelocityDamping") 
                    {
                        auto* m = emitter.AddModule<VelocityDampingModule>();
                        m->SetDamping(mod.value("damping", 0.0f));
                    }
                }
            }
            return true;
        }
    };
}
