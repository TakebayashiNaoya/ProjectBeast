/**
 * @file RandomDevice.h
 * @brief ランダムデバイスの定義
 */
#pragma once
#include <random>


namespace app
{
	namespace util
	{
		/**
		 * @brief ランダムデバイス
		 */
		class RandomDevice
		{
		public:
			/**
			 * @brief 乱数生成器を取得する
			 * @return 乱数生成器
			 */
			static std::mt19937& GetEngine()
			{
				return m_randomEngine;
			}
			/**
			 * @brief 指定した範囲で乱数を生成する
			 * @param min 最小値
			 * @param max 最大値
			 * @return min～maxの範囲で乱数を返す
			 */
			static int Random(const int min, const int max)
			{
				std::uniform_int_distribution<int> dist(min, max);
				return dist(m_randomEngine);
			}
			/**
			 * @brief 指定した範囲で乱数を生成する
			 * @param min 最小値
			 * @param max 最大値
			 * @return min～maxの範囲で乱数を返す
			 */
			static float Random(const float min, const float max)
			{
				std::uniform_real_distribution<float> dist(min, max);
				return dist(m_randomEngine);
			}
			/**
			 * @brief パーセントで乱数を生成する
			 * @param percent 0.0f～100.0fの範囲で指定する
			 * @return percentの確率でtrueを返す
			 */
			static bool Percent(const float percent)
			{
				const float clamped = std::clamp(percent, PERCENT_MIN, PERCENT_MAX);
				std::bernoulli_distribution dist(clamped / 100.0f);
				return dist(m_randomEngine);
			}
			/**
			 * @brief コンテナからランダムに要素を取得する
			 * @tparam T コンテナの要素型
			 * @param container 要素を取得するコンテナ
			 * @return コンテナからランダムに選択された要素
			 */
			template <typename T>
			static const T& Random(const std::vector<T>& container)
			{
				const int index = Random(0, static_cast<int>(container.size()) - 1);
				return container.at(index);
			}


		private:
			RandomDevice() = delete;
			~RandomDevice() = delete;


		private:
			/** 乱数生成器 */
			static std::mt19937 m_randomEngine;

			/** パーセントの最小値 */
			static constexpr float PERCENT_MIN = 0.0f;
			/** パーセントの最大値 */
			static constexpr float PERCENT_MAX = 100.0f;
		};
	}
}