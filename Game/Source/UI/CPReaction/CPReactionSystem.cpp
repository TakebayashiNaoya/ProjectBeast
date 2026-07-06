/**
 * @file CPReactionSystem.cpp
 * @brief 子ペンギンのリアクションシステムクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CPReactionSystem.h"

#include "CPReactionStatus.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"



namespace app
{
	namespace ui
	{
		CPReactionSystem::CPReactionSystem()
			: m_reactionPackets{}
			, m_targets{}
			, m_priorities{}
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
				m_priorities.at(i) = EnCPReactionPriority::Normal;
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


		void CPReactionSystem::SetTarget(actor::ChildPenguin* childPenguin, const EnCPReactionType type, const EnCPReactionPriority priority)
		{
			if (!childPenguin) return;

			// すでにこの子ペンギンがスロットを使用中であればそれを使い回す。
			// （BattleManager経由の通知は毎フレーム呼ばれる可能性があるため、
			//   呼ばれるたびに新規スロットを消費してしまうのを防ぐ）
			uint8_t index = SearchExistingIndex(childPenguin);

			if (index == REACTION_PACKET_NUM)
			{
				index = SearchTargettableIndex();
				m_targets.at(index) = childPenguin;
			}
			else
			{
				// 既に表示中のリアクションより優先度が低い通知は無視する。
				// これにより、呼び出し順序に関わらず結果が一定になる
				// （優先度が同じ場合は、単純に後から呼ばれた方が反映される）
				if (static_cast<uint8_t>(priority) < static_cast<uint8_t>(m_priorities.at(index)))
				{
					return;
				}
			}

			m_priorities.at(index) = priority;

			auto* menu = m_reactionPackets.at(index).GetMenu();

			// 同じタイプが継続している間はアニメーションを再スタートしない
			if (menu->GetReactionType() == type) return;

			// typeの確定（対象の内部状態による上書き等）は呼び出し側の責務。
			// Systemはタイプ別の分岐を持たず、受け取った値をそのまま反映する。
			menu->PlayUIAnimation(type, childPenguin->GetChildPenguinType());
		}


		uint8_t CPReactionSystem::SearchTargettableIndex() const
		{
			// リアクションしていないスロットを探す
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				if (m_reactionPackets.at(i).GetMenu()->GetReactionType() == EnCPReactionType::None) return i;
			}

			// 見つからなければ、先頭のスロットを上書きする
			return 0;
		}


		uint8_t CPReactionSystem::SearchExistingIndex(const actor::ChildPenguin* childPenguin) const
		{
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				if (m_targets.at(i) == childPenguin) return i;
			}

			return REACTION_PACKET_NUM;
		}


		void CPReactionSystem::UpdateReactionPositions()
		{
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				auto* menu = m_reactionPackets.at(i).GetMenu();
				auto*& target = m_targets.at(i);

				// タイマー終了などでリアクションが終わっていたらターゲットを解放する
				if (menu->GetReactionType() == EnCPReactionType::None)
				{
					target = nullptr;
					m_priorities.at(i) = EnCPReactionPriority::Normal;
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
