/**
 * @file FinishMenu.cpp
 * @brief FINISH演出の動的処理クラス
 * @author 立山
 */
#include "stdafx.h"
#include "FinishMenu.h"
#include "Source/UI/UIAnimation.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			struct FinishInfo
			{
				uint32_t key;
			};

			constexpr FinishInfo FINISH_ICON_KEYS[] =
			{
				{ Hash32("FinishIcon") }
			};
			constexpr int FINISH_ICON_SIZE = static_cast<int>(std::size(FINISH_ICON_KEYS));
		}


		//------------------------------------------------------------
		// FinishIcon
		//------------------------------------------------------------

		FinishIcon::FinishIcon()
			: m_icon(nullptr)
		{}

		FinishIcon::~FinishIcon()
		{}

		void FinishIcon::Update()
		{}

		void FinishIcon::SetUIIcon(UIIcon* icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "FinishIcon: UIIcon の登録に失敗しました。");
		}


		//------------------------------------------------------------
		// FinishMenu
		//------------------------------------------------------------

		FinishMenu::FinishMenu()
			: m_isStarted(false)
			, m_isFinished(false)
			, m_timer(0.0f)
		{}


		void FinishMenu::Update()
		{
			// 演出が開始していなければ全アイコン非表示で終わり
			if (!m_isStarted)
			{
				for (auto& icon : m_finishIconMap)
				{
					icon.second->SetIsDraw(false);
				}
				return;
			}

			// 演出済みなら何もしない
			if (m_isFinished) return;

			// 経過時間を進める
			m_timer += g_gameTime->GetFrameDeltaTime();
			if (m_timer >= FINISH_DURATION)
			{
				m_isFinished = true;
			}

			// アイコン更新（アニメーションが走っている）
			for (auto& icon : m_finishIconMap)
			{
				icon.second->SetIsDraw(true);
				icon.second->Update();
			}

			// MenuBase の更新（Canvas 更新）
			FinishClass::Update();
		}


		void FinishMenu::StartFinish()
		{
			if (m_isStarted) return;

			m_isStarted = true;
			m_isFinished = false;
			m_timer = 0.0f;

			// アイコンのアニメーションを再生
			for (const auto& info : FINISH_ICON_KEYS)
			{
				auto* icon = GetUI<UIIcon>(info.key);
				if (icon)
				{
					icon->PlayAnimation();
				}
			}
		}


		void FinishMenu::InitializeLogic()
		{
			m_finishIconMap.clear();
			m_finishIconMap.reserve(FINISH_ICON_SIZE);

			for (const auto& info : FINISH_ICON_KEYS)
			{
				Icon finishIcon = std::make_unique<FinishIcon>();
				auto* icon = GetUI<UIIcon>(info.key);
				finishIcon->SetUIIcon(icon);
				m_finishIconMap.emplace(info.key, std::move(finishIcon));

				if (icon)
				{
					// スケールアップ → 通常サイズ のアニメーション
					auto scaleAnim = std::make_unique<UIScaleAnimation>();
					Vector3 scaleStart(0.0f, 0.0f, 0.0f);
					Vector3 scaleEnd(1.0f, 1.0f, 1.0f);
					scaleAnim->SetParameter(
						scaleStart,
						scaleEnd,
						0.4f,
						util::EasingType::EaseInOut,
						util::LoopMode::Once
					);
					icon->AddAnimation(Hash32("FinishScaleAnim"), std::move(scaleAnim));

					// 表示後フェードアウト
					auto colorAnim = std::make_unique<UIColorAnimation>();
					Vector4 colorStart(1.0f, 1.0f, 1.0f, 1.0f);
					Vector4 colorEnd(1.0f, 1.0f, 1.0f, 0.0f);
					colorAnim->SetParameter(
						colorStart,
						colorEnd,
						1.0f,
						util::EasingType::Linear,
						util::LoopMode::Once
					);
					icon->AddAnimation(Hash32("FinishColorAnim"), std::move(colorAnim));
				}
			}
		}
	}
}