/**
 * @file FormationFetchRule.h
 * @brief 散開陣の迎えで、野良の子と迎えに行く仲間の組を決める規則
 */
#pragma once
#include <algorithm>
#include <cfloat>
#include <vector>


namespace app
{
	namespace actor
	{
		/** 散開陣の迎えの組み合わせ（純関数。ChildPenguinManagerが使う） */
		namespace fetchRule
		{
			/** 野良の添字と、迎えに行く仲間の添字の組 */
			struct Pair
			{
				int wildIndex;
				int followerIndex;
			};


			/** @brief XZ平面での距離の2乗 */
			inline float DistanceSqXZ(const Vector3& a, const Vector3& b)
			{
				const float dx = a.x - b.x;
				const float dz = a.z - b.z;
				return dx * dx + dz * dz;
			}


			/**
			 * @brief 範囲内の野良を親に近い順に見て、いちばん近い空いた仲間を1体ずつ割り当てる
			 * @param daddyPos          親の位置
			 * @param radius            迎えの範囲（親からの距離）
			 * @param wildPositions     迎えの候補の野良の位置
			 * @param followerPositions 迎えに行ける仲間の位置
			 * @return 組の一覧。仲間が足りなくなったらそこで打ち切る
			 */
			inline std::vector<Pair> SelectPairs(
				const Vector3& daddyPos,
				const float radius,
				const std::vector<Vector3>& wildPositions,
				const std::vector<Vector3>& followerPositions)
			{
				std::vector<Pair> pairs;
				if (radius <= 0.0f) return pairs;

				const float radiusSq = radius * radius;
				std::vector<int> order;
				for (int i = 0; i < static_cast<int>(wildPositions.size()); i++)
				{
					if (DistanceSqXZ(wildPositions[i], daddyPos) <= radiusSq) order.push_back(i);
				}
				// 同じ距離なら添字の小さいほうを先にする
				std::stable_sort(order.begin(), order.end(), [&](int a, int b)
					{
						return DistanceSqXZ(wildPositions[a], daddyPos) < DistanceSqXZ(wildPositions[b], daddyPos);
					});

				std::vector<bool> used(followerPositions.size(), false);
				for (const int wild : order)
				{
					int best = -1;
					float bestSq = FLT_MAX;
					for (int f = 0; f < static_cast<int>(followerPositions.size()); f++)
					{
						if (used[f]) continue;
						const float distSq = DistanceSqXZ(followerPositions[f], wildPositions[wild]);
						if (distSq < bestSq)
						{
							bestSq = distSq;
							best = f;
						}
					}
					if (best < 0) break;
					used[best] = true;
					pairs.push_back({ wild, best });
				}
				return pairs;
			}


			/**
			 * @brief SelectPairsの約束を確かめる（Debugのロード時に呼ぶ。失敗するとK2_ASSERTで止まる）
			 * @return すべて通ればtrue
			 */
			inline bool SelfTest()
			{
				bool passed = true;
				auto check = [&passed](bool condition, const char* message)
					{
						K2_ASSERT(condition, message);
						passed = passed && condition;
					};
				const Vector3 daddy = Vector3::Zero;

				// 範囲外の野良は選ばない
				{
					const auto pairs = SelectPairs(daddy, 100.0f, { Vector3(150.0f, 0.0f, 0.0f) }, { Vector3(10.0f, 0.0f, 0.0f) });
					check(pairs.empty(), "fetchRule: 範囲外の野良を選んでいます");
				}
				// いちばん近い仲間を選ぶ
				{
					const auto pairs = SelectPairs(daddy, 300.0f, { Vector3(200.0f, 0.0f, 0.0f) },
						{ Vector3(-50.0f, 0.0f, 0.0f), Vector3(40.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 40.0f) });
					check(pairs.size() == 1 && pairs[0].followerIndex == 1, "fetchRule: いちばん近い仲間を選んでいません");
				}
				// 1体の仲間を2匹の野良に割り当てない
				{
					const auto pairs = SelectPairs(daddy, 300.0f, { Vector3(100.0f, 0.0f, 0.0f), Vector3(120.0f, 0.0f, 0.0f) },
						{ Vector3(30.0f, 0.0f, 0.0f), Vector3(-30.0f, 0.0f, 0.0f) });
					check(pairs.size() == 2 && pairs[0].followerIndex != pairs[1].followerIndex, "fetchRule: 1体の仲間を2匹に割り当てています");
				}
				// 親に近い野良から先に、近い仲間を取る
				{
					const auto pairs = SelectPairs(daddy, 300.0f, { Vector3(250.0f, 0.0f, 0.0f), Vector3(100.0f, 0.0f, 0.0f) },
						{ Vector3(40.0f, 0.0f, 0.0f) });
					check(pairs.size() == 1 && pairs[0].wildIndex == 1, "fetchRule: 親に近い野良を先にしていません");
				}
				// 仲間が足りなければ打ち切る。上限は設けない
				{
					const auto pairs = SelectPairs(daddy, 300.0f,
						{ Vector3(100.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 100.0f), Vector3(-100.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, -100.0f) },
						{ Vector3(10.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 10.0f), Vector3(-10.0f, 0.0f, 0.0f) });
					check(pairs.size() == 3, "fetchRule: 仲間の数だけ組を作っていません");
				}
				// 仲間がいなければ空
				{
					const auto pairs = SelectPairs(daddy, 300.0f, { Vector3(100.0f, 0.0f, 0.0f) }, {});
					check(pairs.empty(), "fetchRule: 仲間がいないのに組を作っています");
				}
				// 範囲が0なら空
				{
					const auto pairs = SelectPairs(daddy, 0.0f, { Vector3(0.0f, 0.0f, 0.0f) }, { Vector3(10.0f, 0.0f, 0.0f) });
					check(pairs.empty(), "fetchRule: 範囲0で組を作っています");
				}
				// 高さは無視する
				{
					const auto pairs = SelectPairs(daddy, 100.0f, { Vector3(50.0f, 500.0f, 0.0f) },
						{ Vector3(60.0f, -500.0f, 0.0f), Vector3(0.0f, 500.0f, 0.0f) });
					check(pairs.size() == 1 && pairs[0].followerIndex == 0, "fetchRule: 高さを距離に入れています");
				}
				// 同じ距離なら添字の小さい仲間を選ぶ
				{
					const auto pairs = SelectPairs(daddy, 300.0f, { Vector3(100.0f, 0.0f, 0.0f) },
						{ Vector3(100.0f, 0.0f, 20.0f), Vector3(100.0f, 0.0f, -20.0f) });
					check(pairs.size() == 1 && pairs[0].followerIndex == 0, "fetchRule: 同じ距離のとき添字の小さい仲間を選んでいません");
				}
				return passed;
			}
		}
	}
}
