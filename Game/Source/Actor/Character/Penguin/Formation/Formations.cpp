/**
 * @file Formations.cpp
 * @brief 陣形インターフェースと全具体クラスの実装
 * @author 竹林
 */
#include "stdafx.h"
#include "Formations.h"
#include "Passive/PassiveFactory.h"
#include "Ult/UltFactory.h"


namespace app
{
	namespace actor
	{
		void RingFormation::CalculatePositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int count
		)
		{
			m_outerRadius = 0.0f;
			if (count == 0) return;

			// 前方を基準角度にする（最初の1体が常に正面に来る）
			Vector3 fwd(forward.x, 0.0f, forward.z);
			if (fwd.x * fwd.x + fwd.z * fwd.z < 0.0001f) {
				fwd = Vector3(0.0f, 0.0f, 1.0f);
			}
			else {
				fwd.Normalize();
			}

			// 最初の1体が正面に来るように、前方ベクトルから角度を求める
			const float startAngle = atan2f(fwd.x, fwd.z);
			// 配置した数
			int placed = 0;

			// リング ring (1始まり): m_baseFollowers*ring 体を半径 m_radiusPerRing*ring に等間隔配置
			for (int ring = 1; placed < count; ++ring)
			{
				// リングの配置数と半径
				const int   ringCount = m_baseFollowers * ring;
				// リングの半径
				const float radius    = m_radiusPerRing * ring;
				// 配置を決定
				for (int i = 0; i < ringCount && placed < count; ++i)
				{
					const float angle = startAngle + (float)i / ringCount * 2.0f * Math::PI;
					Vector3 pos = center;
					pos.x += radius * sinf(angle);
					pos.z += radius * cosf(angle);
					out.push_back(pos);
					m_outerRadius = max(m_outerRadius, radius);
					++placed;
				}
			}
		}


		

		/****************************************/


		CircleFormation::CircleFormation() : RingFormation(9, 22.0f)
		{
			m_passive = PassiveFactory::CreateCirclePassive();
			m_ult     = UltFactory::CreateCircleUlt();
		}


		

		/****************************************/


		DefenseFormation::DefenseFormation() : RingFormation(9, 8.0f)
		{
			m_joinMargin = 10.0f;
			m_passive    = PassiveFactory::CreateDefensePassive();
			m_ult        = UltFactory::CreateDefenseUlt();
		}


		

		/****************************************/


		ScatterFormation::ScatterFormation() : RingFormation(9, 40.0f)
		{
			m_joinMargin = 50.0f;
			m_passive    = PassiveFactory::CreateScatterPassive();
			m_ult        = UltFactory::CreateScatterUlt();
		}


		

		/****************************************/


		TriangleFormation::TriangleFormation()
		{
			m_joinMargin = 15.0f;
			m_passive    = PassiveFactory::CreateTrianglePassive();
			m_ult        = UltFactory::CreateTriangleUlt();
		}


		void TriangleFormation::CalculatePositions(
			const Vector3& center,
			const Vector3& forward,
			std::vector<Vector3>& out,
			int count
		)
		{
			m_outerRadius = 0.0f;
			if (count == 0) return;

			// XZ 平面上の前方・右方ベクトル（正規化）
			Vector3 fwd(forward.x, 0.0f, forward.z);
			Vector3 right;
			if (fwd.x * fwd.x + fwd.z * fwd.z < 0.0001f)
			{
				fwd   = Vector3(0.0f, 0.0f, 1.0f);
				right = Vector3(1.0f, 0.0f, 0.0f);
			}
			else
			{
				fwd.Normalize();
				right = Vector3(fwd.z, 0.0f, -fwd.x);
			}

			const float ROW = ROW_SPACING;
			const float COL = COL_SPACING;
			int placed = 0;

			// 座標追加ヘルパー: count 体に達したら false を返す
			auto Add = [&](float fwdDist, float rightDist) -> bool
			{
				if (placed >= count) return false;
				Vector3 pos = center;
				pos.x += fwd.x * fwdDist + right.x * rightDist;
				pos.z += fwd.z * fwdDist + right.z * rightDist;
				out.push_back(pos);
				Vector3 diff = pos - center;
				diff.y = 0.0f;
				m_outerRadius = max(m_outerRadius, diff.Length());
				++placed;
				return true;
			};

			// ── レベル 1: 4行三角形（プレイヤー = ボーリング5番ピン位置）──
			//   k=+2: 先頭, k=+1: 左上・右上, k=0: 左・右, k=-1: 後方4体
			if (!Add( 2.0f * ROW,  0.0f		 )) return;  // ① 先頭
			if (!Add( 1.0f * ROW, -0.5f * COL)) return;  // ② 左上
			if (!Add( 1.0f * ROW,  0.5f * COL)) return;  // ③ 右上
			if (!Add( 0.0f		, -1.0f * COL)) return;  // ④ 左
			if (!Add( 0.0f		,  1.0f * COL)) return;  // ⑤ 右
			if (!Add(-1.0f * ROW, -1.5f * COL)) return;  // ⑥ 左下
			if (!Add(-1.0f * ROW, -0.5f * COL)) return;  // ⑦ 真後ろ左
			if (!Add(-1.0f * ROW,  0.5f * COL)) return;  // ⑧ 真後ろ右
			if (!Add(-1.0f * ROW,  1.5f * COL)) return;  // ⑨ 右下

			// ── レベル L (L≥2) 拡張: 三角形を外周1層ずつ広げる ──
			//
			// レベル L の三角形: 行数 = 3L+1, 総ピン数 = (3L+1)*(3L+2)/2
			// プレイヤー行は常に k=0, 三角形の行は k = +2L (頂点) 〜 k = -L (底辺)
			//
			// 拡張手順 (レベル L-1 → L):
			//   1. 新頂点    : k = +2L (1体)
			//   2. 頂点2行目 : k = +2L-1 (2体、左→右)
			//   3. 既存行    : k = +2(L-1) 〜 -(L-1) の各行に外縁ペアを左→右の順で追加
			//      行 k の新外縁横距離 = (2L - k) / 2 * COL
			//   4. 新底辺    : k = -L (3L+1体、左→右)
			//
			for (int L = 2; placed < count; ++L)
			{
				// 1. 新頂点
				if (!Add(+2.0f * L * ROW, 0.0f)) return;

				// 2. 頂点から2行目 (k = 2L-1): 2体
				if (!Add(+(2.0f * L - 1.0f) * ROW, -0.5f * COL)) return;
				if (!Add(+(2.0f * L - 1.0f) * ROW, +0.5f * COL)) return;

				// 3. 既存行 (k = 2(L-1) 〜 -(L-1)): 外縁ペア（左→右）
				for (int k = 2 * (L - 1); k >= -(L - 1); --k)
				{
					const float outerRight = (2.0f * L - k) * 0.5f * COL;
					if (!Add((float)k * ROW, -outerRight)) return;
					if (!Add((float)k * ROW, +outerRight)) return;
				}

				// 4. 新底辺 (k = -L): 3L+1体、左から右へ
				const int   nBottom     = 3 * L + 1;
				const float bottomFwd   = -(float)L * ROW;
				const float bottomStart = -(float)(3 * L) * 0.5f * COL;
				for (int i = 0; i < nBottom; ++i)
				{
					if (!Add(bottomFwd, bottomStart + i * COL)) return;
				}
			}
		}
	}
}
