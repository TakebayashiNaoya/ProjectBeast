#pragma once




namespace app
{
	namespace actor
	{
		class ActorStatus
		{
		public:
			ActorStatus() = default;
			virtual ~ActorStatus() = default;


		public:
			/*
			 * セットアップ
			 * ステータスの持ち主が必ず呼び出すこと
			 */
			virtual void Setup() = 0;
		};
	}
}

