#pragma once
#include "Decal.h"
#include <vector>

namespace app { namespace actor { class TerrainObject; } }

namespace app {
	namespace effect {
		class DecalManager : public Noncopyable {
		public:
			static DecalManager& Get() { static DecalManager instance; return instance; }

			void SpawnFootprint(const Vector3& position, float yawRadian, DecalKind kind = DecalKind::SnowFootprint, float size = 40.0f, float lifeSeconds = 8.0f, float fadeOutSeconds = 1.5f, const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, bool autoDetectSurface = true, int priority = 1);
			void Update();
			void Render(RenderContext& rc);

		private:
			DecalManager() = default;
			~DecalManager() = default;

			void EnsureInited();
			nsK2EngineLow::Texture* GetTextureForKind(DecalKind kind);

			struct GroundHitInfo { bool isHit = false; Vector3 position; Vector3 normal = Vector3::Up; };
			GroundHitInfo RaycastGround(const Vector3& fromPosXZ) const;

		private:
			static constexpr int MAX_DECAL_NUM = 64;
			static constexpr int MAX_CHILD_DECAL_NUM = 40;
			static constexpr float RAY_START_HEIGHT = 50.0f;
			static constexpr float RAY_MAX_DISTANCE = 200.0f;
			static constexpr float PROJECTED_SURFACE_OFFSET = 1.0f;

			std::vector<Decal> m_decals;

			nsK2EngineLow::Texture m_snowFootprintTex;
			nsK2EngineLow::Texture m_grassFootprintTex;
			nsK2EngineLow::Texture m_rockFootprintTex;

			bool m_isInited = false;

			TerrainHeightInfo BuildTerrainHeightInfo() const;
		};
	}
}