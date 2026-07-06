/**
 * @file CPReactionSystem.cpp
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionSystem.h"

#include "CPReactionStatus.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ClumsyChildPenguinIState.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/NaughtyChildPenguinIState.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/NaughtyChildPenguinStateMachine.h"



namespace app
{
	namespace ui
	{
		CPReactionSystem::CPReactionSystem()
			: m_reactionPackets{}
			, m_targets{}
			, m_reactionStatusParent(nullptr)
		{}


		CPReactionSystem::~CPReactionSystem()
		{}


		void CPReactionSystem::Initialize()
		{
			m_reactionStatusParent = std::make_unique<CPReactionStatus>();
			m_reactionStatusParent->SetUp();

			for (int i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				auto& it = m_reactionPackets.at(i);
				it.Initialize("Assets/parameter/UI/cpReaction/CPReaction.json");

				it.GetMenu()->SetStatus(m_reactionStatusParent.get());

				m_targets.at(i) = nullptr;
			}
		}


		void CPReactionSystem::Update()
		{
			for (auto& packet : m_reactionPackets)
			{
				packet.Update();
			}

			UpdateReactionPositions();
		}


		void CPReactionSystem::Render(RenderContext& rc)
		{
			for (auto& packet : m_reactionPackets)
			{
				packet.Render(rc);
			}
		}


		void CPReactionSystem::SetTarget(actor::ChildPenguin* childPenguin, const EnReactionType type)
		{
			if (!childPenguin) return;

			const uint8_t index = SearchTargettableIndex();

			m_targets.at(index) = childPenguin;

			EnReactionType currentType = type;

			auto penguinType = childPenguin->GetChildPenguinType();


			// ステートマシンを取得する
			switch (penguinType)
			{
			case actor::EnChildPenguinType::Naughty:
				auto naughty = dynamic_cast<actor::NaughtyChildPenguinStateMachine*>(childPenguin->GetStateMachine());

				const bool isWhirlpool = naughty->GetIsGoingToWakeBear();
				const bool isWakeBear = naughty->GetIsGoingToWhirlpool();

				currentType = (isWhirlpool || isWakeBear) ? EnReactionType::Happy : type;
				break;
			}




			auto* menu = m_reactionPackets.at(index).GetMenu();
			menu->PlayUIAnimation(currentType, childPenguin->GetChildPenguinType());
		}


		uint8_t CPReactionSystem::SearchTargettableIndex() const
		{
			// リアクションしていないスロットを探す
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				if (m_reactionPackets.at(i).GetMenu()->GetReactionType() == EnReactionType::None) return i;
			}

			// 見つからなければ、先頭のスロットを上書きする
			return 0;
		}


		void CPReactionSystem::UpdateReactionPositions()
		{
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				auto* menu = m_reactionPackets.at(i).GetMenu();
				auto*& target = m_targets.at(i);

				// タイマー終了などでリアクションが終わっていたらターゲットを解放する
				if (menu->GetReactionType() == EnReactionType::None)
				{
					target = nullptr;
					continue;
				}

				if (!target) continue;

				// 前方判定は行わず、ターゲットが存在する限り常に描画する
				menu->SetIsDraw(true);

				const Vector3 targetPosition = target->GetTransform().m_position;

				Vector2 screenPosition = Vector2::Zero;
				CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(screenPosition, targetPosition);

				menu->SetTargetPosition(Vector3(screenPosition.x, screenPosition.y, 0.0f));
			}
		}
	}
}