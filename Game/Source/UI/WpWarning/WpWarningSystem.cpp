/**
 * @file WpWarningSystem.cpp
 * @brief WpWarningのシステムクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "WpWarningSystem.h"

#include "WpWarningStatus.h"

#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			/** 描画可能な距離 */
			constexpr float DRAWABLE_LENGTH = 500.0f;
		}


		WpWarningSystem::WpWarningSystem()
			: m_daddyPenguin(nullptr)
			, m_parentStatus(nullptr)
			, m_wpInfos(0)
		{}


		WpWarningSystem::~WpWarningSystem()
		{}


		void WpWarningSystem::Initialize()
		{
			// ステータスを生成,初期化
			m_parentStatus = std::make_unique<WpWarningStatus>();
			m_parentStatus->SetUpUI();

			// パケットを初期化
			for (auto& packet : m_packets)
			{
				// packetの中身を生成、初期化
				packet.Initialize("Assets/parameter/UI/wpWarning/WpWarning.json");
				// packetのMenuにステータスをセット
				packet.GetMenu()->SetStatus(m_parentStatus.get());
			}
		}


		void WpWarningSystem::Update()
		{
			UpdateDrawFlags();

			for (auto& packet : m_packets)
			{
				packet.Update();
			}
		}


		void WpWarningSystem::Render(RenderContext& rc)
		{
			for (auto& packet : m_packets)
			{
				packet.Render(rc);
			}
		}


		void WpWarningSystem::UpdateDrawFlags()
		{
			// 渦潮マネージャーを取得
			auto* manager = nature::WhirlpoolManager::GetInstance();
			if (!manager) return;

			// 親ペンギンの位置を取得
			if (!m_daddyPenguin) return;
			const Vector3 dpPosition = Vector3(
				m_daddyPenguin->GetTransform().m_position.x,
				0.0f,
				m_daddyPenguin->GetTransform().m_position.z
			);

			// 描画距離の2乗と比較して、描画フラグを更新する
			constexpr float drawableLengthSq = DRAWABLE_LENGTH * DRAWABLE_LENGTH;


			// 規定距離よりも近い渦潮に対して距離とポインタを保存する
			m_wpInfos.clear();

			// 渦潮を走査
			// 規定の距離よりも近い渦潮を描画フラグ更新の対象とする
			manager->ForEach([&](nature::Whirlpool* info)
				{
					// 渦潮の位置を取得
					const Vector3 wpPosition = Vector3(
						info->GetTransform().m_position.x,
						0.0f,
						info->GetTransform().m_position.z
					);

					// 親ペンギンと渦潮の距離の2乗を計算
					const float diffSq = (dpPosition - wpPosition).LengthSq();


					if (diffSq <= drawableLengthSq)
					{
						m_wpInfos.push_back({ diffSq, info });
					}
				});

			// 範囲内に存在する渦潮の数が規定よりも多い場合、パケットの数に絞る
			const int dataNum = std::min<int>(m_wpInfos.size(), PACKET_NUM);

			// 距離の近いものを取得
			std::partial_sort(
				m_wpInfos.begin(),
				m_wpInfos.begin() + dataNum,
				m_wpInfos.end(),
				[](const WpInfo& a, const WpInfo& b)
				{
					return a.lengthSq < b.lengthSq;
				}
			);


			// 描画フラグを更新
			for (int i = 0; i < m_packets.size(); ++i)
			{
				// パケットからMenuを取得
				auto* menu = m_packets.at(i).GetMenu();
				if (!menu) continue;

				// 近くに渦潮が存在しない場合描画しない
				if (i < dataNum)
				{
					menu->SetIsDraw(true);
					menu->SetWhirlpool(m_wpInfos.at(i).wp);
				}
				else
				{
					menu->SetIsDraw(false);
					menu->SetWhirlpool(nullptr);
				}
			}
		}
	}
}