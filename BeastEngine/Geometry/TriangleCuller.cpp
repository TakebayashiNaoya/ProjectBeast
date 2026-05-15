/**
 * @file TriangleCuller.cpp
 * @brief トライアングルカリングクラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "Geometry/TriangleCuller.h"


namespace nsBeastEngine
{
	void TriangleCuller::Cull(
		const Vector3* worldVertices,
		int numVertices,
		const uint32_t* srcIndices,
		int numIndices,
		const Frustum& frustum,
		std::vector<uint32_t>& outVisibleIndices
	) const
	{
		// インデックス数は3の倍数であること
		const int numTriangles = numIndices / 3;
		const int numPlanes = frustum.GetPlaneCount();

		for (int triNo = 0; triNo < numTriangles; triNo++)
		{
			const uint32_t i0 = srcIndices[triNo * 3 + 0];
			const uint32_t i1 = srcIndices[triNo * 3 + 1];
			const uint32_t i2 = srcIndices[triNo * 3 + 2];

			// インデックスが頂点配列の範囲外なら三角形をスキップする
			if (static_cast<int>(i0) >= numVertices ||
				static_cast<int>(i1) >= numVertices ||
				static_cast<int>(i2) >= numVertices)
			{
				continue;
			}

			const Vector3& v0 = worldVertices[i0];
			const Vector3& v1 = worldVertices[i1];
			const Vector3& v2 = worldVertices[i2];

			// 分離軸判定:
			// ある平面に対して3頂点すべてが外側にある場合のみカリングする。
			// 1頂点でも内側または境界上にあれば、その平面では分離できない。
			// 全6平面で分離できなければ可視と判定する（保守的）。
			// これにより、頂点が視錐台の外にあっても三角形が
			// 視錐台をまたいでいるケースを正しく可視と判定できる。
			bool isCulled = false;
			for (int planeNo = 0; planeNo < numPlanes; planeNo++)
			{
				float a, b, c, d;
				frustum.GetPlane(planeNo, a, b, c, d);

				// 各頂点の符号付き距離を計算する
				const float dist0 = a * v0.x + b * v0.y + c * v0.z + d;
				const float dist1 = a * v1.x + b * v1.y + c * v1.z + d;
				const float dist2 = a * v2.x + b * v2.y + c * v2.z + d;

				// 3頂点すべてが負（平面の外側）ならこの平面で分離できる → カリング
				if (dist0 < 0.0f && dist1 < 0.0f && dist2 < 0.0f)
				{
					isCulled = true;
					break;
				}
			}

			if (!isCulled)
			{
				outVisibleIndices.push_back(i0);
				outVisibleIndices.push_back(i1);
				outVisibleIndices.push_back(i2);
			}
		}
	}
}