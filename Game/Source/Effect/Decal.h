#pragma once
#include "Resource/ModelResource.h"
#include <memory>
#include <string>

namespace app {
	namespace effect {
		enum class DecalKind { SnowFootprint, GrassFootprint, RockFootprint };

		// ★追加: 地形の凹凸判定に必要な情報をまとめた構造体
		struct TerrainHeightInfo {
			nsK2EngineLow::Texture* heightmapTex = nullptr; // 地形のハイトマップ（GPU用、フル解像度）
			float halfWidth = 1.0f;
			float halfDepth = 1.0f;
			float heightScale = 1.0f;
			float yOffset = 0.0f;
		};

		class Decal {
		public:
			Decal() = default;
			~Decal() = default;

			Decal(const Decal&) = delete;
			Decal& operator=(const Decal&) = delete;
			Decal(Decal&&) noexcept = default;
			Decal& operator=(Decal&&) noexcept = default;

			void Prepare();
			// ★変更: 深度バッファ方式をやめ、地形法線(normal)＋ハイトマップ(terrainInfo)方式に変更
			void Spawn(const Vector3& pos, const Vector3& normal, float yaw, float size, DecalKind kind,
				nsK2EngineLow::Texture* texture, const Vector4& color, float lifeSeconds, float fadeOutSeconds,
				int priority, const char* sharedTkmKey, const TerrainHeightInfo& terrainInfo);

			bool Update(float deltaTime);
			void Render(RenderContext& rc);

			bool IsActive() const { return m_isActive; }
			float GetRemainingLife() const { return m_remainingLife; }
			int GetPriority() const { return m_priority; }

		private:
			struct cbDecal {
				float alpha = 1.0f;
				float padding[3] = { 0 };
			};
			// ★追加: 地形の凹凸判定用の定数バッファ(b1)。TerrainObjectのTerrainCbとは別物。
			struct cbTerrainHeight {
				float halfWidth = 1.0f;
				float halfDepth = 1.0f;
				float heightScale = 1.0f;
				float yOffset = 0.0f;
			};

			std::unique_ptr<nsBeastEngine::ModelRender> m_modelRender;
			cbDecal         m_cb;
			cbTerrainHeight m_terrainCb; // ★追加

			DecalKind m_kind = DecalKind::SnowFootprint;
			float m_remainingLife = 0.0f;
			float m_fadeOutSeconds = 0.5f;
			int   m_priority = 0;
			bool  m_isActive = false;
			bool  m_isModelInited = false;
		};
	}
}