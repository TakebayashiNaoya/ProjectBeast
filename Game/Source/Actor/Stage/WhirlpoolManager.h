/**
 * @file WhirlpoolManager.h
 * @brief 渦潮を管理するクラス
 * @author 藤谷
 */
#pragma once
#include "IStage.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Whirlpool;

		/**
		 * @brief 渦潮を管理するクラス
		 */
		class WhirlpoolManager : public Actor
		{
		public:
			void Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		private:
			WhirlpoolManager() = default;
			virtual ~WhirlpoolManager() override = default;


		public:
			void ForEach(std::function<void(Whirlpool*)> cb);


		private:
			std::vector<Whirlpool*> m_whirlpools;


		public:
			/**
			 * @brief インスタンスの生成
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new WhirlpoolManager();
				}
			}

			/**
			 * @brief インスタンスの取得
			 * @return インスタンスのポインタ
			 */
			static WhirlpoolManager* GetInstance()
			{
				return m_instance;
			}


			/**
			 * @brief インスタンスの破棄
			 */
			static void DestroyInstance()
			{
				delete m_instance;
			}


		private:
			static WhirlpoolManager* m_instance;
		};
	}
}

