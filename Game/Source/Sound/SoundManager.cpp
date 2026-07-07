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
	constexpr uint8_t CP_REACTION_SE_PLAY_NUM = 5;

	constexpr uint8_t CP_SE_PLAY_NUM = 5;

	constexpr uint8_t CP_WATER_OUT_NUM = 10;

	constexpr uint8_t CLUMSY_CP_CRY_NUM = 5;

	constexpr float DEFAULT_VOLUME = 0.5f;

	constexpr const char* SOUND_GO_NAME = "se";
}


namespace app
{
	SoundManager* SoundManager::m_instance = nullptr;


	SoundManager::SoundManager()
		: m_masterVolume(DEFAULT_VOLUME)
		, m_bgmVolume(DEFAULT_VOLUME)
		, m_seVolume(DEFAULT_VOLUME)
		, m_voiceVolume(DEFAULT_VOLUME)
	{
		m_seList.clear();

		for (int i = 0; i < ARRAYSIZE(soundInformation); i++) {
			const auto& info = soundInformation[i];
			g_soundEngine->ResistWaveFileBank(i, info.assetPath.c_str());
		}

		// ===== SE種別ごとの同時再生数上限を設定 =====
		// 子ペンギンの足音（数が多いため制限）
		m_seConcurrentLimitMap[enSoundKind_PenguinSneak] = CP_SE_PLAY_NUM;
		m_seConcurrentLimitMap[enSoundKind_PenguinDash] = CP_SE_PLAY_NUM;
		m_seConcurrentLimitMap[enSoundKind_PenguinSlide] = CP_SE_PLAY_NUM;
		//m_seConcurrentLimitMap[enSoundKind_PenguinWaterIn] = CP_SE_PLAY_NUM;
		m_seConcurrentLimitMap[enSoundKind_PenguinWaterOut] = CP_WATER_OUT_NUM;
		//m_seConcurrentLimitMap[enSoundKind_PenguinSwimming] = CP_SE_PLAY_NUM;
		// 子ペンギンのリアクションSE（同種が重複しないよう制限）
		m_seConcurrentLimitMap[enSoundKind_CPReactionHappy] = CP_REACTION_SE_PLAY_NUM;
		m_seConcurrentLimitMap[enSoundKind_CPReactionTrouble] = CP_REACTION_SE_PLAY_NUM;
		m_seConcurrentLimitMap[enSoundKind_ChildPenguinCRY] = CLUMSY_CP_CRY_NUM;

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
			if (!se->IsPlaying()) {
				eraseSEList.push_back(key);
				DeleteGO(se);
			}
		}
		for (const auto& key : eraseSEList) {
			m_seList.erase(key);
			m_seHandleKindMap.erase(key);
		}


		/** Voiceリストから再生していないものがあれば削除する */
		std::vector<VoiceHandle> eraseVoiceList;
		for (auto& it : m_voiceList) {
			const auto key = it.first;
			auto* voice = it.second;
			/** 再生が終わっているなら削除 */
			if (!voice->IsPlaying()) {
				eraseVoiceList.push_back(key);
				DeleteGO(voice);    // 再生を止める
			}
		}
		for (const auto& key : eraseVoiceList) {
			m_voiceList.erase(key);
		}


		// リクエストされたSEを再生する
		for (const auto& infoList : m_seInfomationList) {
			for (const auto& info : infoList) {
				auto* se = NewGO<SoundSource>(0, SOUND_GO_NAME);
				se->Init(info.m_kind, info.m_is3D);
				se->SetVolume(m_masterVolume * m_seVolume * info.m_volumeMagnification);
				se->Play(info.m_isLoop);
				m_seList.emplace(info.m_handle, se);
				m_seHandleKindMap.emplace(info.m_handle, info.m_kind);
			}
		}
		for (auto& infoList : m_seInfomationList) {
			infoList.clear();
		}

