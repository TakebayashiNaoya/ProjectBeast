/**
 * @file FeverIconMenu.cpp
 * @brief フィーバータイム開始時にアイコンを画面上から下へ落下させる演出
 * @author 竹林
 */
#include "stdafx.h"
#include "FeverIconMenu.h"
#include "Source/Manager/FeverTimeManager.h"
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UI/Animation/UIAnimationParameter.h"
#include "Source/UIAnimationTypes.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 落下開始Y座標（画面上端より外側から降ってくるように） */
			constexpr float FEVER_ICON_START_Y = 575.0f;
			/** 落下終了Y座標（画面下端より外側まで抜けるように） */
			constexpr float FEVER_ICON_END_Y = -575.0f;
			/** 落下にかかる時間（秒） */
			constexpr float FEVER_FALL_DURATION = 2.5f;

			/** feverFallAnimの定義を含むアニメーションパラメーターJSON */
			const char* FEVER_ANIM_PARAMETER_JSON_PATH = "Assets/parameter/UI/fever/FeverAnimParameter.json";
		}


		FeverIconMenu::FeverIconMenu()
		{
			/** UIAnimationFactory::AttachはUIAnimationParameterに定義が無いと失敗するため、先に読み込んでおく */
			UIAnimationParameter::Get().Load(FEVER_ANIM_PARAMETER_JSON_PATH);
		}


		void FeverIconMenu::Update()
		{
			auto* icon = GetUI<UIIcon>(Hash32("FeverIcon"));

			const bool isFeverActive = FeverTimeManager::GetInstance()->IsActive();

			/** フィーバー開始の瞬間（false→true）にのみ落下演出を開始する */
			if (isFeverActive && !m_wasFeverActive && icon)
			{
				Vector3 startPos = m_defaultPos;
				startPos.y = FEVER_ICON_START_Y;
				Vector3 endPos = m_defaultPos;
				endPos.y = FEVER_ICON_END_Y;

				icon->m_transform.m_localTransform.m_position = startPos;
				icon->m_isDraw = true;

				UIAnimationFactory::Attach<UITranslateAnimation>(icon, animKey::FEVER_FALL_ANIM_KEY);
				if (auto* anim = icon->FindAnimation(animKey::FEVER_FALL_ANIM_KEY))
				{
					auto* translateAnim = static_cast<UITranslateAnimation*>(anim);
					translateAnim->SetParameter(
						startPos, endPos,
						FEVER_FALL_DURATION,
						util::EasingType::Linear,
						util::LoopMode::Once
					);
					anim->PlayAnimation();
				}

				m_isFalling = true;
			}
			m_wasFeverActive = isFeverActive;

			if (m_isFalling)
			{
				if (icon) icon->UpdateAnimation();

				auto* anim = icon ? icon->FindAnimation(animKey::FEVER_FALL_ANIM_KEY) : nullptr;
				if (!anim || !anim->IsPlayAnimation())
				{
					m_isFalling = false;
					if (icon) icon->m_isDraw = false;
				}
			}

			MenuBase::Update();
		}


		void FeverIconMenu::InitializeLogic()
		{
			m_wasFeverActive = false;
			m_isFalling = false;

			auto* icon = GetUI<UIIcon>(Hash32("FeverIcon"));
			if (icon)
			{
				m_defaultPos = icon->m_transform.m_localTransform.m_position;
				icon->m_isDraw = false;
			}
		}
	}
}
