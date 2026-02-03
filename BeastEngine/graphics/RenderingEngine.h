/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林尚哉
 */
#pragma once
#include "graphics/light/SceneLight.h"
#include "MyRenderer.h"
 // ファイル不足のため一時的に無効化
 // #include "graphics/preRender/ShadowMapRender.h"
 // #include "graphics/postEffect/PostEffect.h"
 // #include "graphics/preRender/LightCulling.h"
 // #include "geometry/SceneGeometryData.h"
 // #include "graphics/light/VolumeLightRender.h"

namespace nsK2Engine {
	class IRenderer;
	class GemometryData;
	class VolumeLightRender; // 前方宣言で逃げる
}

namespace nsBeastEngine {
	using namespace nsK2EngineLow;
	using namespace nsK2Engine;

	class RenderingEngine : public Noncopyable
	{
	public:
		// ディファードライティング用の定数バッファ
		struct SDeferredLightingCB
		{
			Light m_light;
			Matrix mlvp[MAX_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];
			float m_iblLuminance;
			int m_isIBL;
			int m_isEnableRaytracing;
		};

		// イベント
		enum EnEvent {
			enEventReInitIBLTexture,
		};
		struct SEventListenerData {
			void* pListenerObj;
			std::function<void(EnEvent enEvent)> listenerFunc;
		};

		RenderingEngine();
		~RenderingEngine();

		void Init(bool isSoftShadow);
		void Execute(RenderContext& rc);

		// --- ModelRender等が呼ぶアクセサ・登録関数 (k2Engine互換) ---

		void AddRenderObject(IRenderer* renderObject)
		{
			m_renderObjects.push_back(renderObject);
		}

		// ダミー実装: レイトレワールドへの登録
		void AddModelToRaytracingWorld(Model& model) { /* 未実装 */ }
		void RemoveModelFromRaytracingWorld(Model& model) { /* 未実装 */ }

		// ダミー実装: ジオメトリデータ登録
		void RegisterGeometryData(GemometryData* geomData) { /* 未実装 */ }
		void UnregisterGeometryData(GemometryData* geomData) { /* 未実装 */ }

		// ダミー実装: シャドウマップクエリ (空の処理を返すことでModelRenderのエラーを防ぐ)
		void QueryShadowMapTexture(std::function< void(Texture& shadowMap) > queryFunc)
		{
			// シャドウマップがまだないので何もしない
		}

		SDeferredLightingCB& GetDeferredLightingCB() { return m_deferredLightingCB; }

		bool IsSoftShadow() const { return m_isSoftShadow; }

		// IBLテクスチャ取得 (ダミーテクスチャを返す)
		Texture& GetIBLTexture() { return m_dummyIBLTexture; }

		// イベントリスナー
		void AddEventListener(void* pListenerObj, std::function<void(EnEvent enEvent)> listenerFunc)
		{
			m_eventListeners.push_back({ pListenerObj, listenerFunc });
		}
		void RemoveEventListener(void* pListenerObj)
		{
			auto it = std::find_if(
				m_eventListeners.begin(),
				m_eventListeners.end(),
				[&](const SEventListenerData& listenerData) {return listenerData.pListenerObj == pListenerObj; }
			);
			if (it != m_eventListeners.end()) {
				m_eventListeners.erase(it);
			}
		}

	private:
		void InitMainRenderTarget();
		void InitCopyMainRenderTargetToFrameBufferSprite();
		void ForwardRendering(RenderContext& rc);
		void CopyMainRenderTargetToFrameBuffer(RenderContext& rc);
		void Update();

	private:
		static RenderingEngine* m_instance;

		RenderTarget m_mainRenderTarget;
		Sprite m_copyMainRtToFrameBufferSprite;
		std::vector<IRenderer*> m_renderObjects;

		SceneLight m_sceneLight;
		SDeferredLightingCB m_deferredLightingCB;

		std::list< SEventListenerData > m_eventListeners;

		// ダミーデータ
		Texture m_dummyIBLTexture;
		bool m_isSoftShadow = false;

		// 不足しているファイル用のメンバ変数はコメントアウト
		// ShadowMapRender m_shadowMapRenders[MAX_DIRECTIONAL_LIGHT];
		// PostEffect m_postEffect;
		// LightCulling m_lightCulling;
		// SceneGeometryData m_sceneGeometryData;
		// VolumeLightRender m_volumeLightRender;
	};
}