/**
 * @file FeverTimeManager.cpp
 * @brief フィーバータイムを管理するクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "FeverTimeManager.h"
#include "TimeManager.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"
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
	}


	void FeverTimeManager::Update()
	{
		if (!m_isActive)
		{
			/** 残り時間が閾値を下回った瞬間に一度だけフィーバータイムへ入る */
			const float curTime = TimeManager::GetInstance().GetCurTime();
			if (curTime <= 0.0f || curTime > m_feverStartTime) return;

			StartFever();
		}

		if (m_pendingDropCount <= 0) return;

		m_dropTimer += g_gameTime->GetFrameDeltaTime();
		if (m_dropTimer < m_dropInterval) return;

		m_dropTimer -= m_dropInterval;
		m_pendingDropCount--;
		actor::ChildPenguinManager::GetInstance()->SpawnFromSky(m_dropHeight);
	}


	void FeverTimeManager::StartFever()
	{
		m_isActive = true;
		m_dropTimer = 0.0f;

		/** 現在の野良数（生存数−救出済み数）とステージ初期数の差分を投下キューの初期数にする */
		auto* manager = actor::ChildPenguinManager::GetInstance();
		const int cap = manager->GetInitialTotalCount();
		const int strayNum = manager->GetChildPenguinNum() - manager->GetRescuedNum();
		m_pendingDropCount = max(0, cap - strayNum);
	}


	void FeverTimeManager::OnPenguinCaught()
	{
		/** フィーバー中でなければ何もしない（通常時の捕獲では補充しない） */
		if (!m_isActive) return;

		/** 捕獲された分だけ投下キューに追加し、連続して降り続けるようにする */
		m_pendingDropCount++;
	}
}
