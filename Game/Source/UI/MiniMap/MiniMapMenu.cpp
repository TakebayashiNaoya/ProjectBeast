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

				m_map->m_isDraw = true;
				m_frame->m_isDraw = true;
			}
			// アニメーション中の更新。
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}
			else
			{
				m_daddy->m_isDraw = true;
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

			const Vector3 position = m_miniMapStatus->GetMapCenterPos();

			m_map->m_transform.m_localTransform.m_position = position;
			m_frame->m_transform.m_localTransform.m_position = position;
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

			const std::string path = "Assets/spriteData/UI/Icon/MiniMap/";
			const std::string ext = ".DDS";

			for (uint8_t i = 0; i < static_cast<uint8_t>(EnMiniMapIconType::Num); ++i)
			{
				auto& it = m_iconVectors.at(i);


				for (uint8_t j = 0; j < it.num; ++j)
				{
					// ステータスから初期値を取得
					auto& info = m_miniMapStatus->GetIconInitializeInfos().at(i);

					const std::string fullPath = path + info.path + ext;
					const uint32_t key = Hash32((info.path + std::to_string(j)).c_str());


					// UIIconを生成、初期化
					canvas->CreateUI<UIIcon>(key);

					auto* icon = canvas->FindUI<UIIcon>(key);
					K2_ASSERT(icon, "登録失敗");
					icon->m_isDraw = false;

					icon->Initialize(fullPath.c_str(), info.width, info.height, position, scale, rotation, color);

					// アイコンを配列に追加
					it.icons.push_back(icon);
				}

				K2_ASSERT(it.icons.size() == it.num, "サイズ不一致");
			}

			// マップ上のアイコンをすべて生成し終わってから親ペンギンのアイコンを生成する

			const auto& daddyInfo = m_miniMapStatus->GetDaddyInfo();
			const std::string name = daddyInfo.path;
			canvas->CreateUI<UIIcon>(Hash32(name.c_str()));
			m_daddy = canvas->FindUI<UIIcon>(Hash32(name.c_str()));

			K2_ASSERT(m_daddy, "登録失敗");

			m_daddy->Initialize(
				(path + name + ext).c_str(),
				daddyInfo.width,
				daddyInfo.height,
				position, scale, rotation, color
			);


			m_daddy->m_transform.m_localTransform.m_position = m_miniMapStatus->GetMapCenterPos();
			m_daddy->m_isDraw = false;
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

				// 前フレームの表示をリセット
				for (auto* icon : iconVector.icons)
				{
					icon->m_isDraw = false;
				}

				const uint8_t count = static_cast<uint8_t>(
					min(actorPositions.at(i).size(), iconVector.icons.size())
					);

				for (uint8_t j = 0; j < count; ++j)
				{
					auto& icon = iconVector.icons.at(j);

					// マップ座標に変換する。
					bool canConvert = WorldPosConverterToMapPos(
						centerActorPosition,
						actorPositions.at(i).at(j),
						icon->m_transform.m_localTransform.m_position
					);

					// アニメーションが終了していない場合は描画しない。
					const bool isDraw = canConvert && m_startingAnimLogic.IsAnimationFinished();

					// マップ範囲内のみ表示する。
					icon->m_isDraw = isDraw;
				}
			}
		}




		/***************************************************/


		MiniMapMenu::MapIconInfo::MapIconInfo()
			: num(0)
			, isFirstCall(true)
		{}
	}
}