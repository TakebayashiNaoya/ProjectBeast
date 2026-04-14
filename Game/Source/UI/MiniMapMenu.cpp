/**
 * @file MiniMapMenu.cpp
 * @brief ミニマップの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "MiniMapMenu.h"
#include "Source/Actor/Character/penguin/ChildPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// シロクマのアイコンキー。
			constexpr uint32_t POLAR_BEAR_ICON_KEYS[] =
			{
					{ Hash32("bearIcon0")}
				,	{ Hash32("bearIcon1")}
				,	{ Hash32("bearIcon2")}
			};


			// 文字列型のアイコンキー。
			using IconKey = std::string;

			// ミニマップに表示するアイコンの構造体。
			struct MiniMapInfo
			{
				IconKey key;
				uint8_t number;
			};

			/**
			 * @brief 子ペンギンのアイコンの種類。
			 */
			enum class EnChildPenType : uint8_t
			{
				Blue = 0, // まじめ
				Orange,   // おっちょこちょい
				Pink,	  // 甘えん坊
				Yellow,	  // やんちゃ
				Green,    // 世話焼き
				Max,
				None = Max
			};

			// 子ペンギンのアイコンの数。
			constexpr uint8_t CHILD_PEN_TYPE_ICON = 20;
			// 子ペンギンのアイコンの種類の数。
			constexpr uint8_t MINIMAP_ICON_SIZE = static_cast<uint8_t>(EnChildPenType::Max);

			MiniMapInfo MINIMAP_ICON_KEYS[MINIMAP_ICON_SIZE] =
			{
					{ "childBlueIcon",   0 }  // 0: Blue   (まじめ)
				,	{ "childOrangeIcon", 0 }  // 1: Orange (おっちょこちょい)
				,	{ "childPinkIcon",   0 }  // 2: Pink   (甘えん坊)
				,	{ "childYellowIcon", 0 }  // 3: Yellow (やんちゃ)
				,	{ "childGreenIcon",  0 }  // 4: Green  (世話焼き)
			};

			const Vector3 MAP_CENTER_POS = Vector3(-500.0f, 0.0f, 0.0f);
			constexpr float MAP_RADIUS = 200.0f;
			constexpr float MAP_LIMITE_DISTANCE = 400.0f;
		}


		MiniMapMenu::MiniMapMenu()
			: m_isDraw(false)
			, m_daddyPenguin(nullptr)
		{
			// 子ペンギンとエネミーのマネージャーのインスタンスをコンストラクタで取得しておく。
			app::actor::ChildPenguinManager::GetInstance();
			app::actor::EnemyManager::GetInstance();
		}


		MiniMapMenu::~MiniMapMenu()
		{}


		void MiniMapMenu::Update()
		{
			// ミニマップのアイコンの描画。
			auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
			if (miniMapIcon) miniMapIcon->m_isDraw = m_isDraw;

			auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
			if (daddyIcon) daddyIcon->m_isDraw = m_isDraw;

			auto* blueIcon = GetUI<UIIcon>(Hash32("childBlueIcon"));
			if (blueIcon) blueIcon->m_isDraw = m_isDraw;

			auto* orangeIcon = GetUI<UIIcon>(Hash32("childOrangeIcon"));
			if (orangeIcon) orangeIcon->m_isDraw = m_isDraw;

			auto* pinkIcon = GetUI<UIIcon>(Hash32("childPinkIcon"));
			if (pinkIcon) pinkIcon->m_isDraw = m_isDraw;

			auto* yellowIcon = GetUI<UIIcon>(Hash32("childYellowIcon"));
			if (yellowIcon) yellowIcon->m_isDraw = m_isDraw;

			auto* greenIcon = GetUI<UIIcon>(Hash32("childGreenIcon"));
			if (greenIcon) greenIcon->m_isDraw = m_isDraw;

			auto* bearIcon = GetUI<UIIcon>(Hash32("bearIcon"));
			if (bearIcon)bearIcon->m_isDraw = m_isDraw;

			// 親ペンギンのが存在する時に、親ペンギン、子ペンギン、シロクマのアイコンをマップに表示する。
			if (m_daddyPenguin)
			{
				MapDaddyPen();
				MapChildPen();
				MapPolarBear();
			}


			MenuBase::Update();
		}


		bool MiniMapMenu::worldPosConverterToMapPos(Vector3 worldCenterPos, Vector3 worldPos, Vector3& mapPos)
		{
			worldCenterPos.y = 0.0f;
			worldPos.y = 0.0f;

			Vector3 diff = worldPos - worldCenterPos;

			if (diff.LengthSq() >= MAP_LIMITE_DISTANCE * MAP_LIMITE_DISTANCE)
			{
				return false;
			}

			float length = diff.Length();

			Vector3 forward = g_camera3D->GetForward();
			Quaternion rot;
			rot.SetRotationY(atan2(-forward.x, forward.z));
			rot.Apply(diff);

			diff.Normalize();
			diff *= length * MAP_RADIUS / MAP_LIMITE_DISTANCE;

			mapPos = Vector3(MAP_CENTER_POS.x + diff.x, MAP_CENTER_POS.y + diff.z, 0.0f);
			return true;
		}


		void MiniMapMenu::MapChildPen()
		{
			auto childPenMgr = app::actor::ChildPenguinManager::GetInstance()->GetChildPenguin();

			// アイコンを常に0から19までを順番に使用するために。
			for (auto& info : MINIMAP_ICON_KEYS)
			{
				// 全アイコンを一旦、描画のフラグをfalseに
				for (uint8_t i = 0; i < CHILD_PEN_TYPE_ICON; ++i)
				{
					IconKey key = info.key + std::to_string(i);
					UIIcon* icon = GetUI<UIIcon>(Hash32(key.c_str()));
					if (icon)icon->m_isDraw = false;
				}
				// アイコンの数を0にして、indexの値がおかしくならないようにさせる。
				info.number = 0;
			}

			// アイコンに適用させるisDraw。
			bool iconIsDraw = m_isDraw;


			for (auto* child : childPenMgr)
			{
				if (!child) continue;

				Vector3 daddyPos = m_daddyPenguin->GetTransform().m_position;
				Vector3 childPos = child->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;

				if (worldPosConverterToMapPos(daddyPos, childPos, mapPos))
				{
					// アイコンの取得用にnullptrで初期化。
					UIIcon* childIcon = nullptr;

					// 子ペンギンのタイプに対応したアイコンを取得。
					auto type = child->GetChildPenguinType();

					switch (type)
					{
					// タイプごとのアイコンは20個までとする。
					case app::actor::EnChildPenguinType::Serious:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Blue)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}
					case app::actor::EnChildPenguinType::Clingy:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Pink)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}
					case app::actor::EnChildPenguinType::Naughty:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Yellow)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
 						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						iconIsDraw = false;
						break;
					}
					case app::actor::EnChildPenguinType::Clumsy:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Orange)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}
					case app::actor::EnChildPenguinType::Caring:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Green)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}
					default:
					{
						K2_ASSERT(false, "子ペンギンのタイプが不正です。");
						break;
					}
					};

					if (childIcon == nullptr) continue;

					childIcon->m_isDraw = m_isDraw;
					childIcon->m_transform.m_localTransform.m_position = mapPos;
				}
			}
		}


		void MiniMapMenu::MapDaddyPen()
		{
			// 親ペンギンのアイコン。
			auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
			if (daddyIcon)
			{
				daddyIcon->m_isDraw = m_isDraw;
				// マップの中心に親ペンギンのアイコンを表示(固定表示)。
				daddyIcon->m_transform.m_localTransform.m_position = MAP_CENTER_POS;
			}
		}


		void MiniMapMenu::MapPolarBear()
		{
			// エネミーのリストを取得。
			const auto& enemis = app::actor::EnemyManager::GetInstance()->GetEnemies();				
			// シロクマのアイコンのインデックス(要素数)。
			int bearIconIndex = 0;

			// エネミーのリスト分だけループさせる。
			for (auto* enemy : enemis)
			{
				// シロクマ以外は描画させない。
				if (!enemy)continue;
				// シロクマのアイコン数を超えたら描画しない。
				if (bearIconIndex >= static_cast<int>(std::size(POLAR_BEAR_ICON_KEYS)))break;
				
				// アイコンの描画フラグ。
				bool isBearDraw = m_isDraw;

				// シロクマのアイコンを取得。
				auto* bearIcon = GetUI<UIIcon>(POLAR_BEAR_ICON_KEYS[bearIconIndex]);
				
				if (!bearIcon)
				{
					bearIconIndex++;
					continue;
				}

				// 親ペンギンの座標とシロクマの座標を取得して、マップ座標に変換する。
				Vector3 daddyPos = m_daddyPenguin->GetTransform().m_position;
				Vector3 enemyPos = enemy->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;

				// ワールド座標をマップ座標に変換出来たら、シロクマのアイコンをミニマップに表示する。
				if (worldPosConverterToMapPos(daddyPos, enemyPos, mapPos))
				{
					// 座標更新。
					bearIcon->m_transform.m_localTransform.m_position = mapPos;
					// ミニマップの範囲内は表示。
					bearIcon->m_isDraw = isBearDraw;
				}
				else
				{
					// ミニマップの範囲外は非表示。
					bearIcon->m_isDraw = false;
				}

				// シロクマのアイコンの要素数を超えないようにインデックスを増加させる。
				bearIconIndex++;
			}

			// 使われなかったアイコンを非表示にする。
        for (uint32_t i = bearIconIndex; i < static_cast<int>(std::size(POLAR_BEAR_ICON_KEYS)); i++)
			{
				auto* bearIcon = GetUI<UIIcon>(POLAR_BEAR_ICON_KEYS[i]);
				if (bearIcon)bearIcon->m_isDraw = false;
			}
		}


		void MiniMapMenu::InitializeLogic()
		{
			// 初期は非表示。
			auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
			if (miniMapIcon) miniMapIcon->m_isDraw = !m_isDraw;

			auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
			if (daddyIcon) daddyIcon->m_isDraw = !m_isDraw;

			auto* blueIcon = GetUI<UIIcon>(Hash32("childBlueIcon"));
			if (blueIcon) blueIcon->m_isDraw = !m_isDraw;

			auto* orangeIcon = GetUI<UIIcon>(Hash32("childOrangeIcon"));
			if (orangeIcon) orangeIcon->m_isDraw = !m_isDraw;

			auto* pinkIcon = GetUI<UIIcon>(Hash32("childPinkIcon"));
			if (pinkIcon) pinkIcon->m_isDraw = !m_isDraw;

			auto* yellowIcon = GetUI<UIIcon>(Hash32("childYellowIcon"));
			if (yellowIcon) yellowIcon->m_isDraw = !m_isDraw;

			auto* greenIcon = GetUI<UIIcon>(Hash32("childGreenIcon"));
			if (greenIcon) greenIcon->m_isDraw = !m_isDraw;

			auto* polarBear = GetUI<UIIcon>(Hash32("PolarBearIcon"));
			if (polarBear)polarBear->m_isDraw = !m_isDraw;
		}
	}
}