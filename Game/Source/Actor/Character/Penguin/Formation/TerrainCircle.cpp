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


		void TerrainCircle::Update(const Vector3& center, float radius, float alphaScale)
		{
			m_edgeVertexCount = 0;
			m_fillVertexCount = 0;

			/** フェード用にアルファだけスケールした色を作る */
			Vector4 edgeColor = m_edgeColor;
			edgeColor.w *= alphaScale;
			Vector4 fillColor = m_fillColor;
			fillColor.w *= alphaScale;

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
				m_edgeVerts[m_edgeVertexCount++]  = { points[i],    edgeColor };
				m_edgeVerts[m_edgeVertexCount++]  = { points[next], edgeColor };
			}

			if (m_hasFill)
			{
				/** 中心点も地表 Y をサンプリング */
				const Vector3 centerPt(center.x, SampleSurfaceY(center.x, center.z) + HEIGHT_OFFSET, center.z);

				/** TRIANGLE_LIST: セグメント i は 中心 → points[i] → points[i+1] */
				for (int i = 0; i < m_segments; ++i)
				{
					const int next                    = (i + 1) % m_segments;
					m_fillVerts[m_fillVertexCount++]  = { centerPt,     fillColor };
					m_fillVerts[m_fillVertexCount++]  = { points[i],    fillColor };
					m_fillVerts[m_fillVertexCount++]  = { points[next], fillColor };
				}
			}
		}


		void TerrainCircle::InitBand(int segments, const Vector4& color)
		{
			m_bandSegments = segments;
			m_bandColor = color;

			/** TRIANGLE_LIST: 1セグメント = (外側⇔中央) 2枚 + (中央⇔内側) 2枚 = 12頂点 */
			const int bandVerts = segments * 12;
			m_bandVerts.resize(bandVerts);
			m_bandVertexBuffer.Init(sizeof(Vertex) * bandVerts, sizeof(Vertex));
			m_bandIndexBuffer.Init(sizeof(uint16_t) * bandVerts, sizeof(uint16_t));

			std::vector<uint16_t> idx(bandVerts);
			for (int i = 0; i < bandVerts; ++i)
			{
				idx[i] = static_cast<uint16_t>(i);
			}
			m_bandIndexBuffer.Copy(idx.data());
		}


		void TerrainCircle::UpdateBand(const Vector3& center, float radius, float halfWidth, float alphaScale)
		{
			m_bandVertexCount = 0;
			if (m_bandSegments <= 0) return;

			const float innerRadius = (std::max)(radius - halfWidth, 1.0f);
			const float outerRadius = radius + halfWidth;
			const float angleStep = 2.0f * Math::PI / m_bandSegments;

			/** 帯中央の色（濃い）と縁の色（透明）。縁→中央のグラデーションになる */
			Vector4 midColor = m_bandColor;
			midColor.w *= alphaScale;
			Vector4 edgeColor = m_bandColor;
			edgeColor.w = 0.0f;

			/** 円周の各角度で、地表 Y は帯中央の1点だけサンプリングして内外にも使い回す。
			 *  （半幅ぶんの高低差は誤差として許容。レイキャスト数を1/3に抑える） */
			std::vector<Vector3> inner(m_bandSegments), mid(m_bandSegments), outer(m_bandSegments);
			for (int i = 0; i < m_bandSegments; ++i)
			{
				const float angle = i * angleStep;
				const float c = cosf(angle);
				const float s = sinf(angle);
				const float y = SampleSurfaceY(center.x + radius * c, center.z + radius * s) + HEIGHT_OFFSET;
				inner[i] = Vector3(center.x + innerRadius * c, y, center.z + innerRadius * s);
				mid[i]   = Vector3(center.x + radius      * c, y, center.z + radius      * s);
				outer[i] = Vector3(center.x + outerRadius * c, y, center.z + outerRadius * s);
			}

			for (int i = 0; i < m_bandSegments; ++i)
			{
				const int next = (i + 1) % m_bandSegments;

				/** 外側リング⇔中央リング */
				m_bandVerts[m_bandVertexCount++] = { outer[i],    edgeColor };
				m_bandVerts[m_bandVertexCount++] = { mid[i],      midColor };
				m_bandVerts[m_bandVertexCount++] = { outer[next], edgeColor };
				m_bandVerts[m_bandVertexCount++] = { outer[next], edgeColor };
				m_bandVerts[m_bandVertexCount++] = { mid[i],      midColor };
				m_bandVerts[m_bandVertexCount++] = { mid[next],   midColor };

				/** 中央リング⇔内側リング */
				m_bandVerts[m_bandVertexCount++] = { mid[i],      midColor };
				m_bandVerts[m_bandVertexCount++] = { inner[i],    edgeColor };
				m_bandVerts[m_bandVertexCount++] = { mid[next],   midColor };
				m_bandVerts[m_bandVertexCount++] = { mid[next],   midColor };
				m_bandVerts[m_bandVertexCount++] = { inner[i],    edgeColor };
				m_bandVerts[m_bandVertexCount++] = { inner[next], edgeColor };
			}
		}


		void TerrainCircle::RenderBand(RenderContext& rc)
		{
			if (m_bandVertexCount == 0) return;
			m_bandVertexBuffer.Copy(m_bandVerts.data());
			rc.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			rc.SetVertexBuffer(m_bandVertexBuffer);
			rc.SetIndexBuffer(m_bandIndexBuffer);
			rc.DrawIndexed(static_cast<UINT>(m_bandVertexCount));
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
