/**
 * @file FormationVisual.cpp
 * @brief 陣形に付属させる演出（ビジュアルエフェクト）の実装
 */
#include "stdafx.h"
#include "FormationVisual.h"
#include "FormationContext.h"
#include "MasterFormationParameter.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Util/JsonConverter.h"
#include "graphics/effect/BeastEffectEmitter.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			/** エフェクトのY座標オフセット */
			constexpr float OFFSET_Y = 10.0f;

			/** 演出が合わせる円に追従する速さ（1秒あたりの補間率） */
			constexpr float SCALE_FOLLOW_SPEED = 4.0f;
			/** efkの半径がこれ以下なら大きさを変えない（0割りを防ぐ） */
			constexpr float MIN_EFFECT_RADIUS = 0.001f;

			/** 演出の見た目の調整値 */
			constexpr const char* TUNING_JSON_PATH = "Assets/parameter/character/penguin/formation/FormationVisualTuning.json";
			/** 調整値のファイルの変更を見る間隔（秒） */
			constexpr float TUNING_RELOAD_INTERVAL = 0.5f;
			/** 陣形ごとのJSONのキー（EnFormationTypeの並び順） */
			constexpr const char* EFFECT_RADIUS_KEYS[] = { "speedEffectRadius", "defenseEffectRadius", "gatherEffectRadius" };
			static_assert(std::size(EFFECT_RADIUS_KEYS) == static_cast<size_t>(EnFormationType::Num), "調整値のキーの数が陣形の数と合わない");

			struct VisualTuning
			{
				/** efkの大きさ1のときの半径（30の陣形で大きさ8になる値） */
				float effectRadius[static_cast<int>(EnFormationType::Num)] = { 3.75f, 3.75f, 3.75f };
				/** 合わせる円を地面に描くか（デバッグビルドだけ） */
				bool showFitCircle = false;
				bool isLoaded = false;
#if defined(APP_DEBUG)
				time_t lastWriteTime = 0;
				float reloadTimer = 0.0f;
#endif
			};

			VisualTuning& GetTuningStorage()
			{
				static VisualTuning tuning;
				return tuning;
			}

			void LoadTuning(VisualTuning& tuning)
			{
				nlohmann::json j;
				if (util::JsonConverter::IsLoadJsonFile(j, TUNING_JSON_PATH))
				{
					for (int i = 0; i < static_cast<int>(EnFormationType::Num); ++i)
					{
						tuning.effectRadius[i] = util::JsonConverter::ToFloat(j, EFFECT_RADIUS_KEYS[i], tuning.effectRadius[i]);
					}
					tuning.showFitCircle = util::JsonConverter::ToBool(j, "showFitCircle", tuning.showFitCircle);
				}
				tuning.isLoaded = true;
#if defined(APP_DEBUG)
				tuning.lastWriteTime = util::JsonConverter::GetFileLastWriteTime(TUNING_JSON_PATH);
#endif
			}

			/** @brief 調整値を返す。初回に読み、デバッグビルドではファイルが変わったら読み直す */
			const VisualTuning& GetTuning(float dt)
			{
				VisualTuning& tuning = GetTuningStorage();
				if (!tuning.isLoaded) LoadTuning(tuning);
#if defined(APP_DEBUG)
				tuning.reloadTimer += dt;
				if (tuning.reloadTimer >= TUNING_RELOAD_INTERVAL)
				{
					tuning.reloadTimer = 0.0f;
					if (util::JsonConverter::CheckFileModified(TUNING_JSON_PATH, tuning.lastWriteTime)) LoadTuning(tuning);
				}
#endif
				return tuning;
			}
		}


		FormationAuraVisual::FormationAuraVisual(EnFormationType type, EnEffectKind loopKind, EnEffectKind endKind, EnAuraFit fit, const MasterFormationParameter& param)
			: m_type(type)
			, m_loopKind(loopKind)
			, m_endKind(endKind)
			, m_fit(fit)
			, m_param(param)
		{
		}


		float FormationAuraVisual::CalcFitRadius(const FormationContext& ctx) const
		{
			if (m_fit == EnAuraFit::FetchRange)
			{
				const int level = (ctx.penguinManager != nullptr) ? ctx.penguinManager->GetFormationLevel() : 0;
				return m_param.passiveFetchRadiusBase + m_param.passiveFetchRadiusPerLevel * static_cast<float>(level);
			}
			return ctx.formationRadius;
		}


		float FormationAuraVisual::CalcScale(const FormationContext& ctx) const
		{
			const float effectRadius = GetTuning(0.0f).effectRadius[static_cast<int>(m_type)];
			if (effectRadius <= MIN_EFFECT_RADIUS) return m_scale;
			return (std::max)(CalcFitRadius(ctx), 0.0f) / effectRadius;
		}


		EffectHandle FormationAuraVisual::PlayAttached(EnEffectKind kind, float scale, const FormationContext& ctx)
		{
			auto& em = EffectManager::Get();
			const EffectHandle handle = em.PlayEffect(
				kind,
				ctx.daddyPenguin->GetTransform().m_position,
				Quaternion::Identity,
				Vector3(Vector3::One * scale)
			);
			em.AttachEffect(
				handle,
				&ctx.daddyPenguin->GetTransform().m_position,
				Vector3(0.0f, OFFSET_Y, 0.0f)
			);
			return handle;
		}


		void FormationAuraVisual::Enter(const FormationContext& ctx)
		{
			m_scale = CalcScale(ctx);
			m_handle = PlayAttached(m_loopKind, m_scale, ctx);
		}


		void FormationAuraVisual::Update(float dt, const FormationContext& ctx)
		{
			const VisualTuning& tuning = GetTuning(dt);
#if defined(APP_DEBUG)
			if (ctx.penguinManager != nullptr)
			{
				ctx.penguinManager->SetFormationFitCircle(tuning.showFitCircle ? CalcFitRadius(ctx) : 0.0f, m_type);
			}
#else
			(void)tuning;
#endif

			// 合わせる円が広がったら、演出も追いかけて広げる（一気に変わらないよう補間する）
			const float t = std::clamp(dt * SCALE_FOLLOW_SPEED, 0.0f, 1.0f);
			m_scale += (CalcScale(ctx) - m_scale) * t;

			auto* effect = EffectManager::Get().FindEffect(m_handle);
			if (effect == nullptr)
			{
				// 再生が終わったら出し直してループさせる
				m_handle = PlayAttached(m_loopKind, m_scale, ctx);
				return;
			}
			effect->SetScale(Vector3(Vector3::One * m_scale));
		}


		void FormationAuraVisual::Exit(const FormationContext& ctx)
		{
			EffectManager::Get().StopEffect(m_handle);
#if defined(APP_DEBUG)
			if (ctx.penguinManager != nullptr) ctx.penguinManager->SetFormationFitCircle(0.0f, m_type);
#endif
			if (m_endKind == EnEffectKind::None) return;
			// 抜けるときのctxは新しい陣形の半径になっているため、直前までの大きさで出す
			m_handle = PlayAttached(m_endKind, m_scale, ctx);
		}
	}
}
