/**
 * @file CPReactionSystem.cpp
 * @brief 子ペンギンのリアクションシステムクラス
 */
#include "stdafx.h"
#include "CPReactionSystem.h"

#include "CPReactionStatus.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinManager.h"



namespace app
{
	namespace ui
	{
		namespace
		{
			/** シロクマの？/！アイコンの基準位置へ足すワールドYオフセット（背丈ぶん持ち上げる） */
			constexpr float ENEMY_MARK_WORLD_OFFSET_Y = 150.0f;

			/** シロクマの吹き出しの色（子ペンギンのタイプ色と混ざらないグレー） */
			const Vector4 ENEMY_BUBBLE_COLOR(0.35f, 0.35f, 0.35f, 1.0f);
		}


		CPReactionSystem::CPReactionSystem()
			: m_reactionPackets{}
			, m_targets{}
			, m_targetWorldOffsetsY{}
			, m_markPackets{}
			, m_markTargets{}
			, m_markWorldOffsetsY{}
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
				m_targetWorldOffsetsY.at(i) = 0.0f;
				m_priorities.at(i) = EnCPReactionPriority::Normal;
			}

			// 頭上マーク（？/！）用のパケットも同じレイアウトで用意する
			for (int i = 0; i < MARK_PACKET_NUM; ++i)
			{
				auto& it = m_markPackets.at(i);
				it.Initialize("Assets/parameter/UI/cpReaction/CPReaction.json");

				it.GetMenu()->SetStatus(m_reactionStatusParent.get());

				m_markTargets.at(i) = nullptr;
				m_markWorldOffsetsY.at(i) = 0.0f;
			}
		}


		void CPReactionSystem::Update()
		{
			for (auto& packet : m_reactionPackets)
			{
				packet.Update();
			}
			for (auto& packet : m_markPackets)
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
			for (auto& packet : m_markPackets)
			{
				packet.Render(rc);
			}
		}


		uint8_t CPReactionSystem::AcquireSlot(actor::Actor* target, const EnCPReactionPriority priority)
		{
			// すでにこのアクターがスロットを使用中であればそれを使い回す。
			// （BattleManager経由の通知は毎フレーム呼ばれる可能性があるため、
			//   呼ばれるたびに新規スロットを消費してしまうのを防ぐ）
			uint8_t index = SearchExistingIndex(target);

			if (index == REACTION_PACKET_NUM)
			{
				index = SearchTargettableIndex();
				m_targets.at(index) = target;
			}
			else
			{
				// 既に表示中のリアクションより優先度が低い通知は無視する。
				// これにより、呼び出し順序に関わらず結果が一定になる
				// （優先度が同じ場合は、単純に後から呼ばれた方が反映される）
				if (static_cast<uint8_t>(priority) < static_cast<uint8_t>(m_priorities.at(index)))
				{
					return REACTION_PACKET_NUM;
				}
			}

			m_priorities.at(index) = priority;
			return index;
		}


		void CPReactionSystem::SetTarget(actor::ChildPenguin* childPenguin, const EnCPReactionType type, const EnCPReactionPriority priority)
		{
			if (!childPenguin) return;

			// Noneの通知は「頭上マークを消せ」の意味（親を見つけた瞬間の？の消去に使う）
			if (type == EnCPReactionType::None)
			{
				ClearMark(childPenguin);
				return;
			}

			// ？/！は吹き出しとは別枠の頭上マークとして出す。
			// 同じ枠を使うと、「！」の直後に来る入隊の吹き出し（Happy）が
			// 優先度調停で棄却されてしまう
			if (type == EnCPReactionType::Question || type == EnCPReactionType::Exclamation)
			{
				SetMarkTarget(childPenguin, type, 0.0f);
				return;
			}

			const uint8_t index = AcquireSlot(childPenguin, priority);
			if (index == REACTION_PACKET_NUM) return;

			m_targetWorldOffsetsY.at(index) = 0.0f;

			auto* menu = m_reactionPackets.at(index).GetMenu();

			// 同じタイプが継続している間はアニメーションを再スタートしない
			if (menu->GetReactionType() == type) return;

			// typeの確定（対象の内部状態による上書き等）は呼び出し側の責務。
			// Systemはタイプ別の分岐を持たず、受け取った値をそのまま反映する。
			menu->PlayUIAnimation(type, childPenguin->GetChildPenguinType());
		}


		void CPReactionSystem::SetEnemyTarget(actor::Enemy* enemy, const EnCPReactionType type)
		{
			if (!enemy) return;

			SetMarkTarget(enemy, type, ENEMY_MARK_WORLD_OFFSET_Y);
		}


		void CPReactionSystem::SetMarkTarget(actor::Actor* target, const EnCPReactionType type, const float worldOffsetY)
		{
			// 同じアクターが使用中のスロットがあれば使い回す
			uint8_t index = MARK_PACKET_NUM;
			for (uint8_t i = 0; i < MARK_PACKET_NUM; ++i)
			{
				if (m_markTargets.at(i) == target)
				{
					index = i;
					break;
				}
			}

			// 無ければ空きスロットを探す。全部使用中なら先頭を上書きする
			if (index == MARK_PACKET_NUM)
			{
				index = 0;
				for (uint8_t i = 0; i < MARK_PACKET_NUM; ++i)
				{
					if (m_markPackets.at(i).GetMenu()->GetReactionType() == EnCPReactionType::None)
					{
						index = i;
						break;
					}
				}
				m_markTargets.at(index) = target;
			}

			m_markWorldOffsetsY.at(index) = worldOffsetY;

			auto* menu = m_markPackets.at(index).GetMenu();

			// 同じタイプが継続している間は再スタートしない
			if (menu->GetReactionType() == type) return;

			// マークは吹き出しを出さないので色は使われないが、Menuの共通経路に合わせて渡す
			menu->PlayUIAnimationWithColor(type, ENEMY_BUBBLE_COLOR);
		}


		void CPReactionSystem::ClearMark(actor::Actor* target)
		{
			if (!target) return;

			for (uint8_t i = 0; i < MARK_PACKET_NUM; ++i)
			{
				if (m_markTargets.at(i) != target) continue;

				m_markPackets.at(i).GetMenu()->ForceFinish();
				m_markTargets.at(i) = nullptr;
				m_markWorldOffsetsY.at(i) = 0.0f;
				return;
			}
		}


		void CPReactionSystem::NotifyTargetDestroyed(actor::Actor* target)
		{
			if (!target) return;

			// 吹き出しとマークは別枠なので、同じアクターが両方に載っていることがある。
			// 片方だけ消すと残った方が解放済みの座標を読み続けるため、両方を走査する
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				if (m_targets.at(i) != target) continue;

				m_reactionPackets.at(i).GetMenu()->ForceFinish();
				m_targets.at(i) = nullptr;
				m_targetWorldOffsetsY.at(i) = 0.0f;
				m_priorities.at(i) = EnCPReactionPriority::Normal;
			}

			for (uint8_t i = 0; i < MARK_PACKET_NUM; ++i)
			{
				if (m_markTargets.at(i) != target) continue;

				m_markPackets.at(i).GetMenu()->ForceFinish();
				m_markTargets.at(i) = nullptr;
				m_markWorldOffsetsY.at(i) = 0.0f;
			}
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


		uint8_t CPReactionSystem::SearchExistingIndex(const actor::Actor* target) const
		{
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				if (m_targets.at(i) == target) return i;
			}

			return REACTION_PACKET_NUM;
		}


		void CPReactionSystem::UpdateReactionPositions()
		{
			// 頭上マーク（？/！）の座標更新
			for (uint8_t i = 0; i < MARK_PACKET_NUM; ++i)
			{
				auto* menu = m_markPackets.at(i).GetMenu();
				auto*& target = m_markTargets.at(i);

				// タイマー終了でマークが消えていたらターゲットを解放する
				if (menu->GetReactionType() == EnCPReactionType::None)
				{
					target = nullptr;
					m_markWorldOffsetsY.at(i) = 0.0f;
					continue;
				}

				if (!target) continue;

				Vector3 markPosition = target->GetTransform().m_position;
				markPosition.y += m_markWorldOffsetsY.at(i);

				const Vector3 daddyPos = actor::ChildPenguinManager::GetInstance()->GetDaddyPosition();
				const float markDrawableDistance = m_reactionStatusParent->GetDrawableDistance();
				if ((markPosition - daddyPos).LengthSq() > markDrawableDistance * markDrawableDistance)
				{
					menu->SetIsDraw(false);
					continue;
				}
				menu->SetIsDraw(true);

				Vector2 markScreenPos = Vector2::Zero;
				CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(markScreenPos, markPosition);
				menu->SetTargetPosition(Vector3(markScreenPos.x, markScreenPos.y, 0.0f));
			}

			// 吹き出しの座標更新
			for (uint8_t i = 0; i < REACTION_PACKET_NUM; ++i)
			{
				auto* menu = m_reactionPackets.at(i).GetMenu();
				auto*& target = m_targets.at(i);

				// タイマー終了などでリアクションが終わっていたらターゲットを解放する
				if (menu->GetReactionType() == EnCPReactionType::None)
				{
					target = nullptr;
					m_targetWorldOffsetsY.at(i) = 0.0f;
					m_priorities.at(i) = EnCPReactionPriority::Normal;
					continue;
				}

				if (!target) continue;

				Vector3 targetPosition = target->GetTransform().m_position;
				targetPosition.y += m_targetWorldOffsetsY.at(i);

				// プレイヤーから一定距離より離れている場合は吹き出しを表示しない
				const Vector3 daddyPosition = actor::ChildPenguinManager::GetInstance()->GetDaddyPosition();
				const float drawableDistance = m_reactionStatusParent->GetDrawableDistance();
				const float distanceSq = (targetPosition - daddyPosition).LengthSq();
				if (distanceSq > drawableDistance * drawableDistance)
				{
					menu->SetIsDraw(false);
					continue;
				}

				// 前方判定は行わず、規定距離内であれば常に描画する
				menu->SetIsDraw(true);

				Vector2 screenPosition = Vector2::Zero;
				CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(screenPosition, targetPosition);

				menu->SetTargetPosition(Vector3(screenPosition.x, screenPosition.y, 0.0f));
			}
		}
	}
}
