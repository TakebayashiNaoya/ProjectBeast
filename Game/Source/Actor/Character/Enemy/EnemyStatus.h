/**
 * @file EnemyStatus.h
 * @brief エネミーのステータス
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief エネミーのステータスクラス
		 */
		class EnemyStatus :public CharacterStatus
		{
		public:
			// ここにエネミー固有のステータス用のゲッター関数を追加していく


		public:
			/**
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


			void Update() override;


		public:
			EnemyStatus();
			~EnemyStatus() override;


		private:
			// ここにエネミー固有のステータスを追加していく
		};
	}
}

