/**
 * @file EffectManager.cpp
 * @brief 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 * @author 藤谷
 */
#include "stdafx.h"
#include "EffectManager.h"


namespace app
{
	EffectManager* EffectManager::m_instance = nullptr; //初期化


	EffectManager::EffectManager()
	{
		m_effectList.clear();

		// サウンドの登録
		for (int i = 0; i < ARRAYSIZE(effectInformation); ++i) {
			const auto& info = effectInformation[i];
			EffectEngine::GetInstance()->ResistEffect(i, info.assetPath);
		}
	}


	EffectManager::~EffectManager()
	{}


	void EffectManager::Update()
	{
		// 再生が終了したエフェクトをm_effectListから除外する
		// NOTE: EffectEmitterはIsPlay()がfalseになるとDeleteGO(this)で自己削除される。
		//       自己削除のタイミングでm_effectListは更新されないため、
		//       毎フレームここで再生状態を確認し、無効なエントリを除外する。
		for (auto it = m_effectList.begin(); it != m_effectList.end(); )
		{
			if (it->second == nullptr || !it->second->IsPlay())
			{
				it = m_effectList.erase(it);
			}
			else
			{
				++it;
			}
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
		auto* newEffect = NewGO<EffectEmitter>(0);
		newEffect->Init(static_cast<int>(kind));
		newEffect->SetPosition(position);
		newEffect->SetRotation(rotation);
		newEffect->SetScale(scale);
		newEffect->Play();

		m_effectList.emplace(m_effectHandleCount, newEffect);

		return m_effectHandleCount++;
	}


	void EffectManager::StopEffect(const EffectHandle handle)
	{
		auto* effect = FindEffect(handle);
		if (effect == nullptr) {
			return;
		}
		effect->Stop();
	}


	void EffectManager::UnregisterEffect(const EffectHandle handle)
	{
		auto it = m_effectList.find(handle);
		if (it != m_effectList.end())
		{
			m_effectList.erase(it);
		}
	}


	void EffectManager::SetEffectPosition(const EffectHandle handle, const Vector3& position)
	{
		auto* effect = FindEffect(handle);
		if (effect == nullptr) {
			return;
		}
		effect->SetPosition(position);
	}
}