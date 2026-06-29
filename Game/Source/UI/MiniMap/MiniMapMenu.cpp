/**
 * @file MiniMapMenu.cpp
 * @brief ミニマップの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "MiniMapMenu.h"
#include "MiniMapStatus.h"

#define CHECK_ICON(icon) K2_ASSERT(icon, "UIがnullptr")


namespace app
{
	namespace ui
	{
		MiniMapMenu::MiniMapMenu()
			: m_daddy(nullptr)
			, m_map(nullptr)
			, m_frame(nullptr)
			, m_isDraw(false)
		{
			// ミニマップ専用ステータスを生成する。
			m_miniMapStatus = std::make_unique<MiniMapStatus>();
			m_miniMapStatus->SetUp();
		}


		MiniMapMenu::~MiniMapMenu()
		{}


		void MiniMapMenu::Update()
		{
			if (!m_startingAnimLogic.IsAnimationStarted())
			{
				m_startingAnimLogic.Initialize(
					this,
					{ "MiniMapIcon", "MiniMapFrameIcon", "DaddyIcon" },
					{},
					Vector3(-400.0f, 0.0f, 0.0f)
				);

				DrawMapIcons(true);
			}
			// アニメーション中の更新。
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}
			else
			{
				//m_isDraw = true;
				//UpdateDrawFlag();
			}






			// マップのフレームアイコンをカメラの向きに合わせて回転させる。
			MapFrameRotation();

			MenuBase::Update();
		}


		bool MiniMapMenu::WorldPosConverterToMapPos(Vector3 worldCenterPos, Vector3 worldPos, Vector3& mapPos)
		{
			worldCenterPos.y = 0.0f;
			worldPos.y = 0.0f;

			Vector3 diff = worldPos - worldCenterPos;
			const float diffLengthSq = diff.LengthSq();

			// 距離の上限を計算する。
			const float dis = m_miniMapStatus->GetLimitDistance();
			const float disSq = std::pow(dis, 2.0f);

			// 距離の上限を超えていたらマップに表示しない。
			if (diffLengthSq >= disSq)
			{
				return false;
			}

			float length = diff.Length();

			// カメラの向きに合わせてワールド座標の差分を回転させる。
			Vector3 forward = CameraSystem::Get().GetMainCamera().GetForward();
			Quaternion rot;
			rot.SetRotationY(atan2(-forward.x, forward.z));
			rot.Apply(diff);

			// マップの大きさ / 距離の限界値 の倍率でマップ座標に変換する。
			const float mapRadius = m_miniMapStatus->GetRadius();
			const float mapLimitDis = m_miniMapStatus->GetLimitDistance();
			diff *= mapRadius / mapLimitDis;

			// マップ中心座標 + 回転させた差分でマップ座標を算出する。
			Vector3 mapCenterPos = m_miniMapStatus->GetMapCenterPos();
			mapPos = Vector3(mapCenterPos.x + diff.x, mapCenterPos.y + diff.z, 0.0f);

			return true;
		}


		void MiniMapMenu::MapFrameRotation()
		{
			// カメラの向きに合わせてマップのフレームアイコンを回転させる。
			Vector3 forward = CameraSystem::Get().GetMainCamera().GetForward();
			const float angle = atan2(forward.x, forward.z);

			Quaternion qrot;
			// Z軸回転を適用。
			qrot.SetRotationZ(angle);
			// 各アイコンに回転を適用させる。
			m_frame->m_transform.m_localTransform.m_rotation = qrot;
		}


		void MiniMapMenu::UpdateDrawFlag()
		{
			DrawMapIcons(m_isDraw);

			for (auto& it : m_iconVectors)
			{
				for (auto* icon : it.icons)
				{
					SetDrawFlag(icon, m_isDraw);
				}
			}
		}


		void MiniMapMenu::SetDrawFlag(UIIcon* icon, const bool isDraw)
		{
			CHECK_ICON(icon);
			icon->m_isDraw = isDraw;
		}


		UIIcon* MiniMapMenu::GetAndInitIcon(const uint32_t key)
		{
			auto* icon = GetUI<UIIcon>(key);
			K2_ASSERT(icon, "登録失敗");
			icon->m_isDraw = false;
			return icon;
		}


		void MiniMapMenu::InitializeLogic()
		{
			// アイコンを取得、初期化
			m_map = GetAndInitIcon(Hash32("MiniMapIcon"));
			m_frame = GetAndInitIcon(Hash32("MiniMapFrameIcon"));
			m_daddy = GetAndInitIcon(Hash32("DaddyIcon"));

			const Vector3 position = m_miniMapStatus->GetMapCenterPos();

			m_map->m_transform.m_localTransform.m_position = position;
			m_frame->m_transform.m_localTransform.m_position = position;
			m_daddy->m_transform.m_localTransform.m_position = position;
		}


		void MiniMapMenu::InitializeMapIcon()
		{
			// ステータスから初期値を取得
			auto position = m_miniMapStatus->GetInitPosition();
			auto scale = m_miniMapStatus->GetInitScale();
			auto rotation = m_miniMapStatus->GetInitRotation();
			auto color = m_miniMapStatus->GetInitColor();

			auto* canvas = GetCanvas();
			K2_ASSERT(canvas, "取得失敗");


			for (uint8_t i = 0; i < static_cast<uint8_t>(EnMiniMapIconType::Num); ++i)
			{
				auto& it = m_iconVectors.at(i);


				for (uint8_t j = 0; j < it.num; ++j)
				{
					// ステータスから初期値を取得
					auto& info = m_miniMapStatus->GetIconInitializeInfos().at(i);

					const std::string path = "Assets/spriteData/UI/Icon/MiniMap/" + info.path + ".DDS";
					const uint32_t key = Hash32((info.path + std::to_string(j)).c_str());


					// UIIconを生成、初期化
					canvas->CreateUI<UIIcon>(key);

					auto* icon = canvas->FindUI<UIIcon>(key);
					K2_ASSERT(icon, "登録失敗");
					icon->m_isDraw = false;

					icon->Initialize(path.c_str(), info.width, info.height, position, scale, rotation, color);

					// アイコンを配列に追加
					it.icons.push_back(icon);
				}

				K2_ASSERT(it.icons.size() == it.num, "サイズ不一致");
			}
		}


		void MiniMapMenu::SetIconNum(const EnMiniMapIconType type, const uint8_t num)
		{
			auto& it = m_iconVectors.at(static_cast<uint8_t>(type));

			K2_ASSERT(it.icons.size() == 0 || !it.isFirstCall, "初期化済み");

			it.num = num;
			it.icons.reserve(num);
			it.isFirstCall = false;
		}


		void MiniMapMenu::SetActorPositions(
			const Vector3& centerActorPosition,
			const ActorPositions& actorPositions
		)
		{
			for (uint8_t i = 0; i < static_cast<uint8_t>(EnMiniMapIconType::Num); ++i)
			{
				auto& iconVector = m_iconVectors.at(i);

				for (uint8_t j = 0; j < actorPositions.at(i).size(); ++j)
				{
					auto& icon = iconVector.icons.at(j);

					// マップ座標に変換する。
					bool canConvert = WorldPosConverterToMapPos(
						centerActorPosition,
						actorPositions.at(i).at(j),
						icon->m_transform.m_localTransform.m_position
					);

					// マップ座標に変換できたら表示する。
					icon->m_isDraw = true;
				}
			}
		}

		void MiniMapMenu::DrawMapIcons(const bool isDraw)
		{
			SetDrawFlag(m_daddy, isDraw);
			SetDrawFlag(m_map, isDraw);
			SetDrawFlag(m_frame, isDraw);
		}




		/***************************************************/


		MiniMapMenu::MapIconInfo::MapIconInfo()
			: num(0)
			, isFirstCall(true)
		{}
	}
}