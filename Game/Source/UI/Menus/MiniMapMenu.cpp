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
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Manager/IglooManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Nature/WhirlpoolManager.h"


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

			// 渦潮のアイコン数。
			constexpr uint32_t WHIRLPOOL_ICON_SIZE = 10;
			// 渦潮のアイコンキー。
			constexpr std::array<uint32_t, WHIRLPOOL_ICON_SIZE> WHIRLPOOL_ICON_KEYS =
			{
					Hash32("WhirlpoolIcon0")
				,	Hash32("WhirlpoolIcon1")
				,	Hash32("WhirlpoolIcon2")
				,	Hash32("WhirlpoolIcon3")
				,	Hash32("WhirlpoolIcon4")
				,	Hash32("WhirlpoolIcon5")
				,	Hash32("WhirlpoolIcon6")
				,	Hash32("WhirlpoolIcon7")
				,	Hash32("WhirlpoolIcon8")
				,	Hash32("WhirlpoolIcon9")
			};

			// イグルーのアイコンの数。
			constexpr uint32_t IGLOO_SIZE = 3;
			// イグルーの配列。
			constexpr std::array<uint32_t, IGLOO_SIZE> IGLOO_ICON_KEYS = {
					Hash32("IglooIcon0")
				,	Hash32("IglooIcon1")
				,	Hash32("IglooIcon2")
			};

			// シロクマの巣のアイコン数。
			constexpr uint32_t BEAR_NEST_ICON_SIZE = 3;
			// シロクマの巣のアイコンキー。
			constexpr std::array<uint32_t, BEAR_NEST_ICON_SIZE> BEAR_NEST_ICON_KEYS =
			{
					Hash32("PBNestIcon0")
				,	Hash32("PBNestIcon1")
				,	Hash32("PBNestIcon2")
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

			// ステージシステムのインスタンスを取得しておく。
			app::actor::StageSystem::GetInstance();

			// イグルーのマネージャーのインスタンスを生成。
			app::actor::IglooManager::GetInstance();

			// 渦潮のマネージャーのインスタンスを生成。
			app::nature::WhirlpoolManager::GetInstance();

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

				auto* mapFrameIcon = GetUI<UIIcon>(Hash32("MiniMapFrameIcon"));
				if (mapFrameIcon) mapFrameIcon->m_isDraw = false;

				auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
				if (daddyIcon) daddyIcon->m_isDraw = false;

				// シロクマの巣のアイコンを全て非表示にする。
				for (const auto& key : BEAR_NEST_ICON_KEYS)
				{
					auto* bearNestsIcon = GetUI<UIIcon>(key);
					if (bearNestsIcon) bearNestsIcon->m_isDraw = false;
				}

				// 渦潮のアイコンを全て非表示にする。
				for (const auto& key : WHIRLPOOL_ICON_KEYS)
				{
					auto* whirlpoolIcon = GetUI<UIIcon>(key);
					if (whirlpoolIcon) whirlpoolIcon->m_isDraw = false;
				}

				// イグルーのアイコンを全て非表示にする。
				for (const auto& key : IGLOO_ICON_KEYS)
				{
					auto* iglooIcon = GetUI<UIIcon>(key);
					if (iglooIcon) iglooIcon->m_isDraw = false;
				}

				// シロクマの巣のアイコンを全て非表示にする。
				for (const auto& key : BEAR_NEST_ICON_KEYS)
				{
					auto* bearNestsIcon = GetUI<UIIcon>(key);
					if (bearNestsIcon) bearNestsIcon->m_isDraw = false;
				}

				// 渦潮のアイコンを全て非表示にする。
				for (const auto& key : WHIRLPOOL_ICON_KEYS)
				{
					auto* whirlpoolIcon = GetUI<UIIcon>(key);
					if (whirlpoolIcon) whirlpoolIcon->m_isDraw = false;
				}

				// イグルーのアイコンを全て非表示にする。
				for (const auto& key : IGLOO_ICON_KEYS)
				{
					auto* iglooIcon = GetUI<UIIcon>(key);
					if (iglooIcon) iglooIcon->m_isDraw = false;
				}

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

			// ミニマップのフレームアイコンを表示する。
			auto* mapFrameIcon = GetUI<UIIcon>(Hash32("MiniMapFrameIcon"));
			if (mapFrameIcon) mapFrameIcon->m_isDraw = true;

			// マップのフレームアイコンをカメラの向きに合わせて回転させる。
			MapFrameRotation();

			// 親ペンギンのが存在する時に
			// @detail 親ペンギン、子ペンギン、シロクマ、渦潮、イグルー、シロクマの巣のアイコンをミニマップに表示する。
			if (m_daddyPenguin)
			{
				MapPolarBearNest();
				MapPolarBear();
				MapChildPen();
				MapWhirlpool();
				MapDaddyPen();
				MapIgloo();
			}


			MenuBase::Update();
		}


		bool MiniMapMenu::WorldPosConverterToMapPos(Vector3 worldCenterPos, Vector3 worldPos, Vector3& mapPos)
		{
			worldCenterPos.y = 0.0f;
			worldPos.y = 0.0f;

			Vector3 diff = worldPos - worldCenterPos;
			const float diffLengthSq = diff.LengthSq();

			// あらかじめ距離の上限を計算しておく。
			const float dis = m_miniMapStatus->GetLimitDistance();
			const float disSq = std::pow(dis, 2.0f);

			// ワールド座標の差分の長さの二乗が距離の上限の二乗ならば、マップに表示しない。
			if (diffLengthSq >= disSq)
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


		void MiniMapMenu::MapFrameRotation()
		{
			// ミニマップを回転させるためのアイコンを取得。
			auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
			auto* mapFrameIcon = GetUI<UIIcon>(Hash32("MiniMapFrameIcon"));

			if (miniMapIcon && mapFrameIcon)
			{
				// カメラの向きに合わせてマップのフレームアイコンを回転させる。
				Vector3 forward = g_camera3D->GetForward();
				const float angle = (atan2(forward.x, forward.z));

				Quaternion qrot;
				// Z軸回転のを適用。
				qrot.SetRotationZ(angle);
				// 各アイコンに回転を適用させる。
				miniMapIcon->m_transform.m_localTransform.m_rotation = qrot;
				mapFrameIcon->m_transform.m_localTransform.m_rotation = qrot;
			}
		}


		void MiniMapMenu::MapPolarBearNest()
		{
			// ステージシステムのオブジェクトマップを取得する。
			const auto& objSystem = actor::StageSystem::GetInstance()->GetObjectMap();

			// インデックスを追加。
			uint32_t index = 0;

			for (const auto& pair : objSystem)
			{
				// オブジェクトのキーを取得する。
				const std::string& key = pair.first;
				// オブジェクトを取得する。
				const auto& stageObj = pair.second;

				// オブジェクトのキーが "bearHome_"で始まるものだけを対象とする。
				if (key.find("bearHome_") != 0) continue;

				// シロクマの巣がマップに表示できる数を超えたら描画しない。
				if (index >= BEAR_NEST_ICON_SIZE) return;
				
				auto* bearNestIcon = GetUI<UIIcon>(BEAR_NEST_ICON_KEYS[index]);
				// 無いときに次のアイコンに行くためにインデックスを増加させる。
				if (!bearNestIcon) {
					index++;
					return;
				}

				// ワールド座標をマップ座標に変換するための座標を用意。
				const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;
				const Vector3& nestPos = stageObj->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;


				if (WorldPosConverterToMapPos(daddyPos, nestPos, mapPos))
				{
					// 座標を更新して、ミニマップの範囲内なら表示する。
					bearNestIcon->m_transform.m_localTransform.m_position = mapPos;
					bearNestIcon->m_isDraw = true;
				}
				else
				{
					bearNestIcon->m_isDraw = false;
				}

				// シロクマの巣のアイコンの要素数を超えないようにインデックスを増加させる。
				index++;
			}

			// 使われなかったアイコンを非表示にする。
			for (uint32_t i = index; i < BEAR_NEST_ICON_SIZE; index++)
			{
				auto* bearNestIcon = GetUI<UIIcon>(BEAR_NEST_ICON_KEYS[index]);
				if (bearNestIcon) bearNestIcon->m_isDraw = false;
			}
		}


		void MiniMapMenu::MapWhirlpool()
		{
			// 渦潮のマネージャーを取得する。
			const auto& wrhirlpoolMgr = nature::WhirlpoolManager::GetInstance();
			
			// 渦潮のアイコンのインデックスを用意する。
			uint32_t whirlpoolIndex = 0;

			// 渦潮の数だけループさせる。
			wrhirlpoolMgr->ForEach([&](nature::Whirlpool* whirlpool)
			{
				// 渦潮がマップに表示できる数を超えたら描画しない。
				if (whirlpoolIndex >= WHIRLPOOL_ICON_SIZE) return;

				auto* whirlpoolIcon = GetUI<UIIcon>(WHIRLPOOL_ICON_KEYS[whirlpoolIndex]);
				
				// 無いときに次のアイコンに行くためにインデックスを増加させる。
				if (!whirlpoolIcon) {
					whirlpoolIndex++;
					return;
				}

				// ワールド座標をマップ座標に変換するための座標を用意。
				const Vector3& daddyPos = m_daddyPenguin->GetTransform().m_position;
				const Vector3& whirlpoolPos = whirlpool->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;
				
				if (WorldPosConverterToMapPos(daddyPos, whirlpoolPos, mapPos))
				{
					// 座標を更新して、ミニマップの範囲内なら表示する。
					whirlpoolIcon->m_transform.m_localTransform.m_position = mapPos;
					whirlpoolIcon->m_isDraw = true;
				}
				else
				{
					whirlpoolIcon->m_isDraw = false;
				}

				// 渦潮のアイコンの要素数を超えないようにインデックスを増加させる。
				whirlpoolIndex++;
			});

			// 使われなかったアイコンを非表示にする。
			for (uint32_t i = whirlpoolIndex; i < WHIRLPOOL_ICON_SIZE; i++)
			{
				auto* whirlpoolIcon = GetUI<UIIcon>(WHIRLPOOL_ICON_KEYS[i]);
				if (whirlpoolIcon) whirlpoolIcon->m_isDraw = false;
			 }
		}


		void MiniMapMenu::MapIgloo()
		{
			// ステージオブジェクトを取得する。
			const auto& iglooObj = actor::StageSystem::GetInstance()->GetObjectMap();

			// 親ペンギンの座標を取得しておく。
			const Vector3 daddyPos = m_daddyPenguin->GetTransform().m_position;

			// イグルーのアイコンのインデックスを用意する。
			uint32_t iglooIndex = 0;

			// ステージオブジェクトの数だけル―プさせる。
			for (const auto& pari : iglooObj)
			{
				// オブジェクトのキーを取得する。
				const std::string& key = pari.first;
				// オブジェクトを取得する。
				const auto& stageObj = pari.second;

				// キーが "igloo"を含むものだけを対象とする。
				// @datail もしキーに"igloo"が含まれていないときは、ありえない値が代入されて処理をスキップする。
				if (key.find("igloo") == std::string::npos) continue;

				// イグルーがマップに表示できる数を超えたら描画しない。
				if (iglooIndex >= IGLOO_SIZE) return;
				
				auto* iglooIcon = GetUI<UIIcon>(IGLOO_ICON_KEYS[iglooIndex]);
				
				// 無いときに次のアイコンに行くためにインデックスを増加させる。
				if (!iglooIcon) {
					iglooIndex++;
					return;
				}
				
				// ワールド座標をマップ座標に変換するための座標を用意。
				const Vector3 iglooPos = stageObj->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;
				
				if (WorldPosConverterToMapPos(daddyPos, iglooPos, mapPos))
				{
					// 座標を更新して、ミニマップの範囲内なら表示する。
					iglooIcon->m_transform.m_localTransform.m_position = mapPos;
					iglooIcon->m_isDraw = true;
				}
				else
				{
					iglooIcon->m_isDraw = false;
				}

				// イグルーのアイコンの要素数を超えないようにインデックスを増加させる。
				iglooIndex++;
			}

			// 使われなかったアイコンを非表示にする。
			for (uint32_t i = iglooIndex; i < IGLOO_SIZE; i++)
			{
				auto* iglooIcon = GetUI<UIIcon>(IGLOO_ICON_KEYS[i]);
				if (iglooIcon) iglooIcon->m_isDraw = false;
			}
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
					if (icon) icon->m_isDraw = false;
				}
				// アイコンの数を0にして、indexの値がおかしくならないようにさせる。
				info.number = 0;
			}

			for (auto* child : childPenMgr)
			{
				// 子ペンギン以外は描画させない。
				if (!child) continue;

				// ワールド座標をマップ座標に変換する座標を用意。
				Vector3 daddyPos = m_daddyPenguin->GetTransform().m_position;
				Vector3 childPos = child->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;

				if (WorldPosConverterToMapPos(daddyPos, childPos, mapPos))
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
			for (const auto& enemy : enemis)
			{	
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
				const Vector3 daddyPos = m_daddyPenguin->GetTransform().m_position;
				Vector3 enemyPos = enemy->GetTransform().m_position;
				Vector3 mapPos = Vector3::Zero;

				// ワールド座標をマップ座標に変換出来たら、シロクマのアイコンをミニマップに表示する。
				if (WorldPosConverterToMapPos(daddyPos, enemyPos, mapPos))
				{
					// 座標更新。
					bearIcon->m_transform.m_localTransform.m_position = mapPos;
					// ミニマップの範囲内は表示。
					bearIcon->m_isDraw = true;
				}
				else
				{
					// ミニマップの範囲外は非表示。
					if(bearIcon) bearIcon->m_isDraw = false;
				}

				// シロクマのアイコンの要素数を超えないようにインデックスを増加させる。
				bearIconIndex++;
			}
			
			// 使われなかったアイコンを非表示にする。
			for (uint32_t i = bearIconIndex; i < BEAR_ICON_SIZE; i++)
			{
				auto* bearIcon = GetUI<UIIcon>(POLAR_BEAR_ICON_KEYS[i]);
				if (bearIcon) bearIcon->m_isDraw = false;
			}
		}

		
		void MiniMapMenu::InitializeLogic()
		{
			// 初期は非表示。
			auto* miniMapIcon = GetUI<UIIcon>(Hash32("MiniMapIcon"));
			if (miniMapIcon) miniMapIcon->m_isDraw = false;

			auto* mapFrameIcon = GetUI<UIIcon>(Hash32("MiniMapFrameIcon"));
			if(mapFrameIcon) mapFrameIcon->m_isDraw = false;

			auto* daddyIcon = GetUI<UIIcon>(Hash32("DaddyIcon"));
			if (daddyIcon) daddyIcon->m_isDraw = false;

			for (const auto& key : BEAR_NEST_ICON_KEYS)
			{
				auto* bearNestsIcon = GetUI<UIIcon>(key);
				if (bearNestsIcon) bearNestsIcon->m_isDraw = false;
			}

			for (const auto& key : WHIRLPOOL_ICON_KEYS)
			{
				auto* whirlpoolIcon = GetUI<UIIcon>(key);
				if (whirlpoolIcon) whirlpoolIcon->m_isDraw = false;
			}

			for (const auto& key : IGLOO_ICON_KEYS)
			{
				auto* iglooIcon = GetUI<UIIcon>(key);
				if (iglooIcon) iglooIcon->m_isDraw = false;
			}

			for(const auto& info : MINIMAP_ICON_KEYS)
			{
				for (uint8_t i = 0; i < CHILD_PEN_TYPE_ICON; ++i)
				{
					IconKey key = info.key + std::to_string(i);
					auto* childIcon = GetUI<UIIcon>(Hash32(key.c_str()));
					if (childIcon) childIcon->m_isDraw = false;
				}
			}

			for (auto& key : POLAR_BEAR_ICON_KEYS)
			{
				auto* polarBear = GetUI<UIIcon>(key);
				if (polarBear) polarBear->m_isDraw = false;
			}
		}
	}
}