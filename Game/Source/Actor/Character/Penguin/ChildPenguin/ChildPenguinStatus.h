/**
 * @file ChildPenguinStatus.h
 * @brief 子ペンギンのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンのステータスクラス
		 */
		class ChildPenguinStatus : public PenguinStatus
		{
		public:
			// ここに子ペンギン固有のステータス用のゲッター関数を追加していく


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


			void Update() override;


			/**
			 * @brief 個体値を設定してロックする
			 * @details タイプ確定後に一度だけ呼び出す。
			 *          ロック後は Update() 内の Setup() 再呼び出しをスキップし、
			 *          ホットリロードによる上書きを防ぐ。
			 * @param walkSpeed  歩き速度の個体値
			 * @param runSpeed   走り速度の個体値
			 * @param swimSpeed  泳ぎ速度の個体値
			 * @param sneakSpeed スニーク速度の個体値
			 * @param slideSpeed スライド速度の個体値
			 * @param jumpPower  ジャンプパワーの個体値
			 */
			void SetIndividualValues(
				float walkSpeed,
				float runSpeed,
				float swimSpeed,
				float sneakSpeed,
				float slideSpeed,
				float jumpPower);


		public:
			ChildPenguinStatus();
			~ChildPenguinStatus() override;


		private:
			// ここに子ペンギン固有のステータスを追加していく

			/**
			 * @brief 個体値ロックフラグ
			 * @details true の間は Update() 内の Setup() 再呼び出しをスキップする
			 */
			bool m_isIndividualValueLocked = false;
		};
	}
}