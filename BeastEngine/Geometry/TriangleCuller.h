/**
 * @file TriangleCuller.h
 * @brief トライアングルカリングクラス
 */
#pragma once
#include "Geometry/Frustum.h"


namespace nsBeastEngine
{
	/**
	 * @brief トライアングルカリングクラス
	 * @details
	 *   ワールド座標変換済みの頂点配列とインデックス配列を受け取り、
	 *   視錐台と交差する三角形のインデックスのみを出力する。
	 *   3頂点のいずれか1頂点でも視錐台の内側にあれば可視と判定する（保守的判定）。
	 *   BeastMeshParts::Draw() および OceanMesh::Draw() から呼ばれる。
	 */
	class TriangleCuller
	{
	public:
		/**
		 * @brief 視錐台に対してトライアングルカリングを行う
		 * @details
		 *   srcIndices を3つずつ取り出して三角形を構成し、
		 *   各頂点が視錐台の内側にあるかどうかを判定する。
		 *   1頂点でも内側であれば可視三角形として outVisibleIndices に追加する。
		 *   outVisibleIndices は呼び出し前に clear() されること。
		 * @param worldVertices		ワールド座標変換済みの頂点位置配列
		 * @param numVertices		頂点の数
		 * @param srcIndices		元のインデックス配列（要素数は numIndices）
		 * @param numIndices		インデックスの数（3の倍数であること）
		 * @param frustum			判定に使用する視錐台
		 * @param outVisibleIndices	可視三角形のインデックスの出力先
		 */
		void Cull(
			const Vector3* worldVertices,
			int numVertices,
			const uint32_t* srcIndices,
			int numIndices,
			const Frustum& frustum,
			std::vector<uint32_t>& outVisibleIndices
		) const;
	};
}