/**
 * @file TerrainCircle.cpp
 * @brief 地形に追従し、内側を塗りつぶせる円の描画クラス
 * @author 竹林
 */
#include "stdafx.h"
#include "TerrainCircle.h"
#include "Source/Nature/Ocean.h"


namespace app
{
	namespace actor
	{
		void TerrainCircle::Init(int segments, const Vector4& edgeColor, const Vector4& fillColor, bool hasFill)
		{
			m_segments  = segments;
			m_edgeColor = edgeColor;
			m_fillColor = fillColor;
			m_hasFill   = hasFill;

			/** LINE_LIST: 1 セグメント = 始点 + 終点 = 2 頂点 */
			const int edgeVerts = segments * 2;
			m_edgeVerts.resize(edgeVerts);
			m_edgeVertexBuffer.Init(sizeof(Vertex) * edgeVerts, sizeof(Vertex));
			m_edgeIndexBuffer.Init(sizeof(uint16_t) * edgeVerts, sizeof(uint16_t));

			std::vector<uint16_t> edgeIdx(edgeVerts);
			for (int i = 0; i < edgeVerts; ++i)
			{
				edgeIdx[i] = static_cast<uint16_t>(i);
			}
			m_edgeIndexBuffer.Copy(edgeIdx.data());

			if (m_hasFill)
			{
				/** TRIANGLE_LIST: 1 セグメント = 中心 + 始点 + 終点 = 3 頂点 */
				const int fillVerts = segments * 3;
				m_fillVerts.resize(fillVerts);
				m_fillVertexBuffer.Init(sizeof(Vertex) * fillVerts, sizeof(Vertex));
				m_fillIndexBuffer.Init(sizeof(uint16_t) * fillVerts, sizeof(uint16_t));

				std::vector<uint16_t> fillIdx(fillVerts);
				for (int i = 0; i < fillVerts; ++i)
				{
					fillIdx[i] = static_cast<uint16_t>(i);
				}
				m_fillIndexBuffer.Copy(fillIdx.data());
			}
		}


		void TerrainCircle::Update(const Vector3& center, float radius)
		{
			m_edgeVertexCount = 0;
			m_fillVertexCount = 0;

			const float angleStep = 2.0f * Math::PI / m_segments;

			/** 円周上の各頂点で地表 Y を個別にサンプリングして地形に追従させる */
			std::vector<Vector3> points(m_segments);
			for (int i = 0; i < m_segments; ++i)
			{
				const float angle = i * angleStep;
				const float x     = center.x + radius * cosf(angle);
				const float z     = center.z + radius * sinf(angle);
				const float y     = SampleSurfaceY(x, z) + HEIGHT_OFFSET;
				points[i]         = Vector3(x, y, z);
			}

			/** LINE_LIST: セグメント i は points[i] → points[i+1]（最後は 0 番に折り返す） */
			for (int i = 0; i < m_segments; ++i)
			{
				const int next                    = (i + 1) % m_segments;
				m_edgeVerts[m_edgeVertexCount++]  = { points[i],    m_edgeColor };
				m_edgeVerts[m_edgeVertexCount++]  = { points[next], m_edgeColor };
			}

			if (m_hasFill)
			{
				/** 中心点も地表 Y をサンプリング */
				const Vector3 centerPt(center.x, SampleSurfaceY(center.x, center.z) + HEIGHT_OFFSET, center.z);

				/** TRIANGLE_LIST: セグメント i は 中心 → points[i] → points[i+1] */
				for (int i = 0; i < m_segments; ++i)
				{
					const int next                    = (i + 1) % m_segments;
					m_fillVerts[m_fillVertexCount++]  = { centerPt,     m_fillColor };
					m_fillVerts[m_fillVertexCount++]  = { points[i],    m_fillColor };
					m_fillVerts[m_fillVertexCount++]  = { points[next], m_fillColor };
				}
			}
		}


		void TerrainCircle::RenderFill(RenderContext& rc)
		{
			if (m_fillVertexCount == 0 || !m_hasFill) return;
			m_fillVertexBuffer.Copy(m_fillVerts.data());
			rc.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			rc.SetVertexBuffer(m_fillVertexBuffer);
			rc.SetIndexBuffer(m_fillIndexBuffer);
			rc.DrawIndexed(static_cast<UINT>(m_fillVertexCount));
		}


		void TerrainCircle::RenderEdge(RenderContext& rc)
		{
			if (m_edgeVertexCount == 0) return;
			m_edgeVertexBuffer.Copy(m_edgeVerts.data());
			rc.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			rc.SetVertexBuffer(m_edgeVertexBuffer);
			rc.SetIndexBuffer(m_edgeIndexBuffer);
			rc.DrawIndexed(static_cast<UINT>(m_edgeVertexCount));
		}


		float TerrainCircle::SampleSurfaceY(float x, float z)
		{
			/** レイキャストで地表の高さをサンプリング */
			const Vector3 rayStart(x, RAYCAST_TOP,    z);
			const Vector3 rayEnd  (x, RAYCAST_BOTTOM, z);

			nsBeastEngine::nsCollision::RaycastHit hit;
			if (nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit))
			{
				return hit.point.y;
			}

			/** 地形がなければ海面（波を含む）でサンプリング */
			if (const nature::Ocean* ocean = nature::Ocean::GetInstance())
			{
				return ocean->SampleWaveHeight(x, z);
			}

			return 0.0f;
		}
	}
}
