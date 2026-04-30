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
#include "Source/Actor/Character/Enemy/Enemy.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// シロクマのアイコンの数。
			constexpr uint32_t BEAR_ICON_SIZE = 3;
			// シロクマのアイコンキー。
			constexpr std::array<uint32_t, BEAR_ICON_SIZE> POLAR_BEAR_ICON_KEYS =
			{
					Hash32("bearIcon0")
				,	Hash32("bearIcon1")
				,	Hash32("bearIcon2")
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
		}


		MiniMapMenu::MiniMapMenu()
			: m_isDraw(false)
			, m_daddyPenguin(nullptr)
		{
			// 子ペンギンとエネミーのマネージャーのインスタンスをコンストラクタで取得しておく。
			app::actor::ChildPenguinManager::GetInstance();
			app::actor::EnemyManager::GetInstance();
			
			// ミニマップ専用ステータスを生成する。
			m_miniMapStatus = std::make_unique<MiniMapStatus>();

			// ミニマップ専用のセットアップUIを呼び出す。
			m_miniMapStatus->SetUpUI();
		}


		MiniMapMenu::~MiniMapMenu()
		{}


		void MiniMapMenu::Update()
		{
			// ミニマップを表示しないときは、全てのアイコンを非表示にする。
			if (!m_isDraw)
			{
				// ミニマップのアイコンの描画。
				auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
				if (miniMapIcon) miniMapIcon->m_isDraw = false;

				auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
				if (daddyIcon) daddyIcon->m_isDraw = false;

				// 子ペンギンのアイコンを全て非表示にする。
				for (const auto& info : MINIMAP_ICON_KEYS)
				{
					// アイコンの数だけループして、全てのアイコンを非表示。
					for (uint8_t i = 0; i < CHILD_PEN_TYPE_ICON; ++i)
					{
						IconKey key = info.key + std::to_string(i);
						auto* childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						if (childIcon) childIcon->m_isDraw = false;
					}
				}

				// シロクマのアイコンを全て非表示にする。
				for (auto key : POLAR_BEAR_ICON_KEYS)
				{
					auto* bearIcon = GetUI<UIIcon>(key);
					if (bearIcon) bearIcon->m_isDraw = false;
				}

				MenuBase::Update();
				return;
			}

			// ミニマップのアイコンを表示する。
			auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
			if (miniMapIcon) miniMapIcon->m_isDraw = true;


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

			// あらかじめ距離の上限を計算しておく。
			const float dis = m_miniMapStatus->GetLimitDistance();
			const float disSq = std::pow(dis, 2.0f);

			if (diff.LengthSq() >= disSq)
			{
				return false;
			}

			float length = diff.Length();

			// カメラの向きに合わせてワールド座標の差分を回転させる。
			Vector3 forward = g_camera3D->GetForward();
			Quaternion rot;
			rot.SetRotationY(atan2(-forward.x, forward.z));
			// ベクトルの回転を適用。
			rot.Apply(diff);

			diff.Normalize();
			
			// マップの大きさと距離の限界値から、回転させた差分をマップ座標に変換するための倍率を計算する。
			const float mapRadius = m_miniMapStatus->GetRadius();
			const float mapLimitDis = m_miniMapStatus->GetLimitDistance();

			// マップの大きさ / 距離の限界値。
			diff *= length * mapRadius / mapLimitDis;

			// あらかじめマップの中心座標を取得して、回転させた差分を計算する。
			Vector3 mapCenterPos = m_miniMapStatus->GetMapCenterPos();
			mapCenterPos = Vector3(mapCenterPos.x + diff.x, mapCenterPos.y + diff.z, 0.0f);

			// マップの中心座標 + 回転させた差分。
			mapPos = mapCenterPos;
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
			//bool iconIsDraw = false;


			for (auto* child : childPenMgr)
			{
				// 子ペンギン以外は描画させない。
				if (!child) continue;

				// ワールド座標をマップ座標に変換する座標を用意。
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
					// まじめ。
					case app::actor::EnChildPenguinType::Serious:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Blue)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}

					// 甘えん坊。
					case app::actor::EnChildPenguinType::Clingy:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Pink)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}

					// やんちゃ。
					case app::actor::EnChildPenguinType::Naughty:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Yellow)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
 						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}

					// おっちょこちょい。
					case app::actor::EnChildPenguinType::Clumsy:
					{
						MiniMapInfo& info = MINIMAP_ICON_KEYS[static_cast<uint8_t>(EnChildPenType::Orange)];
						if (info.number >= CHILD_PEN_TYPE_ICON)continue;
						uint8_t index = info.number++;
						IconKey key = info.key + std::to_string(index);
						childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
						break;
					}

					// 世話焼き。
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

					// アイコンがnullptrの時は、描画させない。
					if (childIcon == nullptr) continue;

					// アイコンの描画フラグと座標を更新する。
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
				daddyIcon->m_transform.m_localTransform.m_position = m_miniMapStatus->GetMapCenterPos();
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
				if (!enemy) continue;
				
				// シロクマのアイコン数を超えたら描画しない。
				if (bearIconIndex >= static_cast<int>(std::size(POLAR_BEAR_ICON_KEYS))) return;

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
					bearIcon->m_isDraw = true;
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
			for (uint32_t i = bearIconIndex; i < BEAR_ICON_SIZE; i++)
			{
				auto* bearIcon = GetUI<UIIcon>(POLAR_BEAR_ICON_KEYS[i]);
				if (bearIcon) bearIcon->m_isDraw = !m_isDraw;
			}
		}

		
		void MiniMapMenu::InitializeLogic()
		{
			// 初期は非表示。
			auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
			if (miniMapIcon) miniMapIcon->m_isDraw = false;

			for(const auto& info : MINIMAP_ICON_KEYS)
			{
				for (uint8_t i = 0; i < CHILD_PEN_TYPE_ICON; ++i)
				{
					IconKey key = info.key + std::to_string(i);
					auto* childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
					if (childIcon) childIcon->m_isDraw = false;
				}
			}

			//auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
			//if (daddyIcon) daddyIcon->m_isDraw = false;

			//auto* blueIcon = GetUI<UIIcon>(Hash32("childBlueIcon0"));
			//if (blueIcon) blueIcon->m_isDraw = false;

			//auto* orangeIcon = GetUI<UIIcon>(Hash32("childOrangeIcon0"));
			//if (orangeIcon) orangeIcon->m_isDraw = false;

			//auto* pinkIcon = GetUI<UIIcon>(Hash32("childPinkIcon0"));
			//if (pinkIcon) pinkIcon->m_isDraw = false;

			//auto* yellowIcon = GetUI<UIIcon>(Hash32("childYellowIcon0"));
			//if (yellowIcon) yellowIcon->m_isDraw = false;

			//auto* greenIcon = GetUI<UIIcon>(Hash32("childGreenIcon0"));
			//if (greenIcon) greenIcon->m_isDraw = false;

			for (auto& key : POLAR_BEAR_ICON_KEYS)
			{
				auto* polarBear = GetUI<UIIcon>(key);
				if (polarBear) polarBear->m_isDraw = false;
			}
		}
	}
}