/**
 * @file SubCameraManager.cpp
 * @brief サブカメラの管理を行うクラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "SubCameraManager.h"
#include "CameraSystem.h"
#include "Graphics/RenderingEngine.h"


namespace nsBeastEngine
{
	namespace
	{
		/** サブカメラのRenderTarget幅 */
		constexpr UINT SUB_CAMERA_RT_WIDTH = 480;
		/** サブカメラのRenderTarget高さ */
		constexpr UINT SUB_CAMERA_RT_HEIGHT = 270;
		/** 背景（枠）スプライトの片側の厚み（px、scale=1時） */
		constexpr UINT SPRITE_BORDER_SIZE = 10;
		/** サブカメラのNearクリップ */
		constexpr float SUB_CAMERA_NEAR = 1.0f;
		/** サブカメラのFarクリップ */
		constexpr float SUB_CAMERA_FAR = 13000.0f;

		/** 小窓の表示位置X（画面左下、スクリーン座標中心=(0,0)系） */
		constexpr float SPRITE_POS_X = -500.0f;
		/** 小窓の表示位置Y（完全表示時、スクリーン座標中心=(0,0)系） */
		constexpr float SPRITE_POS_Y = -250.0f;
		/** スライドアウト時に画面外に出るまでの余白（px） */
		constexpr float SPRITE_SLIDE_MARGIN = 50.0f;
		/** スライドアニメーション速度係数（大きいほど速い） */
		constexpr float SPRITE_SLIDE_SPEED = 10.0f;
	}


	SubCameraManager* SubCameraManager::m_instance = nullptr;


	void SubCameraManager::Begin(std::function<void()> onBegin)
	{
		// スライドアウト中に再起動を要求された場合はペンディングエンドをキャンセルするだけ
		// サブカメラはまだ生きているので生成し直さない
		if (m_isActive && m_pendingEnd)
		{
			m_pendingEnd = false;
			m_pendingEndCallback = nullptr;
			if (onBegin) onBegin();
			return;
		}

		if (m_isActive) return;

		CameraSystem::Get().CreateSubCamera();

		auto* subCamera = CameraSystem::Get().GetSubCamera();
		subCamera->SetNear(SUB_CAMERA_NEAR);
		subCamera->SetFar(SUB_CAMERA_FAR);

		InitRenderTargetAndSprite();

		m_isActive = true;
		m_targetVisible = false;
		m_slideProgress = 0.0f;
		m_pendingEnd = false;
		m_pendingEndCallback = nullptr;

		if (onBegin) onBegin();
	}


	void SubCameraManager::End(std::function<void()> onEnd)
	{
		if (!m_isActive) return;
		if (m_pendingEnd) return;  // すでに終了アニメーション中

		// スライドアウトを開始し、完了後に実際の終了処理を行う
		m_targetVisible = false;
		m_pendingShow = false;
		m_pendingEnd = true;
		m_pendingEndCallback = onEnd;
	}


	void SubCameraManager::SetSpriteVisible(bool visible)
	{
		if (visible)
		{
			// スライドアウト中（progress > 0 かつ target = false）なら一度完全に隠れるまで待つ
			const bool isSlidingOut = !m_targetVisible && (m_slideProgress > 0.01f);
			if (isSlidingOut)
			{
				m_pendingShow = true;
			}
			else
			{
				m_targetVisible = true;
				m_pendingShow = false;
			}
		}
		else
		{
			m_targetVisible = false;
			m_pendingShow = false;
		}
	}


	void SubCameraManager::Update()
	{
		if (!m_isActive) return;

		UpdateSubCameraTransform();

		// スライドアニメーション: 目標値(0 or 1)へ指数減衰で近づく
		const float target = m_targetVisible ? 1.0f : 0.0f;
		const float dt = g_gameTime->GetFrameDeltaTime();
		m_slideProgress += (target - m_slideProgress) * (1.0f - expf(-SPRITE_SLIDE_SPEED * dt));

		// スライドアウト完了後に終了処理を実行（pendingEnd 優先）
		if (m_pendingEnd && m_slideProgress < 0.01f)
		{
			m_isActive = false;
			m_pendingEnd = false;
			m_pendingShow = false;
			m_slideProgress = 0.0f;
			CameraSystem::Get().DestroySubCamera();
			if (m_pendingEndCallback)
			{
				m_pendingEndCallback();
				m_pendingEndCallback = nullptr;
			}
			return;
		}

		// スライドアウト完了後に保留中のスライドインを開始する
		if (m_pendingShow && m_slideProgress < 0.01f)
		{
			m_targetVisible = true;
			m_pendingShow = false;
		}
	}


	void SubCameraManager::ForceEnd()
	{
		if (!m_isActive) return;
		m_isActive = false;
		m_pendingEnd = false;
		m_pendingShow = false;
		m_targetVisible = false;
		m_slideProgress = 0.0f;
		m_pendingEndCallback = nullptr;
		m_renderingBlocked = false;
		CameraSystem::Get().DestroySubCamera();
	}


	void SubCameraManager::RenderToScreen(nsK2EngineLow::RenderContext& rc)
	{
		if (!m_isActive) return;
		if (m_slideProgress < 0.01f) return;
		if (m_renderingBlocked) return;

		// スケール変化時に左下コーナーを固定する
		// スケール=1 のとき SPRITE_POS_X/Y が中心座標。
		// スケールが変わるとスプライト半サイズが変わるぶん、中心を右上方向にずらして左下を固定する。
		const float halfW = static_cast<float>(SUB_CAMERA_RT_WIDTH) * 0.5f;
		const float halfH = static_cast<float>(SUB_CAMERA_RT_HEIGHT) * 0.5f;
		const float posX = SPRITE_POS_X + (m_spriteScale - 1.0f) * halfW;
		const float posY = SPRITE_POS_Y + (m_spriteScale - 1.0f) * halfH;

		// progress=0 のとき完全に画面外へ出るスライド量を画面高さから動的に計算する。
		// 固定値にすると解像度によってはスライドが途中で止まって見えるため。
		// slideDist = screenHalfH + spriteHalfH + posY + margin
		//   (posYは負値。1080p例: 540 + 135 + (-250) + 50 = 475)
		const float screenHalfH = static_cast<float>(g_graphicsEngine->GetFrameBufferHeight()) * 0.5f;
		const float spriteHalfH = halfH * m_spriteScale;
		const float slideDist = screenHalfH + spriteHalfH + posY + SPRITE_SLIDE_MARGIN;

		// progress=0 のとき下方向にオフセット（画面外）、progress=1 のとき定位置
		const float animY = posY - slideDist * (1.0f - m_slideProgress);

		// 背景（枠）を先に描画し、その上にサブビューを重ねる
		m_bgSprite.Update(
			Vector3(posX, animY, 0.0f),
			Quaternion::Identity,
			Vector3(m_spriteScale, m_spriteScale, 1.0f)
		);
		m_bgSprite.Draw(rc);

		m_sprite.Update(
			Vector3(posX, animY, 0.0f),
			Quaternion::Identity,
			Vector3(m_spriteScale, m_spriteScale, 1.0f)
		);
		m_sprite.Draw(rc);
	}


	void SubCameraManager::InitRenderTargetAndSprite()
	{
		// 背景（枠）スプライト: DDS テクスチャを所有するため Begin() のたびに
		// 再 Init() するとデファードリリースでクラッシュする。初回のみ初期化する。
		if (!m_bgSpriteInitialized)
		{
			SpriteInitData bgInitData;
			bgInitData.m_ddsFilePath[0] = "Assets/spriteData/UI/Icon/warning.DDS";
			bgInitData.m_width  = SUB_CAMERA_RT_WIDTH  + SPRITE_BORDER_SIZE * 2;
			bgInitData.m_height = SUB_CAMERA_RT_HEIGHT + SPRITE_BORDER_SIZE * 2;
			bgInitData.m_fxFilePath = "Assets/shader/sprite.fx";
			bgInitData.m_alphaBlendMode = AlphaBlendMode_Trans;
			bgInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			m_bgSprite.Init(bgInitData);
			m_bgSpriteInitialized = true;
		}

		SpriteInitData spriteInitData;
		spriteInitData.m_textures[0] = &g_renderingEngine->GetSubCameraRenderTarget().GetRenderTargetTexture();
		spriteInitData.m_width = SUB_CAMERA_RT_WIDTH;
		spriteInitData.m_height = SUB_CAMERA_RT_HEIGHT;
		spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
		spriteInitData.m_alphaBlendMode = AlphaBlendMode_None;
		spriteInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_sprite.Init(spriteInitData);
	}


	void SubCameraManager::UpdateSubCameraTransform()
	{
		auto* subCamera = CameraSystem::Get().GetSubCamera();
		if (subCamera == nullptr) return;

		subCamera->SetPosition(m_cameraPosition);
		subCamera->SetTarget(m_targetPosition);
	}
}
