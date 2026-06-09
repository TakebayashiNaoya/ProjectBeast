/**
 * @file Curve.h
 * @brief 線形補間のCurve処理群
 * @author 忽那, 藤谷
 */
#pragma once


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
		 * @brief 線形補間、イーズイン、イーズアウト、イーズインアウト、バウンスアウト、バウンスイン
		 */
		enum class EasingType { Linear, EaseIn, EaseOut, EaseInOut, BounceOut, BounceIn };



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
			{}


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
				case EasingType::Linear:      return t;
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
					/**
					 * @brief 終点でバウンド
					 */
				case EasingType::BounceOut:
					return BounceOut(t);
					/**
					 * @brief BounceOutの逆で、開始点でバウンド
					 */
				case EasingType::BounceIn:
					return 1.0f - BounceOut(1.0f - t);
				}
				return t;
			}


			/**
			 * @brief バウンスの補助関数
			 */
			static float BounceOut(float t)
			{
				// 1回目のバウンド。
				if (t < 1.0f / 2.75f)
				{
					// 7.5625fは、(1/2.75)^2の逆数。
					return 7.5625f * t * t;
				}
				// 2回目のバウンド。(中ぐらい)
				else if (t < 2.0f / 2.75f)
				{
					t -= 1.5f / 2.75f;
					return 7.5625f * t * t + 0.75f;
				}
				// 3回目のバウンド。(小さい)
				else if (t < 2.5f / 2.75f)
				{
					t -= 2.25f / 2.75f;
					return 7.5625f * t * t + 0.9375f;
				}
				// 4回目のバウンド。(ほぼ止まる)
				else
				{
					t -= 2.625f / 2.75f;
					return 7.5625f * t * t + 0.984375f;
				}
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




		/********************************************************************/


		/**
		 * @brief 二次ベジェ曲線のクラス
		 * @tparam T ベジェ曲線の型（Vector2、Vector3のみ）
		 * @details 始点、制御点、終点の3点を使って曲線を描く
		 *			制御点は曲がる方向と強さを決める点
		 *			時間経過に応じて現在の値を取得できる
		 */
		template<typename T>
		class QuadraticBezierCurve
		{
		private:
			/** 始める数値 (P0) */
			T m_startValue;
			/** 制御点 (P1) 曲がる方向と強さを決める点 */
			T m_controlValue;
			/** 終わる数値 (P2) */
			T m_endValue;
			/** 時間の間隔 */
			float m_duration;
			/** 現在の時間 */
			float m_currentTime;
			/** ループモード */
			LoopMode m_loopMode;
			/** 再生するかどうか */
			bool m_isPlaying;
			/** 方向 */
			int m_direction;


		public:
			QuadraticBezierCurve()
				: m_duration(1.0f)
				, m_currentTime(0.0f)
				, m_loopMode(LoopMode::Once)
				, m_isPlaying(false)
				, m_direction(1)
			{}

			/**
			 * @brief 初期化
			 * @param start 始める位置
			 * @param control 制御点（この点に向かってカーブします）
			 * @param end 終わる位置
			 * @param timeSec 時間
			 */
			void Initialize(
				const T& start,
				const T& control,
				const T& end,
				const float timeSec,
				LoopMode loopMode = LoopMode::Once
			)
			{
				m_startValue = start;
				m_controlValue = control;
				m_endValue = end;
				m_duration = std::max<float>(0.0001f, timeSec);
				m_currentTime = 0.0f;
				m_loopMode = loopMode;
				m_direction = 1;
			}

			/** 再生 */
			void Play() { m_isPlaying = true; }

			/** 停止 */
			void Stop() { m_isPlaying = false; }

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

			/** 現在の値（座標）を取得 */
			const T GetCurrentValue() const
			{
				// 進行度 t を 0.0 ~ 1.0 にクランプ
				float t = clamp<float>(m_currentTime / m_duration, 0.0f, 1.0f);

				// ベジェ曲線の計算に必要な (1 - t)
				float u = 1.0f - t;

				// (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
				return (m_startValue * (u * u)) +
					(m_controlValue * (2.0f * u * t)) +
					(m_endValue * (t * t));
			}

			/** 再生中かどうかを取得 */
			bool IsPlaying() const { return m_isPlaying; }
		};

		// using宣言の例
		/** Vector2型の二次ベジェ曲線 */
		using Vector2BezierCurve = QuadraticBezierCurve<Vector2>;
		/** Vector3型の二次ベジェ曲線 */
		using Vector3BezierCurve = QuadraticBezierCurve<Vector3>;
	}
}