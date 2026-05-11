/**
 * @file Frustum.h
 * @brief 視錐台（フラスタム）クラス
 * @author 竹林
 */
#pragma once


namespace nsBeastEngine
{
	/**
	 * @brief 視錐台（フラスタム）クラス
	 * @details
	 *   ビュープロジェクション行列から6平面を抽出し、
	 *   AABBや球との交差判定を行う。
	 *   RenderingEngine::Execute()の先頭でUpdate()を呼ぶことで
	 *   毎フレーム最新の視錐台に更新される。
	 */
	class Frustum
	{
	private:
		/**
		 * @brief 平面を表す構造体
		 * @details 法線 (a, b, c) と距離 d で平面 ax + by + cz + d = 0 を表す
		 */
		struct SPlane
		{
			float a = 0.0f;
			float b = 0.0f;
			float c = 0.0f;
			float d = 0.0f;
		};

		/** 視錐台を構成する平面の数 */
		static constexpr int PLANE_NUM = 6;

		/** 平面のインデックス */
		enum EnPlane
		{
			enPlane_Left = 0,	/** 左平面   */
			enPlane_Right,		/** 右平面   */
			enPlane_Top,		/** 上平面   */
			enPlane_Bottom,		/** 下平面   */
			enPlane_Near,		/** 近平面   */
			enPlane_Far,		/** 遠平面   */
		};


	public:
		/**
		 * @brief 視錐台を更新する
		 * @details
		 *   ビュープロジェクション行列から6平面を抽出する。
		 *   RenderingEngine::Execute()の先頭で毎フレーム呼ぶこと。
		 * @param viewProjMatrix ビュープロジェクション行列
		 */
		void Update(const Matrix& viewProjMatrix);

		/**
		 * @brief AABBが視錐台と交差しているか判定する
		 * @details
		 *   AABBの8頂点をワールド空間で計算し、
		 *   6平面すべての外側にある頂点が存在すれば交差なしと判定する。
		 *   スケルトンなし ModelRender の判定に使用する。
		 * @param aabb    ローカル空間のAABB
		 * @param mWorld  ワールド行列
		 * @return 交差していればtrue
		 */
		bool IsIntersectAABB(AABB& aabb, const Matrix& mWorld) const;

		/**
		 * @brief ワールド空間のAABBが視錐台と交差しているか判定する
		 * @details
		 *   スケルトンあり ModelRender のボーンAABBなど、
		 *   すでにワールド空間に変換済みのAABBに使用する。
		 * @param min ワールド空間のAABB最小点
		 * @param max ワールド空間のAABB最大点
		 * @return 交差していればtrue
		 */
		bool IsIntersectAABBWorld(const Vector3& min, const Vector3& max) const;

		/**
		 * @brief 球が視錐台と交差しているか判定する
		 * @details Whirlpool のカリング判定に使用する。
		 * @param center 球の中心（ワールド座標）
		 * @param radius 球の半径
		 * @return 交差していればtrue
		 */
		bool IsIntersectSphere(const Vector3& center, float radius) const;


	private:
		/**
		 * @brief 平面を正規化する
		 * @param plane 正規化する平面
		 */
		void NormalizePlane(SPlane& plane);


	private:
		/** 視錐台の6平面 */
		SPlane m_planes[PLANE_NUM];
	};
}