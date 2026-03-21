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
		/** エフェクトリストから再生していないものがあれば削除する */
		std::vector<EffectHandle> eraseSEList;
		for (auto& it : m_effectList) {
			const auto key = it.first;
			auto* se = it.second;
			/** 再生が終わっているなら削除 */
			if (!se->IsPlay()) {
				delete se;
				eraseSEList.push_back(key);
			}
		}
		for (const auto& key : eraseSEList) {
			m_effectList.erase(key);
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
		EffectEmitter* m_effect = NewGO<EffectEmitter>(0);
		m_effect->Init(static_cast<int>(kind));
		m_effect->SetPosition(position);
		m_effect->SetRotation(rotation);
		m_effect->SetScale(scale);
		m_effect->Play();

		m_effectList.emplace(m_effectHandleCount++, m_effect);

		return m_effectHandleCount;
	}


	void EffectManager::StopEffect(const EffectHandle handle)
	{
		auto* effect = FindEffect(handle);
		if (effect == nullptr) {
			return;
		}
		effect->Stop();
	}
}