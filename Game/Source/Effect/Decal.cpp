/**
 * @file Decal.cpp
 * @brief でかい足跡などのデカールを描画するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "Decal.h"


namespace app {
	namespace effect {
		void Decal::Prepare() {
			if (!m_modelRender) {
				m_modelRender = std::make_unique<nsBeastEngine::ModelRender>();
			}
		}


		void Decal::Spawn(const Vector3& pos, const Vector3& normal, float yaw, float size, DecalKind kind,
			nsK2EngineLow::Texture* texture, const Vector4& color, float lifeSeconds, float fadeOutSeconds,
			int priority, const char* sharedTkmKey, const TerrainHeightInfo& terrainInfo)
		{
			if (!m_isModelInited || m_kind != kind)
			{
				nsK2EngineLow::ModelInitData initData;
				initData.m_tkmFilePath = sharedTkmKey;
				// 深度バッファを読まない専用シェーダーにする（要新規作成）
				initData.m_fxFilePath = "Assets/shader/DecalTerrainHeight.fx";
				initData.m_expandShaderResoruceView[0] = texture;
				// 地形のハイトマップをt1に渡す（凹凸判定用）
				initData.m_expandShaderResoruceView[1] = terrainInfo.heightmapTex;

				initData.m_modelUpAxis = EnModelUpAxis::enModelUpAxisZ;

				m_modelRender->SetForwardRendering(true);

				m_cb.alpha = 1.0f;
				initData.m_expandConstantBuffer2 = &m_cb;
				initData.m_expandConstantBufferSize2 = sizeof(cbDecal);

				// 地形の凹凸判定用の定数バッファ(b1)
				initData.m_expandConstantBuffer = &m_terrainCb;
				initData.m_expandConstantBufferSize = sizeof(cbTerrainHeight);

				m_modelRender->InitFromLoaded(initData);
				m_kind = kind;
				m_isModelInited = true;
			}

			m_modelRender->SetMulColor(color);

			// 箱ではなく板（Quad）を、地形法線に沿わせて配置する。
			// 大きな傾斜はここで吸収し、細かい凹凸はシェーダー側のハイトマップ判定で表現する
			Quaternion yRot;
			yRot.SetRotationY(yaw);
			Quaternion tiltRot;
			tiltRot.SetRotation(Vector3::Up, normal);
			Quaternion finalRot;
			finalRot.Multiply(yRot, tiltRot);

			m_modelRender->SetTRS(pos, finalRot, Vector3(size, 1.0f, size));

			// 回転もオフセットも一切無視して、確実に浮いた場所に出す

			m_remainingLife = lifeSeconds;
			m_fadeOutSeconds = fadeOutSeconds;
			m_priority = priority;
			m_isActive = true;
			m_cb.alpha = 1.0f;
			m_modelRender->SetExpandConstantBuffer2(&m_cb);

			// 地形の凹凸判定用パラメータを更新
			m_terrainCb.halfWidth = terrainInfo.halfWidth;
			m_terrainCb.halfDepth = terrainInfo.halfDepth;
			m_terrainCb.heightScale = terrainInfo.heightScale;
			m_terrainCb.yOffset = terrainInfo.yOffset;

			m_modelRender->Update();
		}


		bool Decal::Update(float deltaTime) {
			if (!m_isActive) return false;
			m_remainingLife -= deltaTime;
			if (m_remainingLife <= 0.0f) {
				m_isActive = false;
				return false;
			}
			m_cb.alpha = (m_remainingLife < m_fadeOutSeconds) ? (m_remainingLife / m_fadeOutSeconds) : 1.0f;
			m_modelRender->SetExpandConstantBuffer2(&m_cb);
			m_modelRender->Update();
			// 毎フレームのカメラ逆行列更新が不要に（そもそも計算していない）
			return true;
		}


		void Decal::Render(RenderContext& rc) {
			if (m_isActive && m_modelRender) {
				m_modelRender->Draw(rc);
			}
		}
	}
}