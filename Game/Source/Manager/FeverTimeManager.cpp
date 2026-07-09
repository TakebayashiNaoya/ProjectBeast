/**
 * @file FeverTimeManager.cpp
 * @brief フィーバータイムを管理するクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "FeverTimeManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/JsonConverter.h"


namespace app
{
	FeverTimeManager* FeverTimeManager::m_instance = nullptr;


	void FeverTimeManager::Start(const char* parameterJsonPath)
	{
		nlohmann::json json;
		if (!util::JsonConverter::IsLoadJsonFile(json, parameterJsonPath)) return;

		m_dropInterval   = util::JsonConverter::ToFloat(json, "dropInterval", m_dropInterval);
		m_dropHeight     = util::JsonConverter::ToFloat(json, "dropHeight", m_dropHeight);
		m_feverDropCount = util::JsonConverter::ToInt(json, "feverDropCount", m_feverDropCount);
		m_feverEnabled   = util::JsonConverter::ToBool(json, "feverEnabled", m_feverEnabled);
	}


	void FeverTimeManager::Update()
	{
		if (m_pendingDropCount <= 0) return;

		m_dropTimer += g_gameTime->GetFrameDeltaTime();
		if (m_dropTimer < m_dropInterval) return;

		m_dropTimer -= m_dropInterval;
		m_pendingDropCount--;
		actor::ChildPenguinManager::GetInstance()->SpawnFromSky(m_dropHeight);
	}


	void FeverTimeManager::TryStartFeverOnAllCaught()
	{
		/** フィーバー無効なステージ（チュートリアル等）、または既に発生済みなら何もしない */
		if (!m_feverEnabled || m_hasTriggered) return;

		StartFever();
	}


	void FeverTimeManager::StartFever()
	{
		m_isActive = true;
		m_hasTriggered = true;
		m_dropTimer = 0.0f;

		/** フィーバー中BGMに切り替える */
		SoundManager::Get().PlayBGM(enSoundKind_FeverTime);

		/** それまでの捕獲数によらず、固定数を投下キューの初期数にする */
		m_pendingDropCount = m_feverDropCount;
		m_totalQueuedCount = m_feverDropCount;
	}


	void FeverTimeManager::OnPenguinCaught()
	{
		/** フィーバー中でなければ何もしない（通常時の捕獲では補充しない） */
		if (!m_isActive) return;

		/** 今回のフィーバーで投下する総数がfeverDropCountを超える場合は補充しない */
		if (m_totalQueuedCount >= m_feverDropCount) return;

		/** 捕獲された分だけ投下キューに追加し、連続して降り続けるようにする */
		m_pendingDropCount++;
		m_totalQueuedCount++;
	}
}
