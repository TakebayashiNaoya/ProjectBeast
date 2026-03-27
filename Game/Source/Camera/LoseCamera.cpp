/**
 * @file CameraSteering.cpp
 * @brief カメラのステアリング処理
 * @author 藤谷
 */
#include "stdafx.h"
#include "LoseCamera.h"


namespace
{
}


namespace app
{
	namespace camera
	{
		namespace
		{
			constexpr float LIMITE_HEIGHT = 220.0f;
		}

		void LoseCamera::Update()
		{
			m_timer += 0.016f; // 本来はdeltaTimeを使用

			// 演出：ゆっくりと上昇しながら、キャラを見下ろす
			// 徐々に高く、遠くへ
			float height = 50.0f + (m_timer * 40.0f);
			float distance = 100.0f + (m_timer * 20.0f);
			// 高さの上限を設定。
			if (height > LIMITE_HEIGHT)
			{
				height = LIMITE_HEIGHT;
				return;
			}

			m_data.position = m_targetPos + Vector3(0, height, -distance);
			m_data.target = m_targetPos; // 倒れたキャラを注視し続ける

			// 演出：少しずつ視野角(FOV)を広げて、孤独感を出す
			m_data.fov = Math::DegToRad(60.0f + (m_timer * 5.0f));
		}




		/******************************************************/


		void DefeatCamera::SetTarget(const Vector3& playerPos)
		{
			// --- 演出設定 ---
			float viewHeight = 300.0f;     // どれくらいの高さから見下ろすか
			float horizontalOffset = 50.0f; // 真上だとキャラが分かりにくい場合、少し手前に引く

			// 1. カメラの位置を計算 (プレイヤーの上空後方)
			m_data.position = playerPos + Vector3(0.0f, viewHeight, -horizontalOffset);

			// 2. 注視点はプレイヤーの足元（地面）
			m_data.target = playerPos;

			// 3. 上方向ベクトルは通常 (真下を見下ろす場合、ここを調整することもあるが、まずはデフォルト)
			m_data.up = Vector3::Up;

			// 4. 画角は標準
			m_data.fov = Math::DegToRad(60.0f);
		}


		void DefeatCamera::OnEnter()
		{}


		void DefeatCamera::Update()
		{}
	}
}