/**
 * @file CPReactionTypes.h
 * @brief 子ペンギンのリアクション関連の型定義
 * @author 藤谷
 */
#pragma once

namespace app
{
	namespace ui
	{
		/**
		 * @brief リアクションのタイプ
		 */
		enum class EnCPReactionType : uint8_t
		{
			Trouble,
			Happy,
			None
		};


		/**
		 * @brief リアクション通知の優先度
		 * @detail 同一ターゲットに対して同フレーム内で複数回通知された場合、
		 *         優先度の低い通知は既に表示中の高い優先度のリアクションを上書きしない。
		 *         （NotifyCPReactionChanged/SetTargetの呼び出し順序に結果が依存しないようにするための仕組み）
		 *         優先度が同じ場合は、単純に後から呼ばれた方が反映される（後勝ち）。
		 */
		enum class EnCPReactionPriority : uint8_t
		{
			Normal,	///< 通常。隊列の入退場（AddFollower/RemoveFollower）など、汎用的な通知に使う
			High	///< 高優先度。特定の行動決定など、狙って出す通知に使う
		};
	}
}
