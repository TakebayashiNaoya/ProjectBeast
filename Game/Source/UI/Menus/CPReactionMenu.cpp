/**
 * @file CPReactionMenu.cpp
 * @brief 子ペンギンのリアクションUIクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionMenu.h"
#include "Source/actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/UI/Animation/UIAnimationFactory.h"
#include "Source/UI/Model/CPReactionStatus.h"
#include "Source/UIAnimationTypes.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		void CPReactionMenu::PlayUIAnimation(const EnReactionType type)
		{

			m_type = type;

			if (m_type == EnReactionType::Trouble)
			{
				m_speechBubble->m_isDraw = true;
				m_troubleReaction->m_isDraw = true;
				m_happyReaction->m_isDraw = false;
			}
			else if (m_type == EnReactionType::Happy)
			{
				m_speechBubble->m_isDraw = true;
				m_troubleReaction->m_isDraw = false;
				m_happyReaction->m_isDraw = true;
			}

			SetAnimation();
			m_timer = 0.0f;
		}


		CPReactionMenu::CPReactionMenu()
			: m_target(nullptr)
			, m_speechBubble(nullptr)
			, m_troubleReaction(nullptr)
			, m_happyReaction(nullptr)
			, m_type(EnReactionType::None)
			, m_timer(0.0f)
		{}


		CPReactionMenu::~CPReactionMenu()
		{}


		void CPReactionMenu::Update()
		{

			DrawFlagUpdate();

			MenuBase::Update();
		}


		void CPReactionMenu::InitializeLogic()
		{
			m_status.reset();
			m_status = std::make_unique<CPReactionStatus>();
			m_status->SetUpUI();

			m_animStatus.reset();
			m_animStatus = std::make_unique<CPReactionAnimStatus>();
			m_animStatus->SetUpUI();

			// アイコンのポインタを取得
			m_speechBubble = GetUI<UIIcon>(Hash32("speechBubble"));
			m_troubleReaction = GetUI<UIIcon>(Hash32("troubleReaction"));
			m_happyReaction = GetUI<UIIcon>(Hash32("happyReaction"));

			// アイコンの描画フラグをリセット
			m_speechBubble->m_isDraw = false;
			m_troubleReaction->m_isDraw = false;
			m_happyReaction->m_isDraw = false;
		}


		void CPReactionMenu::PositionUpdate()
		{
			if (!m_target) return;
			const Vector3 targetPosition = m_target->GetTransform().m_position;

			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, targetPosition);

			const Vector3 prevPosition = Vector3(
				screenPos.x,
				screenPos.y + m_status->GetIconOffsetY(),
				0.0f
			);

			m_speechBubble->m_transform.m_localTransform.m_position = prevPosition + m_status->GetSpeechBubbleOffset();
			m_troubleReaction->m_transform.m_localTransform.m_position = prevPosition + m_status->GetTroubleReactionOffset();
			m_happyReaction->m_transform.m_localTransform.m_position = prevPosition + m_status->GetHappyReactionOffset();
		}


		void CPReactionMenu::DrawFlagUpdate()
		{

			if (!m_target)
			{
				m_type = EnReactionType::None;
			}


			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			switch (m_type)
			{
			case EnReactionType::Trouble:
			{
				m_speechBubble->m_isDraw = true;
				m_troubleReaction->m_isDraw = true;
				m_happyReaction->m_isDraw = false;
				UpdateAnimation();
				PositionUpdate();
				break;
			}
			case EnReactionType::Happy:
			{
				m_speechBubble->m_isDraw = true;
				m_troubleReaction->m_isDraw = false;
				m_happyReaction->m_isDraw = true;
				UpdateAnimation();
				PositionUpdate();
				break;
			}
			case EnReactionType::None:
			{
				break;
			}
			}


			if (m_type != EnReactionType::None && m_isPlayAnimation)
			{
				m_timer += deltaTime;

				if (m_timer >= m_status->GetSwayTime())
				{
					m_timer = 0.0f;
					ResetIcon();
					m_type = EnReactionType::None;
				}
			}
		}


		void CPReactionMenu::ResetIcon()
		{
			m_speechBubble->m_isDraw = false;
			m_troubleReaction->m_isDraw = false;
			m_happyReaction->m_isDraw = false;


			m_troubleReaction->StopAnimation();
			m_happyReaction->StopAnimation();
			m_isPlayAnimation = false;
			m_timer = 0.0f;
			m_type = EnReactionType::None;
			m_target = nullptr;
			PositionUpdate();
		}


		void CPReactionMenu::SetAnimation()
		{
			std::function<void(UIIcon*)> attach =
				[&](UIIcon* icon)
				{
					const uint32_t swayAnimKey = animKey::CPREACTION_SWAY_ANIM_KEY;
					icon->RemoveAnimation(swayAnimKey);

					UIAnimationFactory::Attach<UIRotationAnimation>(icon, swayAnimKey);

					auto* anim = icon->FindAnimation(swayAnimKey);


					if (anim) anim->PlayAnimation();
				};

			attach(m_troubleReaction);
			attach(m_happyReaction);
		}


		void CPReactionMenu::UpdateAnimation()
		{
			if (m_type == EnReactionType::Happy)
			{
				m_troubleReaction->StopAnimation();
				m_happyReaction->UpdateAnimation();
			}
			else if (m_type == EnReactionType::Trouble)
			{
				m_troubleReaction->UpdateAnimation();
				m_happyReaction->StopAnimation();
			}


			m_isPlayAnimation = m_troubleReaction->IsPlayAnimation() || m_happyReaction->IsPlayAnimation();
		}
	}
}