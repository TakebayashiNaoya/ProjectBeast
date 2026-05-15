/**
 * @file EffectManager.cpp
 * @brief 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 * @author 藤谷
 */
#include "stdafx.h"
#include "EffectManager.h"
#include "graphics/effect/BeastEffectEmitter.h"
#include "Geometry/Frustum.h"


namespace app
{
	EffectManager* EffectManager::m_instance = nullptr; //初期化


	EffectManager::EffectManager()
	{
		m_effectList.clear();

		// エフェクトの登録
		// ResistEffect() の初回呼び出し時に TextureLoader などの Loader 初期化が
		// 必要になるため、コンストラクタで一度だけ BeginFrame() を呼んで事前セットアップする。
		// 毎フレームの BeginFrame() は K2EngineLow 側から呼ばれるが、ここでは登録処理の前提条件を満たす目的で実行している。
		EffectEngine::GetInstance()->BeginFrame();

		for (int i = 0; i < ARRAYSIZE(effectInformation); ++i) {
			const auto& info = effectInformation[i];
			EffectEngine::GetInstance()->ResistEffect(i, info.assetPath);
		}
	}


	EffectManager::~EffectManager()
	{}


	void EffectManager::Update(const nsBeastEngine::Frustum& frustum)
	{
		for (auto it = m_effectList.begin(); it != m_effectList.end(); )
		{
			auto* emitter = it->second.emitter;

			// 再生が終了したエフェクトをm_effectListから除外する
			// NOTE: BeastEffectEmitterはIsExist()がfalseになるとDeleteGO(this)で自己削除される。
			//       自己削除のタイミングでm_effectListは更新されないため、
			//       毎フレームここで再生状態を確認し、無効なエントリを除外する。
			if (emitter == nullptr || !emitter->IsExist())
			{
				it = m_effectList.erase(it);
				continue;
			}

			// フラスタムカリング判定
			// エフェクト種別の基準半径にスケールの最大成分を乗算して実効半径を求める
			const uint8_t kindIndex = static_cast<uint8_t>(it->second.kind);
			const EffectInformation& info = effectInformation[kindIndex];
			const Vector3& scale = emitter->GetScale();
			const float maxScale = max(max(scale.x, scale.y), scale.z);
			const float boundingRadius = info.baseRadius * maxScale;

			const bool isVisible = frustum.IsIntersectSphere(emitter->GetPosition(), boundingRadius);
			emitter->SetVisible(isVisible);

			++it;
		}
	}


	EffectHandle EffectManager::PlayEffect(const EnEffectKind kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale)
	{
		// ハンドルが最大数になったら使えない
		// NOTE: そんなに再生するはずがない
		if (m_effectHandleCount == INVALID_EFFECT_HANDLE) {
			K2_ASSERT(false, "エフェクトの再生が多いです。\n");
			return INVALID_EFFECT_HANDLE;
		}
		auto* newEffect = NewGO<nsBeastEngine::BeastEffectEmitter>(0);
		newEffect->Init(static_cast<int>(kind));
		newEffect->SetPosition(position);
		newEffect->SetRotation(rotation);
		newEffect->SetScale(scale);
		newEffect->Play();

		EffectEntry entry;
		entry.emitter = newEffect;
		entry.kind = kind;
		m_effectList.emplace(m_effectHandleCount, entry);

		return m_effectHandleCount++;
	}


	void EffectManager::StopEffect(const EffectHandle handle)
	{
		auto* emitter = FindEffect(handle);
		if (emitter == nullptr) {
			return;
		}
		emitter->Stop();
	}


	void EffectManager::UnregisterEffect(const EffectHandle handle)
	{
		auto it = m_effectList.find(handle);
		if (it != m_effectList.end())
		{
			m_effectList.erase(it);
		}
	}
}