/**
 * @file Decal.h
 * @brief でかい足跡などのデカールを描画するクラス
 */
#pragma once
#include "Resource/ModelResource.h"
#include <memory>
#include <string>


namespace app {
	namespace effect {
		enum class DecalKind { SnowFootprint, GrassFootprint, RockFootprint, BearFootprint };

		/** @brief デカール種別の総数。DecalKind に種類を足したらここも増やすこと */
		static constexpr int DECAL_KIND_NUM = 4;

		// 地形の凹凸判定に必要な情報をまとめた構造体
		struct TerrainHeightInfo {
			nsK2EngineLow::Texture* heightmapTex = nullptr; // 地形のハイトマップ（GPU用、フル解像度）
			float halfWidth = 1.0f;
			float halfDepth = 1.0f;
			float heightScale = 1.0f;
			float yOffset = 0.0f;
			/** 地形の世代番号。ステージが変わるたびに増える（下記参照） */
			int   generation = 0;
		};

		class Decal {
		public:
			Decal() = default;
			~Decal() = default;

			Decal(const Decal&) = delete;
			Decal& operator=(const Decal&) = delete;
			Decal(Decal&&) noexcept = default;
			Decal& operator=(Decal&&) noexcept = default;

			/**
			 * @brief デカール用モデルレンダーの実体を生成する
			 * @details インスタンスの使い回しを前提とし、初回のみ生成する
			 */
			void Prepare();

			/**
			 * @brief デカールを指定位置に生成・再配置する
			 * @param pos 生成位置
			 * @param normal 地形法線（板を地形に沿わせるために使用）
			 * @param yaw Y軸回転（ラジアン）
			 * @param size デカールのサイズ
			 * @param kind デカールの種類
			 * @param texture 使用するテクスチャ
			 * @param color 乗算カラー
			 * @param lifeSeconds 生存時間（秒）
			 * @param fadeOutSeconds フェードアウトにかける時間（秒）
			 * @param priority 優先度（大きいほど消されにくい）
			 * @param sharedTkmKey 共有メッシュのバンクキー
			 * @param terrainInfo 地形の凹凸判定用パラメータ
			 */
			void Spawn(const Vector3& pos, const Vector3& normal, float yaw, float size, DecalKind kind,
				nsK2EngineLow::Texture* texture, const Vector4& color, float lifeSeconds, float fadeOutSeconds,
				int priority, const char* sharedTkmKey, const TerrainHeightInfo& terrainInfo);

			bool Update(float deltaTime);
			void Render(RenderContext& rc);

			/** @brief 現在アクティブかどうかを取得する */
			bool IsActive() const { return m_isActive; }

			/**
			 * @brief デカールを即座に非アクティブ化する
			 * @details ステージ遷移時に呼ぶ。破棄された地形テクスチャを参照したまま
			 *          描画され続けるのを防ぐ
			 */
			void Deactivate() { m_isActive = false; }

			/**
			 * @brief モデルが未初期化・種類違い・地形世代違いなら初期化する
			 * @details Spawn() の内部処理だが、ロード中の事前ウォームアップからも呼ぶ。
			 *          初期化は1回8ms前後かかるため、プレイ中の初回スポーンで行うと
			 *          フレーム落ちになる。ロード画面のうちに全スロットへ済ませておく
			 * @return 初期化を実行したらtrue（初期化済みで何もしなかったらfalse）
			 */
			bool InitModelIfNeeded(DecalKind kind, nsK2EngineLow::Texture* texture,
				const char* sharedTkmKey, const TerrainHeightInfo& terrainInfo);

			/** @brief 残り生存時間を取得する */
			float GetRemainingLife() const { return m_remainingLife; }

			/** @brief 優先度を取得する（大きいほど消されにくい） */
			int GetPriority() const { return m_priority; }


		private:
			struct cbDecal {
				float alpha = 1.0f;
				float padding[3] = { 0 };
			};
			// 地形の凹凸判定用の定数バッファ(b1)。TerrainObjectのTerrainCbとは別物。
			struct cbTerrainHeight {
				float halfWidth = 1.0f;
				float halfDepth = 1.0f;
				float heightScale = 1.0f;
				float yOffset = 0.0f;
			};

			std::unique_ptr<nsBeastEngine::ModelRender> m_modelRender;
			cbDecal         m_cb;
			cbTerrainHeight m_terrainCb;

			DecalKind m_kind = DecalKind::SnowFootprint;
			float m_remainingLife = 0.0f;
			float m_fadeOutSeconds = 0.5f;
			int   m_priority = 0;
			bool  m_isActive = false;
			bool  m_isModelInited = false;
			/** モデル初期化時に地形ハイトマップをバインドした世代。
			 *  ステージが変わると地形テクスチャが破棄されるため、世代が違ったら
			 *  再初期化して新しいハイトマップをバインドし直す。
			 *  これを怠ると解放済みテクスチャをGPUが読みに行き、デバイスハングする */
			int   m_terrainGeneration = -1;
		};
	}
}