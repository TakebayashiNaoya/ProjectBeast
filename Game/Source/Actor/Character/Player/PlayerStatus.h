/**
 * @file PlayerStatus.h
 * @brief プレイヤーのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief プレイヤーのステータスクラス
		 */
		class PlayerStatus : public CharacterStatus
		{
		public:
			// ここにプレイヤー固有のステータス用のゲッター関数を追加していく


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


		public:
			PlayerStatus() = default;
			~PlayerStatus() override = default;


		private:
			// ここにプレイヤー固有のステータスを追加していく
		};
	}
}

