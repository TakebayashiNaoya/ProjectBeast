/**
 * @file FeverTimeManager.cpp
 * @brief フィーバータイムを管理するクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "FeverTimeManager.h"
#include "TimeManager.h"
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

		m_feverStartTime = util::JsonConverter::ToFloat(json, "feverStartTime", m_feverStartTime);
		m_dropInterval   = util::JsonConverter::ToFloat(json, "dropInterval", m_dropInterval);
		m_dropHeight     = util::JsonConverter::ToFloat(json, "dropHeight", m_dropHeight);
		m_feverDropCount = util::JsonConverter::ToInt(json, "feverDropCount", m_feverDropCount);
		m_feverEnabled   = util::JsonConverter::ToBool(json, "feverEnabled", m_feverEnabled);
	}


	void FeverTimeManager::Update()
	{
		if (!m_isActive && !m_hasTriggered)
		{
			/** 残り時間が閾値を下回った瞬間に一度だけフィーバータイムへ入る
			 *  （全員捕獲トリガーとは別経路。どちらか早い方が優先される） */
			const float curTime = TimeManager::GetInstance().GetCurTime();
			if (curTime > 0.0f && curTime <= m_feverStartTime)
			{
				TryStartFever("time");
			}
		}

		if (m_pendingDropCount <= 0)
		{
			/** 今回のフィーバーで積む予定の総数まで使い切っていれば、投下待ちが再び増えることはない
			 *  （捕獲による補充はfeverDropCountに達するまでしか起きないため）ので、ここでフィーバーを終了する */
			if (m_isActive && m_totalQueuedCount >= m_feverDropCount)
			{
				m_isActive = false;
			}
			return;
		}

		m_dropTimer += g_gameTime->GetFrameDeltaTime();
		if (m_dropTimer < m_dropInterval) return;

		m_dropTimer -= m_dropInterval;
		m_pendingDropCount--;
		actor::ChildPenguinManager::GetInstance()->SpawnFromSky(m_dropHeight);
	}


	void FeverTimeManager::TryStartFeverOnAllCaught()
	{
		TryStartFever("all_caught");
	}


	void FeverTimeManager::TryStartFever(const char* reason)
	{
		/** フィーバー無効なステージ（チュートリアル等）、または既に発生済みなら何もしない */
		if (!m_feverEnabled || m_hasTriggered) return;

		StartFever(reason);
	}


	void FeverTimeManager::StartFever(const char* reason)
	{
		m_isActive = true;
		m_hasTriggered = true;
		m_dropTimer = 0.0f;

		// デバイスロスト調査：フィーバー開始時点のVRAM使用量を記録する。
		// クラッシュするとプレイログは書き出されないため、ファイルへ直接追記する
		{
			double usageMB = 0.0;
			double budgetMB = 0.0;
			g_graphicsEngine->QueryVideoMemoryMB(usageMB, budgetMB);

			char buf[256];
			sprintf_s(buf, "[VRAM] fever start     usage %.1f MB / budget %.1f MB heaps %d\n",
				usageMB, budgetMB, nsK2EngineLow::g_numDescriptorHeapLive);
			OutputDebugStringA(buf);

			FILE* fp = nullptr;
			fopen_s(&fp, "Logs/vram_trace.txt", "a");
			if (fp)
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d %s",
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buf);
				fclose(fp);
			}
		}

		/** フィーバー中BGMに切り替える */
		SoundManager::Get().PlayBGM(enSoundKind_FeverTime);

		/** それまでの捕獲数によらず、固定数を投下キューの初期数にする */
		m_pendingDropCount = m_feverDropCount;
		m_totalQueuedCount = m_feverDropCount;

		// 残り時間による自動突入か、全員捕獲による早期突入かを区別して記録する
		if (auto* lm = GameLogManager::GetInstance())
		{
			lm->QueueEvent({
				{ "ev",         "fever_start" },
				{ "reason",     reason },
				{ "drop_count", m_feverDropCount }
			});
		}
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
