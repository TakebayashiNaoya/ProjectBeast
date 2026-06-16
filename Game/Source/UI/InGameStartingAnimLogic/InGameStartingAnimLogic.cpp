/**
 * @file InGameStartingAnimLogic.cpp
 * @brief ゲーム開始時のアニメーションロジッククラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "InGameStartingAnimLogic.h"

#include "Source/UI/Menu.h"
#include "Source/UI/Parts/UIParts.h"


namespace app
{
	namespace ui
	{
		InGameStartingAnimLogic::InGameStartingAnimLogic()
			: m_totalSize(0)
			, m_iconNames{}
			, m_digitNames{}
			, m_uiParts{}
			, m_menu(nullptr)
			, m_startOffset(Vector3::Zero)
			, m_duration(1.0f)
			, m_animState(AnimState::NotStarted)
		{}

		void InGameStartingAnimLogic::Initialize(
			MenuBase* menu,
			const std::vector<std::string> iconNames,
			const std::vector<std::string> digitNames,
			const Vector3 startOffset,
			float duration,
			const std::vector<std::string> textNames
		)
		{
			if (menu == nullptr) return;
			m_menu = menu;
			m_startOffset = startOffset;
			m_duration = util::clamp(duration, 0.1f, 10.0f);
			m_totalSize = iconNames.size() + digitNames.size() + textNames.size();
			m_iconNames = iconNames;
			m_digitNames = digitNames;
			m_textNames = textNames;

			m_uiParts.reserve(m_totalSize);

			m_animState = AnimState::NotStarted;
		}

		void InGameStartingAnimLogic::Update()
		{
			// メニュークラスがない場合は処理しない
			if (m_menu == nullptr) return;

			UpdateUIParts();


			auto ForEach = [&](std::function<void(UIBase*)> func) {
				for (const auto& ui : m_uiParts) {
					if (ui == nullptr) continue;
					func(ui);
				}
				};

			switch (m_animState)
			{
			case AnimState::NotStarted:
			{
				ForEach([&](UIBase* ui) {
					const Vector3 jsonPos = ui->m_transform.m_localTransform.m_position;
					const Vector3 startPos = jsonPos + m_startOffset;
					auto trsAnim = std::make_unique<UITranslateAnimation>();

					trsAnim->SetParameter(
						startPos,
						jsonPos,
						m_duration,
						util::EasingType::EaseInOut,
						util::LoopMode::Once
					);
					trsAnim->SetFunc([ui](const Vector3& pos)
						{
							ui->m_transform.m_localTransform.m_position = pos;
						});

					ui->AddAnimation(Hash32("GameStartFadeIn"), std::move(trsAnim));
					ui->PlayAnimation();
					});

				m_animState = AnimState::Playing;
				break;
			}
			case AnimState::Playing:
			{
				bool allFinished = true;
				ForEach([&](UIBase* ui)
					{
						auto* anim = ui->FindAnimation(Hash32("GameStartFadeIn"));
						if (anim && anim->IsPlayAnimation()) allFinished = false;
					});
				if (allFinished) m_animState = AnimState::Finished;
				break;
			}

			case AnimState::Finished:
				return;
			}
		}


		void InGameStartingAnimLogic::UpdateUIParts()
		{
			m_uiParts.clear();
			m_uiParts.reserve(m_totalSize);

			for (const auto& name : m_iconNames)
				m_uiParts.push_back(m_menu->GetUI<UIIcon>(Hash32(name.c_str())));
			for (const auto& name : m_digitNames)
				m_uiParts.push_back(m_menu->GetUI<UIDigit>(Hash32(name.c_str())));
			for (const auto& name : m_textNames)
				m_uiParts.push_back(m_menu->GetUI<UIText>(Hash32(name.c_str())));
		}
	}
}