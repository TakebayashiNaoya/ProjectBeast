/**
 * @file SoundManager.cpp
 * @brief サウンドの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "SoundManager.h"
#include <algorithm>


namespace app
{
	SoundManager* SoundManager::m_instance = nullptr;


	SoundManager::SoundManager()
		: m_masterVolume(1.0f)
		, m_bgmVolume(1.0f)
		, m_seVolume(1.0f)
		, m_voiceVolume(1.0f)
	{
		m_seList.clear();

		for (int i = 0; i < ARRAYSIZE(soundInformation); i++) {
			const auto& info = soundInformation[i];
			g_soundEngine->ResistWaveFileBank(i, info.assetPath.c_str());
		}
	}


	SoundManager::~SoundManager()
	{
	}


	void SoundManager::Update()
	{
		/** SEリストから再生していないものがあれば削除する */
		std::vector<SEHandle> eraseSEList;
		for (auto& it : m_seList) {
			const auto key = it.first;
			auto* se = it.second;
			/** 再生が終わっているなら削除 */
			if (!se->IsPlaying()) {
				delete se;
				eraseSEList.push_back(key);
			}
		}
		for (const auto& key : eraseSEList) {
			m_seList.erase(key);
		}


		/** Voiceリストから再生していないものがあれば削除する */
		std::vector<VoiceHandle> eraseVoiceList;
		for (auto& it : m_voiceList) {
			const auto key = it.first;
			auto* voice = it.second;
			/** 再生が終わっているなら削除 */
			if (!voice->IsPlaying()) {
				delete voice;
				eraseVoiceList.push_back(key);
			}
		}
		for (const auto& key : eraseVoiceList) {
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

		ApplyBGMVolume();
	}


	void SoundManager::StopBGM()
	{
		if (m_bgm == nullptr) {
			return;
		}
		m_bgm->Stop();
	}


	SEHandle SoundManager::PlaySE(const int kind, const bool isLoop, const bool is3D)
	{
		/** ハンドルが最大数になったら使えない */
		if (m_soundHandleCount == INVALID_SE_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_SE_HANDLE;
		}
		auto* se = new SoundSource;
		se->Init(kind, is3D);
		se->SetVolume(m_masterVolume * m_seVolume);
		se->Play(isLoop);

		m_seList.emplace(m_soundHandleCount++, se);

		return m_soundHandleCount;
	}


	void SoundManager::StopSE(const SEHandle handle)
	{
		auto* se = FindSE(handle);
		if (se == nullptr) {
			return;
		}
		se->Stop();
	}


	VoiceHandle SoundManager::PlayVoice(const int kind, const bool isLoop, const bool is3D)
	{
		/** ハンドルが最大数になったら使えない */
		if (m_soundHandleCount == INVALID_VOICE_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_VOICE_HANDLE;
		}
		auto* voice = new SoundSource;
		voice->Init(kind, is3D);
		voice->SetVolume(m_masterVolume * m_voiceVolume);
		voice->Play(isLoop);

		m_voiceList.emplace(m_soundHandleCount++, voice);

		return m_soundHandleCount;
	}


	void SoundManager::SetMasterVolume(float volume)
	{
		m_masterVolume = std::clamp(volume, 0.0f, 1.0f);


		ApplyBGMVolume();
		ApplySEVolume();
		ApplyVoiceVolume();
	}


	void SoundManager::SetBGMVolume(float volume)
	{
		m_bgmVolume = std::clamp(volume, 0.0f, 1.0f);

		ApplyBGMVolume();
	}


	void SoundManager::SetSEVolume(float volume)
	{
		m_seVolume = std::clamp(volume, 0.0f, 1.0f);

		ApplySEVolume();
	}


	void SoundManager::SetVoiceVolume(float volume)
	{
		m_voiceVolume = std::clamp(volume, 0.0f, 1.0f);

		ApplyVoiceVolume();
	}


	void SoundManager::ApplyBGMVolume()
	{
		if (m_bgm)
		{
			m_bgm->SetVolume(m_masterVolume * m_bgmVolume);
		}
	}


	void SoundManager::ApplySEVolume()
	{
		for (auto& se : m_seList)
		{
			se.second->SetVolume(m_masterVolume * m_seVolume);
		}
	}


	void SoundManager::ApplyVoiceVolume()
	{
		for (auto& voice : m_voiceList)
		{
			voice.second->SetVolume(m_masterVolume * m_voiceVolume);
		}
	}
}