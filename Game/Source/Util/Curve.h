/**
 * @file Curve.h
 * @brief 線形補間のCurve処理群
 * @author 忽那
 */
#pragma once
#include <algorithm>


namespace app
{
	namespace util
	{
		// 汎用的なclamp関数テンプレート
		template <typename T>
		T clamp(T value, T low, T high) {
			if (value < low) {
				return low;
			}
			if (value > high) {
				return high;
			}
			return value;
		}


		/**
		 * @brief イージングの種類
		 * @brief 線形補間、イーズイン、イーズアウト、イーズインアウト
		 */
		enum class EasingType { Liner, EaseIn, EaseOut, EaseInOut };



		/**
		 * @brief ループの種類
		 * @brief 片道、周回(上から下、上から下を繰り返す)、往復(上から下、下から上を繰り返す)
		 */
		enum class LoopMode { Once, Loop, PingPong };



		/**
		 * @brief 汎用的なCurveテンプレート
		 */
		template<typename T>
		class Curve
		{
		private:
			/** 始める数値 */
			T m_startValue;
			/** 終わる数値 */
			T m_endValue;
			/** 時間の間隔 */
			float m_duration;
			/** 現在の時間 */
			float m_currentTime;
			/** イージングタイプ */
			EasingType m_easingType;
			/** ループモード */
			LoopMode m_loopMode;
			/** 再生するかどうか */
			bool m_isPlaying;
			/** 方向 */
			int m_direction;


		public:
			Curve()
				: m_duration(1.0f)
				, m_currentTime(0.0f)
				, m_isPlaying(false)
				, m_direction(0)
			{
			}


			/**
			 * @brief 初期化
			 * @param start 始める数値
			 * @param end 終わる数値
			 * @param timeSec 時間
			 * @param EasingType type = EasingType::EaseOut イーズアウト
			 * @param LoopMode loopMode = LoopMode::Once 片道
			 */
			void Initialize(
				const T& start
				, const T& end
				, const float timeSec
				, const EasingType type = EasingType::EaseOut
				, const LoopMode loopMode = LoopMode::Once
			)
			{
				m_startValue = start;
				m_endValue = end;
				m_duration = std::max<float>(0.0001f, timeSec);
				m_easingType = type;
				m_loopMode = loopMode;
				m_currentTime = 0.0f;
				m_direction = 1;
			}


		public:
			/** 再生 */
			void Play()
			{
				m_isPlaying = true;
			}


			/** 停止 */
			void Stop()
			{
				m_isPlaying = false;
			}


			/** 更新 */
			void Update(float deltaTime)
			{
				/** 再生していなければ何もしない */
				if (!m_isPlaying)return;

				/** 時間を進める */
				m_currentTime += m_loopMode == LoopMode::PingPong ? deltaTime * m_direction : deltaTime;

				/** 終了判定とループ判定 */
				if (m_currentTime >= m_duration)
				{
					if (m_loopMode == LoopMode::Once)
					{
						m_currentTime = m_duration;
						m_isPlaying = false;
					}
					else if (m_loopMode == LoopMode::Loop)
					{
						m_currentTime = 0.0f;
					}
					else if (m_loopMode == LoopMode::PingPong)
					{
						m_currentTime = m_duration;
						m_direction = -1;
					}
				}
				else if (m_currentTime <= 0.0f)
				{
					if (m_loopMode == LoopMode::PingPong)
					{
						m_currentTime = 0.0f;
						m_direction = 1;
					}
				}
			}

			/** 現在の値を取得 */
			const T GetCurrentValue() const
			{
				/**
				 * @brief clamp関数を使って0.0f~1.0fの範囲に収める
				 */


				float t = clamp<float>(m_currentTime / m_duration, 0.0f, 1.0f);
				float rete = ApplyEasingInternal(t);
				return m_startValue + (m_endValue - m_startValue) * rete;
			}


			/** 再生中か取得 */
			bool IsPlaying()const { return m_isPlaying; }


		private:
			/** イージング適用 */
			float ApplyEasingInternal(float t) const
			{
				switch (m_easingType)
				{
					/** 等速 */
				case EasingType::Liner:      return t;
					/** 加速 */
				case EasingType::EaseIn:     return t * t;
					/** 加減速 */
				case EasingType::EaseOut:    return t * (2.0f - t);
					/**
					 * @brief 加速した後ゆっくり減速する
					 */
				case EasingType::EaseInOut:
					if (t < 0.5f)return 2.0f * t * t;
					else         return -1.0f + (4.0f - 2.0f * t) * t;
				}
				return t;
			}
		};


		/**
		 * @brief float型のCurve
		 * @brief Vector2型のCurve
		 * @brief Vector3型のCurve
		 * @brief Vector4型のCurve
		 */
		using FloatCurve = Curve<float>;
		using Vector2Curve = Curve<Vector2>;
		using Vector3Curve = Curve<Vector3>;
		using Vector4Curve = Curve<Vector4>;
	}
}

