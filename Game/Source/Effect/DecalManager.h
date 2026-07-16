/**
 * @file DecalManager.h
 * @brief Decal（本体）の生成・更新・描画をまとめて管理するマネージャー
 * @details EffectManager / SoundManager と同じ Get() シングルトンパターンを踏襲。
 *          呼び出し例: DecalManager::Get().SpawnFootprint(footPos, yaw);
 *
 *          足跡の設置位置は、渡された座標の真上から下方向へレイキャストして
 *          実際の地面の高さを求めてから決定する（RaycastGround を参照）。
 *          さらに、その周辺を格子状にTerrainObject::GetHeightAtでサンプリングし、
 *          地形の凹凸に沿った投影デカール用メッシュを組み立てる。
 */
#pragma once
#include "Decal.h"
#include <vector>

 /** 前方宣言（.cppでのみ実体を使用するため、ヘッダーはこれで十分） */
namespace app
{
	namespace actor
	{
		class TerrainObject;
	}
}

namespace app
{
	namespace effect
	{
		/**
		 * @brief 足跡デカールの生成・更新・描画を行うシングルトンマネージャー
		 */
		class DecalManager : public Noncopyable
		{
		public:
			/** @brief シングルトンインスタンスを取得 */
			static DecalManager& Get()
			{
				static DecalManager instance;
				return instance;
			}

			/**
			 * @brief 足跡デカールを1枚生成する
			 * @details position の真上から下方向へレイキャストし、実際の地面のヒット位置を
			 *          求めてから設置する。地面が見つからない場合（崖・水上など）は生成しない。
			 *          地形（TerrainObject）が取得できる場合は、着地点周辺を格子状にサンプリングして
			 *          地形の凹凸に沿った投影デカールメッシュを組み立てる。
			 * @param position         キャラクターの現在位置（XZのみ使用。Yはレイの開始点計算にのみ使用）
			 * @param yawRadian        進行方向に合わせたY軸回転（ラジアン）
			 * @param kind             デカール種別。autoDetectSurface=true の場合、地形が判定できれば上書きされる
			 * @param size             一辺のサイズ（ワールド単位）
			 * @param lifeSeconds      表示時間（秒）
			 * @param fadeOutSeconds   フェードアウトにかける時間（秒）
			 * @param color            テクスチャに掛けるティント色。autoDetectSurface=true の場合、地形が判定できれば上書きされる
			 * @param autoDetectSurface true の場合、着地点の地形（雪/草/岩）を自動判定して kind と color を決定する
			 */
			void SpawnFootprint(
				const Vector3& position,
				float yawRadian,
				DecalKind kind = DecalKind::SnowFootprint,
				float size = 40.0f,
				float lifeSeconds = 8.0f,
				float fadeOutSeconds = 1.5f,
				const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
				bool autoDetectSurface = true
			);

			/** @brief 更新処理。毎フレーム呼び出すこと。 */
			void Update();

			/** @brief 描画処理。地形などの描画後に呼び出すこと。 */
			void Render(RenderContext& rc);

		private:
			DecalManager() = default;
			~DecalManager() = default;

			/** @brief 初回呼び出し時に一度だけ初期化する（テクスチャロード・プール確保） */
			void EnsureInited();

			/** @brief 種別に応じたテクスチャを取得する */
			nsK2EngineLow::Texture* GetTextureForKind(DecalKind kind);

			/** @brief 地面へのレイキャスト結果 */
			struct GroundHitInfo
			{
				bool    isHit = false;
				Vector3 position;
				Vector3 normal = Vector3(0.0f, 1.0f, 0.0f);
			};

			/**
			 * @brief 指定XZ座標の真上から下方向へレイを飛ばし、地面のヒット位置を取得する
			 */
			GroundHitInfo RaycastGround(const Vector3& fromPosXZ) const;

			/**
			 * @brief 投影デカール用の格子頂点（ワールド座標）を組み立てる
			 * @details terrain が nullptr の場合は center.y を平坦に使う（レイキャストのフォールバック）。
			 * @param center     格子の中心（ワールド座標。yは地形が取れない場合のフォールバックに使用）
			 * @param yawRadian  格子を回転させる角度（進行方向に合わせる）
			 * @param size       格子1辺のワールドサイズ
			 * @param terrain    高さサンプリングに使う地形（nullptr可）
			 * @param gridResolution 1辺あたりの頂点数
			 */
			std::vector<Vector3> BuildProjectedGridPositions(
				const Vector3& center,
				float yawRadian,
				float size,
				actor::TerrainObject* terrain,
				int gridResolution
			) const;

		private:
			/** @brief 同時表示できるデカールの最大数（超えたら古いものから再利用） */
			static constexpr int MAX_DECAL_NUM = 32;
			/** @brief レイの開始点を fromPosXZ よりどれだけ上にするか（ワールド単位） */
			static constexpr float RAY_START_HEIGHT = 50.0f;
			/** @brief レイの最大到達距離（ワールド単位） */
			static constexpr float RAY_MAX_DISTANCE = 200.0f;
			/** @brief 投影デカールの格子解像度（1辺あたりの頂点数。5なら5x5=25頂点） */
			static constexpr int GRID_RESOLUTION = 5;
			/** @brief 地形面からのZファイティング回避オフセット（ワールド単位） */
			static constexpr float PROJECTED_SURFACE_OFFSET = 1.0f;

			std::vector<Decal> m_decals;

			nsK2EngineLow::Texture m_snowFootprintTex;
			nsK2EngineLow::Texture m_grassFootprintTex;
			nsK2EngineLow::Texture m_rockFootprintTex;

			bool m_isInited = false;
		};
	}
}
