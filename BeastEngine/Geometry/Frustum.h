/**
 * @file Frustum.h
 * @brief 視錐台（フラスタム）クラス
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
#if defined(_DEBUG)
		/**
		 * @brief デバッグ用フラスタム縮小スケール
		 * @details
		 *   1.0f のとき通常（画面端 = フラスタム境界）。
		 *   1.0f より大きくするとプロジェクション行列の
		 *   XY スケール成分が拡大され、フラスタムの左右・上下平面が
		 *   画面内側に寄る。境界が画面に映るため動作確認に使用する。
		 *   確認が終わったら 1.0f に戻すこと。
		 */
		static constexpr float DEBUG_FRUSTUM_SHRINK_SCALE = 1.0f;
#endif


	public:
		/**
		 * @brief 視錐台を更新する
		 * @details
		 *   ビュープロジェクション行列から6平面を抽出する。
		 *   RenderingEngine::Execute()の先頭で毎フレーム呼ぶこと。
		 * @param viewProjMatrix ビュープロジェクション行列
		 * @param screenShrinkScale
		 *   プロジェクション行列の XY スケールに乗算する係数。
		 *   1.0f で通常サイズ、大きいほどフラスタムが画面内側に縮小する。
		 *   デバッグ用途以外では 1.0f を渡すこと。
		 */
		void Update(const Matrix& viewProjMatrix, float screenShrinkScale = 1.0f);

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

		/**
		 * @brief 点が視錐台の内側にあるか判定する
		 * @details
		 *   6平面すべての内側（符号付き距離 >= 0）にある場合のみ内側と判定する。
		 *   TriangleCuller から頂点単位の判定に使用する。
		 * @param point 判定する点（ワールド座標）
		 * @return 視錐台の内側にあればtrue
		 */
		bool IsPointInside(const Vector3& point) const;

		/**
		 * @brief 平面の数を取得する
		 * @return 平面の数（常に6）
		 */
		int GetPlaneCount() const { return PLANE_NUM; }

		/**
		 * @brief 指定インデックスの平面係数を取得する
		 * @details TriangleCuller の分離軸判定で使用する。
		 * @param index 平面インデックス（0〜5）
		 * @param outA  平面法線X成分の出力先
		 * @param outB  平面法線Y成分の出力先
		 * @param outC  平面法線Z成分の出力先
		 * @param outD  平面距離成分の出力先
		 */
		void GetPlane(int index, float& outA, float& outB, float& outC, float& outD) const
		{
			outA = m_planes[index].a;
			outB = m_planes[index].b;
			outC = m_planes[index].c;
			outD = m_planes[index].d;
		}


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