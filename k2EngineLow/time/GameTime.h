
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
				return m_fixedFrameDeltaTime * GetTimeScale();
			}

			return m_frameDeltaTime * GetTimeScale();
		}

		/// <summary>
		/// 短いスローモーション（ヒットストップ）を開始する。
		/// </summary>
		/// <remark>
		/// GetFrameDeltaTime()/GetDeltaTime() が返す経過時間を、実時間で
		/// durationSec 秒のあいだ scale 倍に縮め、その後自動で元に戻す。
		/// ウルト発動などの「タメ」の演出に使う。
		/// </remark>
		void StartSlowMotion(float scale, float durationSec)
		{
			m_slowMotionScale = scale;
			m_slowMotionTimer = durationSec;
		}

		/// <summary>
		/// 現在の時間倍率を取得する（スローモーション中でなければ1.0）。
		/// </summary>
		float GetTimeScale() const
		{
			return (m_slowMotionTimer > 0.0f) ? m_slowMotionScale : 1.0f;
		}

		/// <summary>
		/// 直前フレームの実経過時間を取得(単位・秒、平均化なし・最大0.1秒でクランプ済み)
		/// </summary>
		/// <remark>
		/// タイマー・クールダウンなど、実時間と正確に一致させたい処理向け。
		/// </remark>
		/// <returns></returns>
		const float GetDeltaTime() const
		{
			if (m_isFixedFrameDeltaTime) {
				return m_fixedFrameDeltaTime * GetTimeScale();
			}

			return m_rawDeltaTime * GetTimeScale();
		}

		/// <summary>
		/// 1フレームの経過時間をキューにプッシュする
		/// </summary>
		/// <remark>
		/// ロード中の重い同期処理(Ocean::Start()等)を挟んだ直後のフレームは
		/// 実測値が異常に大きくなりうるため、生値の時点で上限クランプする。
		/// これにより ResetFrameDeltaTime() 直後にこの関数が呼ばれても、
		/// 異常値が m_rawDeltaTime や移動平均キューへ混入しない。
		/// </remark>
		/// <param name="deltaTime">経過時間</param>
		void PushFrameDeltaTime(float deltaTime)
		{
			deltaTime = min(MAX_DELTA_TIME, deltaTime);

			m_rawDeltaTime = deltaTime;

			m_frameDeltaTimeQue.push_back(deltaTime);
			if (m_frameDeltaTimeQue.size() > 30.0f) {
				float totalTime = 0.0f;
				for (auto time : m_frameDeltaTimeQue) {
					totalTime += time;
				}
				//平均値をとる。
				m_frameDeltaTime = min(MAX_DELTA_TIME, totalTime / m_frameDeltaTimeQue.size());
				m_frameDeltaTimeQue.pop_front();
			}
		}
		/// <summary>
		/// 移動平均キューをリセットする
		/// </summary>
		/// <remark>
		/// シーン読み込み完了直後など、ロード中の重い同期処理でdeltaTimeが
		/// 異常に大きくなったフレームを含む古い履歴を破棄したい場合に呼ぶ。
		/// </remark>
		void ResetFrameDeltaTime()
		{
			m_frameDeltaTimeQue.clear();
			m_frameDeltaTime = 1.0f / 60.0f;
			m_rawDeltaTime = 1.0f / 60.0f;
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
			const float elapsed = static_cast<float>(m_sw.GetElapsed());
			// スローモーションの残り時間は「実時間」で進める。
			// これにより指定した実時間どおりに必ず終了する。
			if (m_slowMotionTimer > 0.0f) {
				m_slowMotionTimer -= min(MAX_DELTA_TIME, elapsed);
			}
			PushFrameDeltaTime(elapsed);
			m_sw.Start();
		}
	private:
		friend class K2EngineLow;
		static constexpr float MAX_DELTA_TIME = 1.0f / 10.0f;	// 1フレームの経過時間として扱う上限値（生値・移動平均共通）。
		Stopwatch m_sw;
		std::list<float> m_frameDeltaTimeQue;
		float		m_frameDeltaTime = 1.0f / 60.0f;	// 1フレームの経過時間（移動平均・クランプ済み）。
		float		m_rawDeltaTime = 1.0f / 60.0f;		// 直前フレームの実経過時間（平均化・クランプなし）。
		bool		m_isFixedFrameDeltaTime = false;		// 1フレームの経過時間を固定化する。
		float		m_fixedFrameDeltaTime = 1.0f / 60.0f;	// 固定経過時間。
		float		m_slowMotionScale = 1.0f;	// スローモーション中の時間倍率。
		float		m_slowMotionTimer = 0.0f;	// スローモーションの残り時間（実時間・秒）。
	};
}