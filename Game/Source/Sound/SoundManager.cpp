/**
 * @file SoundManager.cpp
 * @brief サウンドの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "SoundManager.h"


namespace app
{
	SoundManager* SoundManager::m_instance = nullptr;


	SoundManager::SoundManager()
	{
		m_seList.clear();
	}


	SoundManager::~SoundManager()
	{
	}


	void SoundManager::Update()
	{
		/** SEリストから再生していないものがあれば削除する */
		std::vector<SoundHandle> eraseList;
		for (auto& it : m_seList) {
			const auto key = it.first;
			auto* se = it.second;
			/** 再生が終わっているなら削除 */
			if (!se->IsPlaying()) {
				eraseList.push_back(key);
			}
		}
		for (const auto& key : eraseList) {
			m_seList.erase(key);
		}
	}


	void SoundManager::PlayBGM(const int kind)
	{
		if (m_bgm == nullptr) {
			m_bgm = new SoundSource;
		}
		else {
			m_bgm->Stop();
		}
		/** 初期化 */
		m_bgm->Init(kind);
		m_bgm->Play(true);
	}


	void SoundManager::StopBGM()
	{
		if (m_bgm == nullptr) {
			return;
		}
		m_bgm->Stop();
	}


	SoundHandle SoundManager::PlaySE(const int kind, const bool isLoop, const bool is3D)
	{
		/** ハンドルが最大数になったら使えない */
		if (m_soundHandleCount == INVALID_SOUND_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_SOUND_HANDLE;
		}
		auto* se = new SoundSource;
		se->Init(kind, is3D);
		se->Play(isLoop);

		m_seList.emplace(m_soundHandleCount++, se);

		return m_soundHandleCount;
	}


	void SoundManager::StopSE(const SoundHandle handle)
	{
		auto* se = FindSE(handle);
		if (se == nullptr) {
			return;
		}
		se->Stop();
	}
}