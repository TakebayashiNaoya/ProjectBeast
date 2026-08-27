/**
 * @file CPReactionMenu.cpp
 * @brief 子ペンギンのリアクションUIクラス
 */
#include "stdafx.h"
#include "CPReactionMenu.h"

#include "CPReactionAnimStatus.h"
#include "CPReactionStatus.h"

#include "Source/UI/Animation/UIAnimationFactory.h"

#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		CPReactionMenu::CPReactionMenu()
			: m_status(nullptr)
			, m_animStatus(nullptr)
			, m_speechBubble(nullptr)
			, m_troubleReaction(nullptr)
			, m_happyReaction(nullptr)
			, m_questionReaction(nullptr)
			, m_exclamationReaction(nullptr)
			, m_type(EnCPReactionType::None)
			, m_timer(0.0f)
			, m_isPlayAnimation(false)
			, m_isDraw(false)
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
			m_questionReaction = GetUI<UIIcon>(Hash32("questionReaction"));
			m_exclamationReaction = GetUI<UIIcon>(Hash32("exclamationReaction"));

			// アイコンの描画フラグをリセット
			m_speechBubble->m_isDraw = false;
			m_troubleReaction->m_isDraw = false;
			m_happyReaction->m_isDraw = false;
			m_questionReaction->m_isDraw = false;
			m_exclamationReaction->m_isDraw = false;
		}


		void CPReactionMenu::SetTargetPosition(const Vector3& screenPosition)
		{
			const Vector3 basePosition = Vector3(
				screenPosition.x,
				screenPosition.y + m_status->GetIconOffsetY(),
				0.0f
			);

			m_speechBubble->m_transform.m_localTransform.m_position = basePosition + m_status->GetSpeechBubbleOffset();
			m_troubleReaction->m_transform.m_localTransform.m_position = basePosition + m_status->GetTroubleReactionOffset();
			m_happyReaction->m_transform.m_localTransform.m_position = basePosition + m_status->GetHappyReactionOffset();
			// ？と！は吹き出しに入れず、頭の真上に単独で出す（オフセットなしの基準位置）
			m_questionReaction->m_transform.m_localTransform.m_position = basePosition;
			m_exclamationReaction->m_transform.m_localTransform.m_position = basePosition;
		}


		void CPReactionMenu::SetIsDraw(const bool isDraw)
		{
			m_isDraw = isDraw;

			// リアクション中でなければ常に非表示
			if (m_type == EnCPReactionType::None)
			{
				m_speechBubble->m_isDraw = false;
				m_troubleReaction->m_isDraw = false;
				m_happyReaction->m_isDraw = false;
				m_questionReaction->m_isDraw = false;
				m_exclamationReaction->m_isDraw = false;
				return;
			}

			// 吹き出しは従来のリアクション（困り/喜び）だけに出す。？/！は頭上に単独表示
			m_speechBubble->m_isDraw = isDraw
				&& (m_type == EnCPReactionType::Trouble || m_type == EnCPReactionType::Happy);
			m_troubleReaction->m_isDraw = isDraw && (m_type == EnCPReactionType::Trouble);
			m_happyReaction->m_isDraw = isDraw && (m_type == EnCPReactionType::Happy);
			m_questionReaction->m_isDraw = isDraw && (m_type == EnCPReactionType::Question);
			m_exclamationReaction->m_isDraw = isDraw && (m_type == EnCPReactionType::Exclamation);
		}


		void CPReactionMenu::PlayUIAnimation(const EnCPReactionType type, const actor::EnChildPenguinType cpType)
		{
			auto& soundMng = SoundManager::Get();

			m_type = type;

			Vector4 speechBubbleColor = Vector4::Black;
			enSoundKind kind = enSoundKind::enSoundKind_None;

			// ？と！はSEを鳴らさない（察知のたびに鳴ると音が洪水になる）
			if (m_type == EnCPReactionType::Trouble)
			{
				kind = enSoundKind::enSoundKind_CPReactionTrouble;
			}
			else if (m_type == EnCPReactionType::Happy)
			{
				kind = enSoundKind::enSoundKind_CPReactionHappy;
			}

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

			m_speechBubble->m_color = speechBubbleColor;

			// ？/！はSEなし（kindがNoneのまま）。Noneを渡すと鳴らさずに戻る
			if (kind != enSoundKind::enSoundKind_None)
			{
				soundMng.PlaySE(kind, 0.8f);
			}

			SetAnimation();
			m_timer = 0.0f;
		}


		void CPReactionMenu::PlayUIAnimationWithColor(const EnCPReactionType type, const Vector4& bubbleColor)
		{
			m_type = type;
			m_speechBubble->m_color = bubbleColor;

			SetAnimation();
			m_timer = 0.0f;
		}


		void CPReactionMenu::ForceFinish()
		{
			if (m_type == EnCPReactionType::None) return;

			ResetIcon();
			m_type = EnCPReactionType::None;
		}


		void CPReactionMenu::DrawFlagUpdate()
		{
			if (m_type == EnCPReactionType::None) return;

			// ？/！はアニメーション（揺れ）を持たないため、タイマーだけで自動終了する
			if (m_type == EnCPReactionType::Question || m_type == EnCPReactionType::Exclamation)
			{
				m_timer += g_gameTime->GetFrameDeltaTime();
				if (m_timer >= m_status->GetSwayTime())
				{
					ResetIcon();
					m_type = EnCPReactionType::None;
				}
				return;
			}

			UpdateAnimation();

			const float deltaTime = g_gameTime->GetFrameDeltaTime();

			if (m_isPlayAnimation)
			{
				m_timer += deltaTime;

				if (m_timer >= m_status->GetSwayTime())
				{
					m_timer = 0.0f;
					ResetIcon();
					m_type = EnCPReactionType::None;
				}
			}
		}


		void CPReactionMenu::ResetIcon()
		{
			m_speechBubble->m_isDraw = false;
			m_troubleReaction->m_isDraw = false;
			m_happyReaction->m_isDraw = false;
			m_questionReaction->m_isDraw = false;
			m_exclamationReaction->m_isDraw = false;

			const auto key = animKey::CPREACTION_SWAY_ANIM_KEY;

			m_troubleReaction->StopAnimation();
			m_troubleReaction->RemoveAnimation(key);
			m_happyReaction->StopAnimation();
			m_happyReaction->RemoveAnimation(key);
			m_questionReaction->StopAnimation();
			m_questionReaction->RemoveAnimation(key);
			m_exclamationReaction->StopAnimation();
			m_exclamationReaction->RemoveAnimation(key);
			m_isPlayAnimation = false;
			m_timer = 0.0f;
			m_isDraw = false;
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

			// 揺れの演出は吹き出しのリアクションだけ。？/！は揺らさない
			attach(m_troubleReaction);
			attach(m_happyReaction);
		}


		void CPReactionMenu::UpdateAnimation()
		{
			// 表示中のタイプ以外のアイコンのアニメーションを止める
			if (m_type != EnCPReactionType::Trouble)     { m_troubleReaction->StopAnimation(); }
			if (m_type != EnCPReactionType::Happy)       { m_happyReaction->StopAnimation(); }
			if (m_type != EnCPReactionType::Question)    { m_questionReaction->StopAnimation(); }
			if (m_type != EnCPReactionType::Exclamation) { m_exclamationReaction->StopAnimation(); }

			m_isPlayAnimation =
				m_troubleReaction->IsPlayAnimation() ||
				m_happyReaction->IsPlayAnimation() ||
				m_questionReaction->IsPlayAnimation() ||
				m_exclamationReaction->IsPlayAnimation();
		}
	}
}
