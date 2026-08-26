/**
 * @file BearReactionSystem.cpp
 * @brief クマのリアクションシステムクラス
 */
#include "stdafx.h"
#include "BearReactionSystem.h"

#include "BearReactionMenu.h"
#include "MasterBearReactionParameter.h"

#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace ui
	{
		void BearReactionSystem::SetReaction(
			const uint8_t index,
			const Vector3& position,
			const core::Transform& daddyTRS,
			const EnBearReactionType type
		)
		{
			K2_ASSERT(index < m_reactions.size(), "indexが範囲外です");
			auto& it = m_reactions.at(index);

			// スクリーン座標に変換
			Vector2 screenPosition = Vector2::Zero;
			CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(screenPosition, position);

			// パラメーターを取得
			const auto* p = core::ParameterManager::Get()->GetParameter<MasterBearReactionParameter>();

			// 有効距離かどうか
			const bool enableLength = (position - daddyTRS.m_position).Length() < p->activeDistance;
			// リアクションタイプが有効かどうか
			const bool isReaction = (type != EnBearReactionType::None);

			const bool isFront = FrontChecker::IsInFront(daddyTRS.m_position, daddyTRS.m_rotation, position);

			it->isActive = enableLength && isReaction && isFront;

			// UIのターゲット座標を設定
			it->packet->GetMenu()->SetTargetPosition(Vector3(
				screenPosition.x + p->offsetX,
				screenPosition.y + p->offsetY,
				0.0f
			));

			it->packet->GetMenu()->SetReactionType(type);
		}


		void BearReactionSystem::Initialize()
		{
			// パラメーターを生成
			core::ParameterManager::Get()->LoadParameter<MasterBearReactionParameter>
				("assets/parameter/UI/bearReaction/BearReactionParameter.json"
					, [](const nlohmann::json& json, MasterBearReactionParameter& p)
					{
						using JS = util::JsonConverter;

						p.offsetX = JS::ToFloat(json, "offsetX");
						p.offsetY = JS::ToFloat(json, "offsetY");
						p.activeDistance = JS::ToFloat(json, "activeDistance");
					}
				);


			K2_ASSERT(m_reactionNum > 0, "サイズ未設定");

			m_reactions.reserve(m_reactionNum);

			for (uint8_t i = 0; i < m_reactionNum; ++i)
			{
				auto newInfo = std::make_unique<ReactionInfo>();
				InitUIPacket(newInfo->packet, "assets/parameter/UI/bearReaction/BearReaction.json");
				newInfo->isActive = false;
				m_reactions.push_back(std::move(newInfo));
			}
		}


		void BearReactionSystem::Update()
		{
			for (const auto& it : m_reactions)
			{
				it->packet->Update();
			}
		}


		void BearReactionSystem::Render(RenderContext& rc)
		{
			for (const auto& it : m_reactions)
			{
				if (it->isActive)
					it->packet->Render(rc);
			}
		}


		BearReactionSystem::BearReactionSystem()
			: m_reactionNum(0)
		{}


		BearReactionSystem::~BearReactionSystem()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterBearReactionParameter>();
		}




		/**********************************************************/


		BearReactionSystem::ReactionInfo::ReactionInfo()
			: packet(nullptr)
			, isActive(false)
		{}


		BearReactionSystem::ReactionInfo::~ReactionInfo()
		{}
	}
}