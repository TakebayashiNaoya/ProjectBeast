/**
 * @file WpWarningSystem.cpp
 * @brief WpWarningのシステムクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "WpWarningSystem.h"

#include "WpWarningMenu.h"
#include "WpWarningStatus.h"

#include "Source/UI/Modules/FrontChecker/FrontChecker.h"


namespace app
{
	namespace ui
	{
		/** 描画可能な距離 */
		constexpr float DRAWABLE_LENGTH = 500.0f;


		WpWarningSystem::WpWarningSystem()
			: m_packets()
			, m_parentStatus(nullptr)
			, m_daddyTransform()
			, m_whirlpoolPositions{}
		{}


		WpWarningSystem::~WpWarningSystem()
		{}


		void WpWarningSystem::Initialize()
		{
			// ステータスを生成,初期化
			m_parentStatus = std::make_unique<WpWarningStatus>();
			m_parentStatus->SetUp();

			// パケットを初期化
			for (auto& packet : m_packets)
			{
				// packetの中身を生成、初期化
				InitUIPacket(packet, "Assets/parameter/UI/wpWarning/WpWarning.json");
				packet->GetMenu()->SetStatus(m_parentStatus.get());
			}
		}


		void WpWarningSystem::Update()
		{
			for (auto& packet : m_packets)
			{
				packet->Update();
			}
		}


		void WpWarningSystem::Render(RenderContext& rc)
		{
			for (auto& packet : m_packets)
			{
				packet->Render(rc);
			}
		}


		void WpWarningSystem::UpdateDrawFlags()
		{
			// 親ペンギンの位置を取得
			const Vector3 dpPosition = Vector3(
				m_daddyTransform.m_position.x,
				0.0f,
				m_daddyTransform.m_position.z
			);


			// 描画距離の2乗と比較して、描画フラグを更新する
			constexpr float drawableLengthSq = DRAWABLE_LENGTH * DRAWABLE_LENGTH;

			auto& posses = m_whirlpoolPositions;

			// 規定距離よりも長いものを除外する
			m_whirlpoolPositions.erase(
				std::remove_if(posses.begin(), posses.end(),
					[drawableLengthSq, dpPosition](const Vector3& position)
					{
						const float diffSq = (dpPosition - position).LengthSq();
						return diffSq > drawableLengthSq;
					}
				),
				posses.end()
			);
			// 上位3つ手前にソート
			const uint8_t size = std::min<uint8_t>(PACKET_NUM, posses.size());
			std::nth_element(posses.begin(), posses.begin() + size, posses.end(),
				[dpPosition](const Vector3& a, const Vector3& b)
				{
					const float diffSqA = (dpPosition - a).LengthSq();
					const float diffSqB = (dpPosition - b).LengthSq();
					return diffSqA < diffSqB;
				}
			);

			for (uint8_t i = 0; i < PACKET_NUM; ++i)
			{
				auto* menu = m_packets.at(i)->GetMenu();

				if (i >= posses.size())
				{
					menu->SetTargetPosition(Vector3::Zero);
					menu->SetIsDraw(false);
					continue;
				}

				// 3d座標を2d座標に変換して、UIの位置を更新する
				Vector2 screenPosition = Vector2::Zero;
				CameraSystem::Get().GetMainCamera().CalcScreenPositionFromWorldPosition(screenPosition, posses.at(i));

				const Vector3 uiPosition = Vector3(
					screenPosition.x,
					screenPosition.y + m_parentStatus->GetIconOffsetY(),
					0.0f
				);
				menu->SetTargetPosition(uiPosition);

				const bool isDraw = FrontChecker::IsInFront(
					m_daddyTransform.m_position,
					m_daddyTransform.m_rotation,
					posses.at(i)
				);


				menu->SetIsDraw(isDraw);
			}
		}
	}
}