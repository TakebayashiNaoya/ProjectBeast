/**
 * @file SpeedLineMenu.cpp
 * @brief 加速時に集中線(スピードライン)を表示するメニュー
 */
#include "stdafx.h"
#include "SpeedLineMenu.h"

#include "Source/Util/CRC32.h"

#include <cstdlib>
#include <cmath>


namespace
{
	// -----------------------------------------------------
	// 演出パラメーター(好みに応じて調整する)
	// -----------------------------------------------------

	/** フレームの経過時間。デルタタイムが取れる環境なら差し替えること */
	constexpr float kDeltaTime = 1.0f / 60.0f;

	/** フェードイン速度(1秒あたりのアルファ増加量)。加速開始は素早く出す */
	constexpr float kFadeInSpeed = 8.0f;
	/** フェードアウト速度。消える時は少し余韻を残す */
	constexpr float kFadeOutSpeed = 3.0f;

	/** 集中線の最大アルファ(1.0だと主張が強すぎるため少し抑える) */
	constexpr float kMaxAlpha = 0.85f;

	/** ズーム脈動の振幅(スケールの±何割揺らすか) */
	constexpr float kPulseAmplitude = 0.04f;
	/** ズーム脈動の速さ(ラジアン/秒) */
	constexpr float kPulseSpeed = 18.0f;

	/** 何フレームごとに集中線を「描き替える」(回転を切り替える)か */
	constexpr int kFlickerInterval = 4;

	/** 2枚目(Sub)のアルファ倍率 */
	constexpr float kSubAlphaRate = 0.5f;


	/** 0.0f～1.0fの乱数 */
	float Random01()
	{
		return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
	}
}


namespace app
{
	namespace ui
	{
		void SpeedLineMenu::InitializeLogic()
		{
			// ホットリロードでUIが作り直されるため、ポインタは毎回取り直す
			m_mainLine = GetUI<UIIcon>(Hash32("SpeedLineMain"));
			m_subLine  = GetUI<UIIcon>(Hash32("SpeedLineSub"));

			K2_ASSERT(m_mainLine != nullptr, "レイアウトJSONに \"SpeedLineMain\" (UIIcon) がありません。\n");

			// JSONで指定された初期値を演出の基準として保存しておく
			if (m_mainLine)
			{
				m_mainBaseScale = m_mainLine->m_transform.m_localTransform.m_scale;
				m_mainBaseColor = m_mainLine->m_color;
				// 非加速時は非表示から開始
				m_mainLine->SetIsDraw(false);
			}
			if (m_subLine)
			{
				m_subBaseScale = m_subLine->m_transform.m_localTransform.m_scale;
				m_subBaseColor = m_subLine->m_color;
				m_subLine->SetIsDraw(false);
			}

			m_targetAlpha = 0.0f;
			m_currentAlpha = 0.0f;
			m_time = 0.0f;
			m_flickerCounter = 0;
			m_flickerDeg = 0.0f;
		}


		void SpeedLineMenu::SetAcceleration(float accel01)
		{
			// 0.0f～1.0fにクランプしてから目標アルファへ変換
			accel01 = (accel01 < 0.0f) ? 0.0f : (accel01 > 1.0f) ? 1.0f : accel01;
			m_targetAlpha = accel01 * kMaxAlpha;
		}


		void SpeedLineMenu::Update()
		{
			if (m_mainLine)
			{
				// -----------------------------------------------------
				// 1. アルファを目標値へ滑らかに追従させる
				//    (加速開始→素早く出現、減速→余韻を残して消える)
				// -----------------------------------------------------
				const float speed = (m_targetAlpha > m_currentAlpha) ? kFadeInSpeed : kFadeOutSpeed;
				const float diff = m_targetAlpha - m_currentAlpha;
				const float step = speed * kDeltaTime;

				if (std::fabsf(diff) <= step)
				{
					m_currentAlpha = m_targetAlpha;
				}
				else
				{
					m_currentAlpha += (diff > 0.0f) ? step : -step;
				}

				// -----------------------------------------------------
				// 2. 見えている間だけ、脈動とちらつきを更新する
				// -----------------------------------------------------
				const bool visible = IsVisible();
				m_mainLine->SetIsDraw(visible);
				if (m_subLine)
				{
					m_subLine->SetIsDraw(visible);
				}

				if (visible)
				{
					m_time += kDeltaTime;

					// ズーム脈動: スケールを微小に揺らして疾走感を出す
					const float pulseScale = 1.0f + kPulseAmplitude * std::sinf(m_time * kPulseSpeed);

					// 描き替えちらつき: 数フレームごとにランダムなZ回転へ切り替え、
					// 手描きアニメの「集中線を描き直した」ように見せる
					if (++m_flickerCounter >= kFlickerInterval)
					{
						m_flickerCounter = 0;
						m_flickerDeg = Random01() * 360.0f;
					}

					ApplyToLine(m_mainLine, m_mainBaseScale, m_mainBaseColor, m_currentAlpha, m_flickerDeg, pulseScale);

					if (m_subLine)
					{
						// 2枚目は逆位相の脈動+別回転で重ねる
						const float subPulse = 1.0f + kPulseAmplitude * std::sinf(m_time * kPulseSpeed + 3.14159265f);
						ApplyToLine(m_subLine, m_subBaseScale, m_subBaseColor, m_currentAlpha * kSubAlphaRate, -m_flickerDeg + 90.0f, subPulse);
					}
				}
			}

			// 最後に基底のUpdateを呼び、UIIcon::Update()に
			// ここで設定した m_color / m_transform を反映させる
			MenuBase::Update();
		}


		void SpeedLineMenu::ApplyToLine(
			UIIcon* line
			, const Vector3& baseScale
			, const Vector4& baseColor
			, const float alpha
			, const float flickerDeg
			, const float pulseScale
		)
		{
			// RGBはJSON指定の色を維持し、アルファのみ演出で制御する
			Vector4 color = baseColor;
			color.w = baseColor.w * alpha;
			line->m_color = color;

			// 脈動スケール
			Vector3 scale = baseScale;
			scale.x *= pulseScale;
			scale.y *= pulseScale;
			line->m_transform.m_localTransform.m_scale = scale;

			// ちらつき回転
			Quaternion rot;
			rot.SetRotationDegZ(flickerDeg);
			line->m_transform.m_localTransform.m_rotation = rot;
		}
	}
}
