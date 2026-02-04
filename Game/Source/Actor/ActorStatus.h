/**
 * @file ActorStatus.h
 * @brief アクターのステータス基底クラス
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace actor
	{
		/**
		 * @brief アクターのステータス基底クラス
		 */
		class ActorStatus
		{
		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			virtual void Setup() = 0;


		public:
			ActorStatus() = default;
			virtual ~ActorStatus() = default;
		};
	}
}

