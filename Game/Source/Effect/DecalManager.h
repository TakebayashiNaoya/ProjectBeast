/**
 * @file DecalManager.h
 * @brief デカールを管理するクラス
 * @author 立山
 */
#pragma once
#include "Decal.h"
#include <vector>

namespace app { namespace actor { class TerrainObject; } }

namespace app {
	namespace effect {
		class DecalManager : public Noncopyable {
		public:
			static DecalManager& Get() { static DecalManager instance; return instance; }

			/**
			 * @brief 足跡デカールを1つ生成する
			 * @param position 生成位置
			 * @param yawRadian Y軸回転（ラジアン）
			 * @param kind デカールの種類
			 * @param autoDetectSurface trueの場合、地形サーフェスに応じてkindとcolorを自動判定する
			 */
			void SpawnFootprint(const Vector3& position, float yawRadian, DecalKind kind = DecalKind::SnowFootprint,
				float size = DEFAULT_FOOTPRINT_SIZE, float lifeSeconds = DEFAULT_LIFE_SECONDS,
				float fadeOutSeconds = DEFAULT_FADE_OUT_SECONDS, const Vector4& color = DEFAULT_FOOTPRINT_COLOR,
				bool autoDetectSurface = true, int priority = DEFAULT_PRIORITY);

			void Update();
			void Render(RenderContext& rc);


		private:
			DecalManager() = default;
			~DecalManager() = default;

			/**
			 * @brief テクスチャ・デカール配列・共有メッシュの初期化を行う
			 * @details 初回呼び出し時のみ実行され、以降は何もしない
			 */
			void EnsureInited();

			/**
			 * @brief デカール種別に対応するテクスチャを取得する
			 * @param kind デカールの種類
			 * @return 対応するテクスチャへのポインタ
			 */
			nsK2EngineLow::Texture* GetTextureForKind(DecalKind kind);

			/** @brief 地面へのレイキャスト結果 */
			struct GroundHitInfo { bool isHit = false; Vector3 position; Vector3 normal = Vector3::Up; };

			/**
			 * @brief 指定座標の真下に向けて地面へレイキャストする
			 * @param fromPosXZ レイの発射基準となるXZ座標（Yは内部でオフセットして調整）
			 * @return ヒット結果（位置・法線・ヒットフラグ）
			 */
			GroundHitInfo RaycastGround(const Vector3& fromPosXZ) const;


			static constexpr int MAX_DECAL_NUM = 64;
			static constexpr int MAX_CHILD_DECAL_NUM = 40;
			static constexpr float RAY_START_HEIGHT = 50.0f;
			static constexpr float RAY_MAX_DISTANCE = 200.0f;
			static constexpr float PROJECTED_SURFACE_OFFSET = 1.0f;

			static constexpr float DEFAULT_FOOTPRINT_SIZE = 40.0f;
			static constexpr float DEFAULT_LIFE_SECONDS = 8.0f;
			static constexpr float DEFAULT_FADE_OUT_SECONDS = 1.5f;
			static inline const Vector4 DEFAULT_FOOTPRINT_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };
			static constexpr int DEFAULT_PRIORITY = 1;

			std::vector<Decal> m_decals;

			nsK2EngineLow::Texture m_snowFootprintTex;
			nsK2EngineLow::Texture m_grassFootprintTex;
			nsK2EngineLow::Texture m_rockFootprintTex;
			nsK2EngineLow::Texture m_bearFootprintTex;

			bool m_isInited = false;

			/**
			 * @brief 現在のステージ地形から、デカール描画に必要な凹凸判定用パラメータを構築する
			 * @return 地形の高さ情報（ハイトマップ・サイズ・スケール等）
			 */
			TerrainHeightInfo BuildTerrainHeightInfo() const;
		};
	}
}