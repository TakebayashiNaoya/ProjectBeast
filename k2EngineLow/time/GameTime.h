
#pragma once

#include "Stopwatch.h"
#include <queue>

namespace nsK2EngineLow {

	class K2EngineLow;

	/// <summary>
	/// ゲームの時間を管理するクラス。
	/// シングルトンパターンで設計されています。
	/// </summary>
	class GameTime  {
		
	public:
		GameTime()
		{
		}
		~GameTime()
		{
		}
		/// <summary>
		/// 1フレームの経過時間を固定化させます。
		/// </summary>
		/// <remark>
		/// どんな時にこれを使うのか？
		/// 例えば、完全同期型のオンラインマルチプレイなど。
		/// 完全同期型のオンラインゲームでは、各クライアント間でゲームの進行速度を一致させる必要があります。
		/// ですので、可変フレームレートでなく、固定フレームレートでゲームを作ります。
		/// そのような場合にゲーム時間を固定化させてください。
		/// </remark>
		/// <param name="fixedFrameDeltaTime"></param>
		void EnableFixedFrameDeltaTime(float fixedFrameDeltaTime)
		{
			m_fixedFrameDeltaTime = fixedFrameDeltaTime;
			m_isFixedFrameDeltaTime = true;
		}
		/// <summary>
		/// 1フレームの経過時間の固定化を解除します。
		/// </summary>
		void DisableFixedFrameDeltaTime()
		{
			m_isFixedFrameDeltaTime = false;
		}
		
		/// <summary>
		/// 1フレームの経過時間を取得(単位・秒)
		/// </summary>
		/// <remark>
		/// 直近30フレームの移動平均かつ最大0.1秒でクランプ済みの値。
		/// Lerp係数など、揺れを抑えたい処理向け。
		/// </remark>
		/// <returns></returns>
		const float GetFrameDeltaTime() const
		{
			if (m_isFixedFrameDeltaTime) {
				// 1フレームの経過時間が固定化されている。
				return m_fixedFrameDeltaTime;
			}

			return m_frameDeltaTime;
		}

		/// <summary>
		/// 直前フレームの実経過時間を取得(単位・秒、平均化・クランプなし)
		/// </summary>
		/// <remark>
		/// タイマー・クールダウンなど、実時間と正確に一致させたい処理向け。
		/// </remark>
		/// <returns></returns>
		const float GetDeltaTime() const
		{
			if (m_isFixedFrameDeltaTime) {
				return m_fixedFrameDeltaTime;
			}

			return m_rawDeltaTime;
		}

		/// <summary>
		/// 1フレームの経過時間をキューにプッシュする
		/// </summary>
		/// <param name="deltaTime">経過時間</param>
		void PushFrameDeltaTime(float deltaTime)
		{
			m_rawDeltaTime = deltaTime;

			m_frameDeltaTimeQue.push_back(deltaTime);
			if (m_frameDeltaTimeQue.size() > 30.0f) {
				float totalTime = 0.0f;
				for (auto time : m_frameDeltaTimeQue) {
					totalTime += time;
				}
				//平均値をとる。
				m_frameDeltaTime = min(1.0f / 10.0f, totalTime / m_frameDeltaTimeQue.size());
				m_frameDeltaTimeQue.pop_front();
			}
		}
		/// <summary>
		/// 計測開始
		/// </summary>
		/// <remark>
		/// 本関数はエンジン内でのみ使用します。
		/// ユーザーは使用しないでください。
		/// </remark>
		void BeginMeasurement()
		{
			//計測開始。
			m_sw.Start();
		}
		/// <summary>
		/// 計測終了
		/// </summary>
		/// <remark>
		/// 本関数はエンジン内でのみ使用します。
		/// ユーザーは使用しないでください。
		/// 計測後は即座に次の区間の計測を再開する（application->Update() の時間も
		/// 次フレームの経過時間に含めるため、Start/Stop の間を空けない）。
		/// </remark>
		void EndMeasurement()
		{
			m_sw.Stop();
			PushFrameDeltaTime(static_cast<float>(m_sw.GetElapsed()));
			m_sw.Start();
		}
	private:
		friend class K2EngineLow;
		Stopwatch m_sw;
		std::list<float> m_frameDeltaTimeQue;
		float		m_frameDeltaTime = 1.0f / 60.0f;	// 1フレームの経過時間（移動平均・クランプ済み）。
		float		m_rawDeltaTime = 1.0f / 60.0f;		// 直前フレームの実経過時間（平均化・クランプなし）。
		bool		m_isFixedFrameDeltaTime = false;		// 1フレームの経過時間を固定化する。
		float		m_fixedFrameDeltaTime = 1.0f / 60.0f;	// 固定経過時間。
	};
}