		/** BGMフェードアウトの進行 */
		if (m_isBgmFading && m_bgm != nullptr)
		{
			m_bgmFadeTimer += g_gameTime->GetFrameDeltaTime();
			const float t = std::clamp(m_bgmFadeTimer / m_bgmFadeDuration, 0.0f, 1.0f);
			const float volumeMagn = m_bgmInformation ? m_bgmInformation->m_volumeMagnification : DEFAULT_VOLUME_MAGNIFICATION;
			m_bgm->SetVolume(m_masterVolume * m_bgmVolume * volumeMagn * (1.0f - t));

			if (t >= 1.0f)
			{
				m_isBgmFading = false;
				StopBGM();
			}
		}
	}


	void SoundManager::PlayBGM(const int kind, const float volumeMagnification)
	{
		const float bgmVolumeMagn = (volumeMagnification == INVALID_VOLUME) ? DEFAULT_VOLUME_MAGNIFICATION : volumeMagnification;

		// BGMはゲーム上に1つしか存在しないため、常に情報を上書きする
		// （BGMInformationのコンストラクタでisLoopは強制的にtrueになる）
		m_bgmInformation.emplace(kind, bgmVolumeMagn);

		if (m_bgm == nullptr) {
			m_bgm = NewGO<SoundSource>(0, SOUND_GO_NAME);
		}
		else {
			m_bgm->Stop();
		}
		/** 初期化 */
		m_bgm->Init(kind);
		m_bgm->Play(m_bgmInformation->m_isLoop); // 必ずtrue（ループ再生）

		/** 新しいBGMを再生し直すので、進行中のフェードアウトは打ち切る */
		m_isBgmFading = false;

		ApplyBGMVolume();
	}


	void SoundManager::StopBGM()
	{
		/** 進行中のフェードアウトがあれば打ち切る（停止済みの音源に対してフェードが動き続けるのを防ぐ） */
		m_isBgmFading = false;

		if (m_bgm == nullptr) {
			return;
		}
		m_bgm->Stop();
	}


	void SoundManager::FadeOutBGM(const float duration)
	{
		if (m_bgm == nullptr) return;

		/** 0秒以下が指定された場合は即座に停止する */
		if (duration <= 0.0f)
		{
			StopBGM();
			return;
		}

		m_isBgmFading = true;
		m_bgmFadeTimer = 0.0f;
		m_bgmFadeDuration = duration;
	}


	SEHandle SoundManager::PlaySE(const int kind, const float volumeMagnification, const bool isLoop, const bool is3D, const EnSoundPriority priority)
	{
		if (m_soundHandleCount == INVALID_SE_HANDLE) {
			K2_ASSERT(false, "サウンドの再生が多いです。\n");
			return INVALID_SE_HANDLE;
		}

		// ===== 同時再生数チェック =====
		auto limitIt = m_seConcurrentLimitMap.find(kind);
		if (limitIt != m_seConcurrentLimitMap.end())
		{
			uint32_t count = 0;

			// 再生中のSEをカウント
			for (const auto& kv : m_seHandleKindMap)
			{
				if (kv.second == kind) { ++count; }
			}
			// リクエスト済み（まだ再生されていない）SEもカウント
			for (const auto& infoList : m_seInfomationList)
			{
				for (const auto& info : infoList)
				{
					if (info.m_kind == kind) { ++count; }
				}
			}

			if (count >= limitIt->second)
			{
				// 上限に達しているので再生しない
				return INVALID_SE_HANDLE;
			}
		}
		// ================================

		SEHandle handle = m_soundHandleCount++;

		const float seVolume = (volumeMagnification == INVALID_VOLUME) ? DEFAULT_VOLUME_MAGNIFICATION : volumeMagnification;
		m_seInfomationList[priority].push_back(SEInformation(kind, isLoop, is3D, handle, seVolume));
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
				it.second->Stop();
				DeleteGO(it.second);
			}
		}
		m_seList.clear();
		m_seHandleKindMap.clear(); // ← 追加

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
		auto* voice = NewGO<SoundSource>(0, SOUND_GO_NAME);
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
			const float volumeMagn = m_bgmInformation ? m_bgmInformation->m_volumeMagnification : DEFAULT_VOLUME_MAGNIFICATION;
			m_bgm->SetVolume(m_masterVolume * m_bgmVolume * volumeMagn);
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