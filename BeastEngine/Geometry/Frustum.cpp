/**
 * @file Frustum.cpp
 * @brief 視錐台（フラスタム）クラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "Geometry/Frustum.h"


namespace nsBeastEngine
{
	void Frustum::Update(const Matrix& viewProjMatrix)
	{
		// Gribb/Hartmann 法でビュープロジェクション行列から6平面を抽出する
		// 行列の各行を m[row][col] で参照する
		const float* m = &viewProjMatrix.m[0][0];

		// 左平面: col0 + col3
		m_planes[enPlane_Left].a = m[3] + m[0];
		m_planes[enPlane_Left].b = m[7] + m[4];
		m_planes[enPlane_Left].c = m[11] + m[8];
		m_planes[enPlane_Left].d = m[15] + m[12];

		// 右平面: col3 - col0
		m_planes[enPlane_Right].a = m[3] - m[0];
		m_planes[enPlane_Right].b = m[7] - m[4];
		m_planes[enPlane_Right].c = m[11] - m[8];
		m_planes[enPlane_Right].d = m[15] - m[12];

		// 上平面: col3 - col1
		m_planes[enPlane_Top].a = m[3] - m[1];
		m_planes[enPlane_Top].b = m[7] - m[5];
		m_planes[enPlane_Top].c = m[11] - m[9];
		m_planes[enPlane_Top].d = m[15] - m[13];

		// 下平面: col3 + col1
		m_planes[enPlane_Bottom].a = m[3] + m[1];
		m_planes[enPlane_Bottom].b = m[7] + m[5];
		m_planes[enPlane_Bottom].c = m[11] + m[9];
		m_planes[enPlane_Bottom].d = m[15] + m[13];

		// 近平面: col2
		m_planes[enPlane_Near].a = m[2];
		m_planes[enPlane_Near].b = m[6];
		m_planes[enPlane_Near].c = m[10];
		m_planes[enPlane_Near].d = m[14];

		// 遠平面: col3 - col2
		m_planes[enPlane_Far].a = m[3] - m[2];
		m_planes[enPlane_Far].b = m[7] - m[6];
		m_planes[enPlane_Far].c = m[11] - m[10];
		m_planes[enPlane_Far].d = m[15] - m[14];

		// 全平面を正規化する
		for (int i = 0; i < PLANE_NUM; i++)
		{
			NormalizePlane(m_planes[i]);
		}
	}


	bool Frustum::IsIntersectAABB(AABB& aabb, const Matrix& mWorld) const
	{
		// AABBの8頂点をワールド空間に変換する
		Vector3 worldVertices[8];
		aabb.CalcVertexPositions(worldVertices, mWorld);

		// 各平面に対して判定する
		for (int planeNo = 0; planeNo < PLANE_NUM; planeNo++)
		{
			const SPlane& plane = m_planes[planeNo];

			// 8頂点のうち1つでも平面の内側にあれば、この平面ではカリングしない
			bool isAnyInside = false;
			for (int vertNo = 0; vertNo < 8; vertNo++)
			{
				const float dist = plane.a * worldVertices[vertNo].x
					+ plane.b * worldVertices[vertNo].y
					+ plane.c * worldVertices[vertNo].z
					+ plane.d;

				if (dist >= 0.0f)
				{
					isAnyInside = true;
					break;
				}
			}

			// 全頂点がこの平面の外側にある → 視錐台の外
			if (!isAnyInside)
			{
				return false;
			}
		}

		return true;
	}


	bool Frustum::IsIntersectAABBWorld(const Vector3& min, const Vector3& max) const
	{
		// 各平面に対して判定する
		for (int planeNo = 0; planeNo < PLANE_NUM; planeNo++)
		{
			const SPlane& plane = m_planes[planeNo];

			// 法線方向に最も「内側」にある頂点（p-vertex）を選ぶ
			// 各軸ごとに法線の符号に応じて min か max を選択する
			Vector3 positiveVertex;
			positiveVertex.x = (plane.a >= 0.0f) ? max.x : min.x;
			positiveVertex.y = (plane.b >= 0.0f) ? max.y : min.y;
			positiveVertex.z = (plane.c >= 0.0f) ? max.z : min.z;

			const float dist = plane.a * positiveVertex.x
				+ plane.b * positiveVertex.y
				+ plane.c * positiveVertex.z
				+ plane.d;

			// 最も内側の頂点すら平面の外側にある → 視錐台の外
			if (dist < 0.0f)
			{
				return false;
			}
		}

		return true;
	}


	bool Frustum::IsIntersectSphere(const Vector3& center, float radius) const
	{
		for (int planeNo = 0; planeNo < PLANE_NUM; planeNo++)
		{
			const SPlane& plane = m_planes[planeNo];

			const float dist = plane.a * center.x
				+ plane.b * center.y
				+ plane.c * center.z
				+ plane.d;

			// 符号付き距離がマイナス側に半径以上あれば、球は完全に外側
			if (dist < -radius)
			{
				return false;
			}
		}

		return true;
	}


	bool Frustum::IsPointInside(const Vector3& point) const
	{
		// 6平面すべての内側にある場合のみ内側と判定する
		for (int planeNo = 0; planeNo < PLANE_NUM; planeNo++)
		{
			const SPlane& plane = m_planes[planeNo];

			const float dist = plane.a * point.x
				+ plane.b * point.y
				+ plane.c * point.z
				+ plane.d;

			// 1平面でも外側にあれば視錐台の外
			if (dist < 0.0f)
			{
				return false;
			}
		}

		return true;
	}


	void Frustum::NormalizePlane(SPlane& plane)
	{
		const float length = sqrtf(
			plane.a * plane.a +
			plane.b * plane.b +
			plane.c * plane.c
		);

		if (length > FLT_EPSILON)
		{
			const float invLength = 1.0f / length;
			plane.a *= invLength;
			plane.b *= invLength;
			plane.c *= invLength;
			plane.d *= invLength;
		}
	}
}