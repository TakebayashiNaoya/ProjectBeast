/**
 * @file CPReactionMenu.cpp
 * @brief 子ペンギンのリアクションUIクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionMenu.h"

#include "CPReactionAnimStatus.h"
#include "CPReactionStatus.h"

#include "Source/UI/Animation/UIAnimationFactory.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"

#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		void CPReactionMenu::PlayUIAnimation(const EnReactionType type)
		{
			// サウンドマネージャーのインスタンスを取得
			auto& soundMng = SoundManager::Get();

			m_type = type;

			bool isDrawSpeechBubble = false;
			bool isDrawTroubleReaction = false;
			bool isDrawHappyReaction = false;
			Vector4 speechBubbleColor = Vector4::Black;

			enSoundKind kind = enSoundKind::enSoundKind_None;

			if (m_type == EnReactionType::Trouble)
			{
				isDrawSpeechBubble = true;
				isDrawTroubleReaction = true;
				isDrawHappyReaction = false;

				kind = enSoundKind::enSoundKind_CPReactionTrouble;
			}
			else if (m_type == EnReactionType::Happy)
			{
				isDrawSpeechBubble = true;
				isDrawTroubleReaction = false;
				isDrawHappyReaction = true;

				kind = enSoundKind::enSoundKind_CPReactionHappy;
			}

			auto cpType = m_target->GetChildPenguinType();
			switch (cpType)
			{
			case actor::EnChildPenguinType::Serious:
				speechBubbleColor = m_status->GetSeriousReactionColor();
				break;
			case actor::EnChildPenguinType::Clingy:
				speechBubbleColor = m_status->GetClingyReactionColor();
				break;
			case actor::EnChildPenguinType::Naughty:
				speechBubbleColor = m_status->GetNaughtyReactionColor();
				break;
			case actor::EnChildPenguinType::Clumsy:
				speechBubbleColor = m_status->GetClumsyReactionColor();
				break;
			case actor::EnChildPenguinType::Caring:
				speechBubbleColor = m_status->GetCaringReactionColor();
				break;
			default:
				break;
			}

			m_speechBubble->m_isDraw = isDrawSpeechBubble;
			m_troubleReaction->m_isDraw = isDrawTroubleReaction;
			m_happyReaction->m_isDraw = isDrawHappyReaction;

			m_speechBubble->m_color = speechBubbleColor;

			soundMng.PlaySE(kind);

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
			, m_isPlayAnimation(false)
			, m_status(nullptr)
			, m_animStatus(nullptr)
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
			m_animStatus.reset();
			m_animStatus = std::make_unique<CPReactionAnimStatus>();

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
			CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(screenPos, targetPosition);

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


			const float deltaTime = g_gameTime->GetFrameDeltaTime();

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

			const auto key = animKey::CPREACTION_SWAY_ANIM_KEY;

			m_troubleReaction->StopAnimation();
			m_troubleReaction->RemoveAnimation(key);
			m_happyReaction->StopAnimation();
			m_happyReaction->RemoveAnimation(key);
			m_isPlayAnimation = false;
			m_timer = 0.0f;
			m_target = nullptr;
		}


		void CPReactionMenu::SetAnimation()
		{
			std::function<void(UIIcon*)> attach =
				[&](UIIcon* icon)
				{
					icon->RemoveAnimation(animKey::CPREACTION_SWAY_ANIM_KEY);

					UIAnimationFactory::Attach<UIRotationAnimation>(icon, animKey::CPREACTION_SWAY_ANIM_KEY);

					auto* anim = icon->FindAnimation(animKey::CPREACTION_SWAY_ANIM_KEY);


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
			}
			else if (m_type == EnReactionType::Trouble)
			{
				m_happyReaction->StopAnimation();
			}


			m_isPlayAnimation = m_troubleReaction->IsPlayAnimation() || m_happyReaction->IsPlayAnimation();
		}
	}
}