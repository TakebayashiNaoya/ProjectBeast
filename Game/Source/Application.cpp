#include "stdafx.h"

#include "Application.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Core/DebugWindow.h"
#include "Source/Core/Fade.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Effect/DecalManager.h"
#include "Source/Effect/EffectManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Scene/SceneManager.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
#if defined(_DEBUG) || defined(K2_DEBUG)
	namespace
	{
		/** トーンマップのデバッグ表示のセクション名 */
		constexpr const char* TONE_MAP_DEBUG_LABEL = u8"トーンマップ";


		/**
		 * @brief トーンマップのデバッグUIを描画する
		 * @details 方式の切り替えと露出などの調整を実行中に行えるようにする。
		 *          方式を変えると、その方式の既定露出が自動で適用される。
		 */
		void DrawToneMapDebug()
		{
			auto& toneMap = g_renderingEngine->GetPostEffectManager().GetToneMap();

			// enNoneで初期化した場合はスプライトを作っていないので切り替えできない
			if (!toneMap.IsSwitchable())
			{
				ImGui::TextWrapped(
					u8"RenderingEngine::InitPostEffectManager() が enNone で初期化しているため、"
					u8"実行中の切り替えはできません。enNone以外で起動してください。");
				return;
			}

			// 方式の切り替え
			int currentType = static_cast<int>(toneMap.GetToneMapType());
			const int typeNum = static_cast<int>(nsBeastEngine::EnToneMapType::enNum);
			if (ImGui::BeginCombo(u8"方式", nsBeastEngine::ToneMap::GetTypeName(toneMap.GetToneMapType())))
			{
				for (int i = 0; i < typeNum; i++)
				{
					const auto type = static_cast<nsBeastEngine::EnToneMapType>(i);
					const bool isSelected = (i == currentType);
					if (ImGui::Selectable(nsBeastEngine::ToneMap::GetTypeName(type), isSelected))
					{
						toneMap.SetToneMapType(type);
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			// enNoneを選んでいる間はパラメーターが効かないので触らせない
			const bool isNone = (toneMap.GetToneMapType() == nsBeastEngine::EnToneMapType::enNone);
			if (isNone)
			{
				ImGui::TextDisabled(u8"トーンマップ無効（比較用の基準）");
				return;
			}

			// 露出
			float exposure = toneMap.GetExposure();
			if (ImGui::SliderFloat(u8"露出", &exposure, 0.1f, 6.0f, "%.2f"))
			{
				toneMap.SetExposure(exposure);
			}

			// ホワイトポイントはenReinhardExtendedでのみ効く
			const bool isExtended =
				(toneMap.GetToneMapType() == nsBeastEngine::EnToneMapType::enReinhardExtended);
			if (!isExtended)
			{
				ImGui::BeginDisabled();
			}
			float whitePoint = toneMap.GetWhitePoint();
			if (ImGui::SliderFloat(u8"ホワイトポイント", &whitePoint, 1.0f, 16.0f, "%.2f"))
			{
				toneMap.SetWhitePoint(whitePoint);
			}
			if (!isExtended)
			{
				ImGui::EndDisabled();
				ImGui::TextDisabled(u8"ホワイトポイントはenReinhardExtendedでのみ有効");
			}

			// 輝度ベース／RGBベースの切り替え
			bool isLuminanceBased = toneMap.IsLuminanceBased();
			if (ImGui::Checkbox(u8"輝度ベースで適用（彩度を保つ）", &isLuminanceBased))
			{
				toneMap.SetLuminanceBased(isLuminanceBased);
			}
			ImGui::TextDisabled(u8"OFFにするとRGB各チャンネル独立。明部の色が白へ抜ける");

			// ガンマ補正
			bool isApplyGamma = toneMap.IsApplyGamma();
			if (ImGui::Checkbox(u8"sRGBエンコード（ガンマ補正）", &isApplyGamma))
			{
				toneMap.SetApplyGamma(isApplyGamma);
			}
			ImGui::TextDisabled(u8"ONにすると全体が大きく明るくなる。ライト強度の再調整が必要");
		}
	}
#endif


	Application::Application()
	{
		nsBeastEngine::nsCollision::PhysicsWorld::Initialize();
		nsBeastEngine::OcclusionDitherManager::Initialize();
		camera::CameraManager::CreateInstance();
		core::ParameterManager::CreateInstance();
		core::Fade::Create();
		SoundManager::CreateInstance();
		NoiseManager::CreateInstance();
		SceneManager::CreateInstance();
		EffectManager::CreateInstance();

#if defined(_DEBUG) || defined(K2_DEBUG)
		DebugWindow::Get().Register(TONE_MAP_DEBUG_LABEL, DrawToneMapDebug);
#endif
	}


	Application::~Application()
	{
#if defined(_DEBUG) || defined(K2_DEBUG)
		DebugWindow::Get().Unregister(TONE_MAP_DEBUG_LABEL);
#endif
		SceneManager::DestroyInstance();
		NoiseManager::DestroyInstance();
		SoundManager::DestroyInstance();
		EffectManager::DestroyInstance();
		camera::CameraManager::DestroyInstance();
		core::ParameterManager::DestroyInstance();
		core::Fade::Delete();
		nsBeastEngine::nsCollision::PhysicsWorld::Finalize();
		nsBeastEngine::OcclusionDitherManager::Finalize();
	}


	void Application::Update()
	{
		camera::CameraManager::Get().Update(6);
		core::ParameterManager::Get()->Update();
		SceneManager::GetInstance()->Update();
		SoundManager::Get().Update();
		// 1フレーム前のフラスタムを使用してエフェクトのカリングを行う
		// ModelRenderの既存カリングと同じ挙動であり、許容される仕様
		EffectManager::Get().Update(g_renderingEngine->GetFrustum());
		core::Fade::Get().Update();

		app::effect::DecalManager::Get().Update();
	}


	void Application::Render(RenderContext& rc)
	{
#if defined(_DEBUG) || defined(K2_DEBUG)
		DebugWindow::Get().Render();
#endif
		SceneManager::GetInstance()->Render(rc);

		app::effect::DecalManager::Get().Render(rc);

		core::Fade::Get().Render(rc);
	}
}