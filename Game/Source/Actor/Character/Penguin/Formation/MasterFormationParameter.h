/**
 * @file MasterFormationParameter.h
 * @brief 陣形・ウルトのパラメーター管理クラス
 * @author 竹林
 */
#pragma once
#include "Source/Core/IMasterParameter.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形・ウルトのパラメーター管理クラス
		 * @details インデックスは EnFormationType の値と対応する
		 */
		struct MasterFormationParameter : public core::IMasterParameter
		{
			appParameter(MasterFormationParameter);
#if defined(APP_PARAM_HOT_RELOAD)
			void Load(const nlohmann::json& j)override
			{
				load(j, *this);
			}

			void Load(std::istream& stream) override
			{
				constexpr size_t BASE_SIZE = sizeof(core::IMasterParameter);
				constexpr size_t DATA_SIZE = sizeof(MasterFormationParameter) - BASE_SIZE;
				stream.read(reinterpret_cast<char*>(this) + BASE_SIZE, DATA_SIZE);
			}
#endif
			/** JSONから受け取る変数群（バイナリ変換の都合上、float→intの順に並べる） */

			/** ウルト持続時間（秒） */
			float ultDuration;
			/** ウルトクールダウン（秒） */
			float ultCooldown;
			/** パッシブ: 固定速度倍率（未使用、全陣形とも passiveSpeedBase/PerLevel のレベル連動速度に統一） */
			float passiveSpeedMultiplier;
			/** パッシブ: レベル0の速度倍率（全陣形で使用） */
			float passiveSpeedBase;
			/** パッシブ: レベルごとの速度増分（全陣形で共通の値を使用） */
			float passiveSpeedPerLevel;
			/** ウルト: 固定速度倍率（未使用の陣形は1.0） */
			float ultSpeedMultiplier;
			/** ウルト: 渦潮近傍時の速度倍率（密集陣のみ使用） */
			float ultWhirlpoolBoostMultiplier;
			/** ウルト: ペンギン呼び戻し距離（未使用なら0） */
			float ultCallDistance;
			/** 形状: リングの半径増分（RingFormation系のみ使用。陣形の広さを決める） */
			float radiusPerRing;
			/** 形状: 入隊判定マージン（最外半径に足す。全陣形共通） */
			float joinMargin;
			/** 形状: 三角陣の行間隔（三角陣のみ使用） */
			float rowSpacing;
			/** 形状: 三角陣の列間隔（三角陣のみ使用） */
			float colSpacing;

			/** パッシブ: 渦潮耐性を持つか（0/1） */
			int passiveWhirlpoolResistance;
			/** ウルト: 渦潮免疫を持つか（0/1） */
			int ultWhirlpoolResistance;
			/** ウルト: シロクマ攻撃を無効化するか（0/1） */
			int ultBearAttackNullify;
			/** 形状: リング1の配置数（RingFormation系のみ使用。リング k は baseFollowers*k 体） */
			int baseFollowers;
		};
	}
}
