/**
 * @file ChildPenguinParameter.h
 * @brief 子ペンギンのパラメーター管理
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinParameter.h"
#include "ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンパラメーター
		 * @details 子ペンギン固有のパラメーターを保持する
		 */
		struct MasterChildPenguinParameter : public MasterPenguinParameter
		{
			appParameter(MasterChildPenguinParameter);

#ifdef APP_PARAM_HOT_RELOAD
			void Load(const nlohmann::json& j) override
			{
				load(j, *this);
			}
#endif // APP_PARAM_HOT_RELOAD

			// 子ペンギン固有のパラメーターをここに追加していく

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
				/** タイプ別乗算カラー */
				float colorR;
				float colorG;
				float colorB;
				float colorA;
				/** 速度系の個体差範囲 */
				Range runSpeed;
				Range swimSpeed;
				Range sneakSpeed;
				Range slideSpeed;
				Range jumpPower;
				/** 距離系の個体差範囲 */
				Range stopDistance;
				Range walkDistance;
				Range runDistance;
				Range joinDistance;
				Range giveUpDistance;
				/**
				 * @brief やんちゃペンギン固有：徘徊を開始する親との距離
				 * @details 待機命令中に親がこの距離以上離れたら徘徊を開始する
				 */
				Range roamTriggerDistance;
				/**
				 * @brief やんちゃペンギン固有：徘徊先を選ぶ半径
				 * @details 現在地からこの半径内のランダムな座標を徘徊先として選ぶ
				 */
				Range roamRadius;
				/**
				 * @brief おっちょこちょいペンギン固有：歩き・走り中の転倒確率（秒あたり）
				 * @details 毎フレーム確率判定を行い、転倒ステートへ遷移させる
				 */
				float tripChancePerSec;
				/**
				 * @brief おっちょこちょいペンギン固有：スライド解除時のスリップ確率
				 * @details スライド終了ステートへ遷移するタイミングで確率判定を行う
				 */
				float slipChance;
				/**
				 * @brief 世話焼きペンギン固有：介入対象を探す最大距離
				 * @details この距離より遠いペンギンには介入しない
				 */
				float interventionRange;
			};

			/** タイプ別パラメーター（インデックス = EnChildPenguinType の値） */
			std::array<ChildPenguinTypeData, static_cast<int>(EnChildPenguinType::Num)> typeData;
		};
	}
}