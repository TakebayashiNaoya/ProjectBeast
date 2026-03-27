/**
 * @file ChildPenguinParameter.h
 * @brief 子ペンギンのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterParameter.h"
#include "ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンパラメーター
		 * @details 子ペンギン固有のパラメーターを保持する
		 */
		struct MasterChildPenguinParameter : public MasterCharacterParameter
		{
			appParameter(MasterChildPenguinParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// 子ペンギン固有のパラメーターをここに追加していく
			float sneakSpeed;
			float slideSpeed;
			float jumpPower;

			/**
			 * @brief min/max の範囲を表す構造体
			 */
			struct Range
			{
				float min;
				float max;
			};

			/**
			 * @brief タイプごとの個体差パラメーター範囲
			 * @details インデックスは EnChildPenguinType の値と対応する
			 */
			struct ChildPenguinTypeData
			{
				// タイプ別乗算カラー
				float colorR;
				float colorG;
				float colorB;
				float colorA;
				// 速度系の個体差範囲
				Range walkSpeed;
				Range runSpeed;
				Range swimSpeed;
				Range sneakSpeed;
				Range slideSpeed;
				Range jumpPower;
				// 距離系の個体差範囲
				Range stopDistance;
				Range walkDistance;
				Range runDistance;
				Range joinDistance;
				Range giveUpDistance;
				Range breakAwayDistance;
			};

			/** タイプ別パラメーター（インデックス = EnChildPenguinType の値） */
			std::array<ChildPenguinTypeData, static_cast<int>(EnChildPenguinType::Num)> typeData;
		};
	}
}