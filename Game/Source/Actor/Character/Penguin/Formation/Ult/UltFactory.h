/**
 * @file UltFactory.h
 * @brief 陣形ごとのウルト効果を組み立てるファクトリー
 * @author 竹林
 */
#pragma once
#include <memory>
#include "IUltEffect.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形ウルトのファクトリー
		 * @details
		 *   各陣形のコンストラクタから呼び出し、デコレーターチェーンを生成する。
		 *   効果の追加・変更はここだけ修正すればよい。
		 *
		 *   各陣形の構成:
		 *   - 円陣    : 速度30%UP ＋ 渦潮免疫 ＋ 距離250以内のペンギン呼び出し
		 *   - 防御陣形 : 渦潮近傍で速度50%UP ＋ シロクマ攻撃無効化
		 *   - 三角陣  : 速度80%UP（純粋なスピード特化）
		 *   - 散開陣  : 距離600以内のペンギン呼び出し（広範囲回収特化）
		 */
		class UltFactory
		{
		public:
			/**
			 * @brief 円陣のウルト効果を生成する
			 */
			static std::unique_ptr<IUltEffect> CreateCircleUlt();
			/**
			 * @brief 防御陣形のウルト効果を生成する
			 */
			static std::unique_ptr<IUltEffect> CreateDefenseUlt();
			/**
			 * @brief 散開陣のウルト効果を生成する
			 */
			static std::unique_ptr<IUltEffect> CreateScatterUlt();
			/**
			 * @brief 三角陣のウルト効果を生成する
			 */
			static std::unique_ptr<IUltEffect> CreateTriangleUlt();
		};
	}
}
