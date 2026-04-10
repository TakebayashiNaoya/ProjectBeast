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
			WhirlpoolManager();
			virtual ~WhirlpoolManager() override = default;


		public:
			/**
			 * @brief 渦潮に対してコールバック関数を実行する関数
			 * @param cb コールバック関数
			 */
			void ForEach(std::function<void(Whirlpool* info)> cb);


		private:
			/**
			 * @brief 渦潮を生成する関数
			 */
			void CreateWhirlpool();


		private:
			/** 渦潮のマップ */
			std::unordered_map<uint8_t, Whirlpool*> m_whirlpoolMap;
			/** 渦潮の生成タイマー */
			float m_timer;


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
			/** インスタンス */
			static WhirlpoolManager* m_instance;
		};
	}
}

