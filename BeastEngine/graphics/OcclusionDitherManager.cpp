/**
 * @file OcclusionDitherManager.cpp
 * @brief ディザリングを管理するクラス
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "OcclusionDitherManager.h"
#include "ModelRender.h"


namespace nsBeastEngine
{
	OcclusionDitherManager* OcclusionDitherManager::m_instance = nullptr;


	namespace
	{
		constexpr float TARGET_HEIGHT_OFFSET = 30.0f;
	}


	void OcclusionDitherManager::Register(ModelRender* modelRender, float ditherStrength, float cylinderRadius)
	{
		if (modelRender == nullptr) { return; }

		// SetPlayerTarget()で登録済みのModelRenderは遮蔽対象リストに追加しない
		if (modelRender == m_playerModelRender) { return; }

		// 同一ModelRenderの二重登録を防ぐ
		// 既に登録済みであれば強度・半径を上書きして早期returnする
		for (auto& entry : m_entries)
		{
			if (entry.modelRender == modelRender)
			{
				entry.cb.ditherStrength = ditherStrength;
				entry.cb.cylinderRadius = cylinderRadius;
				return;
			}
		}

		// DitherEntryを生成する
		// std::listはpush_back後も既存要素のアドレスが不変であることが保証されている
		m_entries.emplace_back();
		DitherEntry& entry = m_entries.back();
		entry.modelRender = modelRender;
		entry.cb.ditherStrength = ditherStrength;
		entry.cb.cylinderRadius = cylinderRadius;

		// cbのポインタをModelRenderのb3にセットする
		// GBufferパスのRenderToGBuffer.fxがb3でDitherCbを参照する
		// ModelRenderのDraw()のたびに自動でGPUに転送される
		modelRender->SetExpandConstantBuffer3(&entry.cb);
	}


	void OcclusionDitherManager::Unregister(ModelRender* modelRender)
	{
		m_entries.remove_if([modelRender](const DitherEntry& entry)
			{
				return entry.modelRender == modelRender;
			});
	}


	void OcclusionDitherManager::SetPlayerTarget(ModelRender* modelRender)
	{
		if (modelRender == nullptr) { return; }

		// 同一ターゲットの再設定時はリスト走査しない
		if (m_playerModelRender == modelRender) { return; }

		m_playerModelRender = modelRender;

		// Register()が先に呼ばれていた場合は遮蔽対象リストから除外する
		Unregister(modelRender);
	}


	void OcclusionDitherManager::Update()
	{
		if (m_entries.empty()) { return; }

		if (m_playerModelRender == nullptr) { return; }

		// カメラのワールド座標を取得する
		const Vector3 cameraWorldPos = g_camera3D->GetPosition();

		// プレイヤーのワールド座標を取得する
		const Vector3 playerWorldPos = m_playerModelRender->GetPosition()
			+ Vector3(0.0f, TARGET_HEIGHT_OFFSET, 0.0f);

		// 全エントリのDitherCbにカメラとプレイヤーのワールド座標を反映する
		// cylinderRadius, depthBias, ditherStrength はRegister時に設定済みの固有値を維持する
		for (auto& entry : m_entries)
		{
			if (entry.modelRender == nullptr)
			{
				continue;
			}

			entry.cb.cameraWorldPos = cameraWorldPos;
			entry.cb.targetWorldPos = playerWorldPos;
		}
	}
}