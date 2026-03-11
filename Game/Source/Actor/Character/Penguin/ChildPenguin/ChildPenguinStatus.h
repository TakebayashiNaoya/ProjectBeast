/**
 * @file ChildPenguinStatus.h
 * @brief 子ペンギンのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 子ペンギンのステータスクラス
		 */
		class ChildPenguinStatus : public CharacterStatus
		{
		public:
			// ここに親ペンギン固有のステータス用のゲッター関数を追加していく


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


			void Update() override;


		public:
			ChildPenguinStatus();
			~ChildPenguinStatus() override;


		private:
			// ここに子ペンギン固有のステータスを追加していく
		};
	}
}

