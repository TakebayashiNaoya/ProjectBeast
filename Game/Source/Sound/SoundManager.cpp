/**
 * @file SoundManager.cpp
 * @brief サウンドの管理をするクラス
 * @author 立山
 */
#include "stdafx.h"
#include "SoundManager.h"
#include <algorithm>


namespace
{
	// SE再生上限数
	constexpr uint8_t MAX_SE_PLAY_NUM = 7;
}


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
	{}


	void SoundManager::Update()
	{
		/** SEリストから再生していないものがあれば削除する */
		std::vector<SEHandle> eraseSEList;
		for (auto& it : m_seList) {
			const auto key = it.first;
			auto* se = it.second;
			/** 再生が終わっているなら削除 */
			if (!se->IsPlaying()) {
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
				eraseVoiceList.push_back(key);
			}
		}
		for (const auto& key : eraseVoiceList) {
			m_voiceList.erase(key);
		}


		// リクエストされたSEを再生する
		int playNum = 0;
		// 優先度別のList
		for (const auto& infoList : m_seInfomationList) {
			// 優先度の中のリスト
			for (const auto& info : infoList) {
				auto* se = NewGO<SoundSource>(0, "se");
				se->Init(info.m_kind, info.m_is3D);
				se->SetVolume(m_masterVolume * m_seVolume);
				se->Play(info.m_isLoop);
				m_seList.emplace(info.m_handle, se);
				// 再生数加算
				++playNum;
				// 再生数を超えたかチェック
				if (playNum >= MAX_SE_PLAY_NUM) {
					break;
				}
			}
			// 既に再生数を超えているなら処理する必要がない
			if (playNum >= MAX_SE_PLAY_NUM) {
				break;
			}
		}
		// 再生完了なのでクリア
		for (auto& infoList : m_seInfomationList) {
			infoList.clear();
		}
	}


	void SoundManager::PlayBGM(const int kind)
	{
		if (m_bgm == nullptr) {
			m_bgm = NewGO<SoundSource>(0, "se");
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


	SEHandle SoundManager::PlaySE(const int kind, const bool isLoop, const bool is3D, const EnSoundPriority priority)
	{
		/** ハンドルが最大数になったら使えない */
		if (m_soundHandleCount == INVALID_SE_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_SE_HANDLE;
		}
		// ハンドルは常に加算していく
		// そのため再生されない可能性があるので、Handle取得時はnullptrチェック必須
		SEHandle handle = m_soundHandleCount++;

		// 優先度別の再生リクエスト情報を追加
		m_seInfomationList[priority].push_back(SEInformation(kind, isLoop, is3D, handle));

		return handle;
	}


	void SoundManager::StopSE(const SEHandle handle)
	{
		auto* se = FindSE(handle);
		if (se == nullptr) {
			return;
		}
		se->Stop();
	}


	void SoundManager::StopAllSE()
	{
		for (auto& it : m_seList) {
			if (it.second != nullptr) {
				it.second->Stop();    // 再生を止める
				DeleteGO(it.second);  // ゲームオブジェクトとして破棄
			}
		}
		m_seList.clear(); // リストを空にする

		//// Voice（鳴き声など）も同様に処理
		//for (auto& it : m_voiceList) {
		//	if (it.second != nullptr) {
		//		it.second->Stop();
		//		DeleteGO(it.second);
		//	}
		//}
		//m_voiceList.clear();

		//m_seList.clear();

		for (auto& infolist : m_seInfomationList) {
			infolist.clear();
		}
	}


	VoiceHandle SoundManager::PlayVoice(const int kind, const bool isLoop, const bool is3D)
	{
		/** ハンドルが最大数になったら使えない */
		if (m_soundHandleCount == INVALID_VOICE_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_VOICE_HANDLE;
		}
		auto* voice = NewGO<SoundSource>(0, "se");
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