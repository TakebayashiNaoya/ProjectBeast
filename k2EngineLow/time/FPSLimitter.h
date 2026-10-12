#pragma once

#include "Stopwatch.h"
namespace nsK2EngineLow {

	/// <summary>
	/// FPSに制限をかけるクラス。
	/// </summary>
	class FPSLimitter {
	public:
		/// <summary>
		/// 1フレームの開始時に呼び出してください。
		/// </summary>
		/// <remarks>
		/// 計測の開始は最初の1回だけ。以降は Wait() の終わりから次の Wait() までを1フレームとして計る。
		/// （BeginFrame() はゲームループの途中（アプリの更新の後）で呼ばれるため、ここで毎フレーム計り直すと
		///   アプリの更新時間が1フレームの時間に含まれず、上限より遅いフレームレートになってしまう）
		/// </remarks>
		void BeginFrame()
		{
			if (!m_isStarted) {
				m_sw.Start();
				m_isStarted = true;
			}
		}
		/// <summary>
		/// 指定されたFPSを超えて動作している場合、ウェイトをかける。
		/// </summary>
		void Wait()
		{
			//スピンロックを行う。
			// 1フレームの最小時間を計算
			float frameDeltaTimeMin = 1000.0f / m_maxFPS;
			float restTime = 0;
			do {
				m_sw.Stop();
				restTime = frameDeltaTimeMin - (int)m_sw.GetElapsedMillisecond();
			} while (restTime > 1.0f);
			// 待ち終わった時点から次のフレームの計測を始める
			m_sw.Start();
		}
		/// <summary>
		/// 現在設定されている最大FPSを取得。
		/// </summary>
		/// <returns></returns>
		int GetMaxFPS() const
		{
			return m_maxFPS;
		}
		/// <summary>
		/// 最大FPSを設定。
		/// </summary>
		/// <param name="maxFps"></param>
		void SetMaxFPS(int maxFPS)
		{
			m_maxFPS = maxFPS;
		}
	private:
		Stopwatch m_sw;
		int m_maxFPS = 60;
		bool m_isStarted = false;	// 計測を開始済みか
	};
}