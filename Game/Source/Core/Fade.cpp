/**
 * @file Fade.cpp
 * @brief ロード画面を表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "Fade.h"


namespace app
{
	namespace core
	{
		Fade* Fade::m_instance = nullptr;

		/** ローディングサークルのサイズ */
		static constexpr float CIRCLE_SIZE = 200.0f;
		/** ローディングサークルの位置（画面右下） */
		static const Vector3 CIRCLE_POSITION = { 700.0f, -350.0f, 0.0f };
		/** ローディングサークルの回転速度（ラジアン/秒） */
		static constexpr float CIRCLE_SPEED = Math::PI * 2.0f;


		Fade::Fade()
			: m_state(FadeState::None)
			, m_timer(0.0f)
			, m_duration(0.0f)
			, m_isFadeIn(false)
			, m_isFadeOut(false)
			, m_circleInited(false)
			, m_showCircle(false)
			, m_circleAngle(0.0f)
		{
			// 背景画像のみコンストラクタで初期化
			m_fadeRender.Init("Assets/spriteData/UI/Load/Load.DDS", 1920.0f, 1080.0f);

			// 遅延初期化：初回呼び出し時にスプライトを生成
			if (!m_circleInited)
			{
				m_circleRender = std::make_unique<SpriteRender>();
				m_circleRender->Init("Assets/spriteData/UI/Load/LoadingCircle.DDS", CIRCLE_SIZE, CIRCLE_SIZE);
				m_circleRender->SetPosition(CIRCLE_POSITION);
				m_circleRender->SetPivot({ 0.5f, 0.5f });
				m_circleRender->Update();
				m_circleInited = true;
			}
		}


		Fade::~Fade()
		{}


		void Fade::Update()
		{
			if (m_state == FadeState::None && !m_showCircle) {
				return;
			}
			FadeProcess();
			m_fadeRender.Update();

			// ローディングサークルの回転
			if (m_showCircle && m_circleRender)
			{
				float delta = g_gameTime->GetFrameDeltaTime();
				m_circleAngle -= CIRCLE_SPEED * delta;

				Quaternion rot;
				rot.SetRotationZ(m_circleAngle);
				m_circleRender->SetRotation(rot);
				m_circleRender->Update();
			}
		}


		void Fade::FadeProcess()
		{
			float delta = g_gameTime->GetFrameDeltaTime();

			if (m_state == FadeState::FadeIn)
			{
				m_isFadeIn = true;
				m_timer -= delta;
				if (m_timer <= 0.0f)
				{
					m_timer = 0.0f;
					m_isFadeIn = false;
					m_state = FadeState::None;
				}
			}
			else if (m_state == FadeState::FadeOut)
			{
				m_isFadeOut = true;
				m_timer += delta;
				if (m_timer >= m_duration)
				{
					m_isFadeOut = false;
					m_timer = m_duration;
				}
			}
		}


		void Fade::FadeIn(float duration)
		{
			m_state = FadeState::FadeIn;
			m_duration = duration;
			m_timer = duration;
		}


		void Fade::FadeOut(float duration)
		{
			m_state = FadeState::FadeOut;
			m_duration = duration;
			m_timer = 0.0f;
		}


		void Fade::ShowLoadingCircle()
		{

			m_showCircle = true;
			m_circleAngle = 0.0f;
		}


		void Fade::HideLoadingCircle()
		{
			m_showCircle = false;
		}


		void Fade::Render(RenderContext& rc)
		{
			if (m_state == FadeState::None && !m_showCircle) {
				return;
			}

			// 背景の描画
			if (m_state != FadeState::None)
			{
				float alpha = m_timer / m_duration;
				alpha = std::clamp(alpha, 0.0f, 1.0f);
				m_fadeRender.SetMulColor({ 1.0f, 1.0f, 1.0f, alpha });
				m_fadeRender.Draw(rc);
			}
			else if (m_showCircle)
			{
				// フェード完了後もサークル表示中は背景を不透明で描画
				m_fadeRender.SetMulColor({ 1.0f, 1.0f, 1.0f, 1.0f });
				m_fadeRender.Draw(rc);
			}

			// ローディングサークルの描画
			if (m_showCircle && m_circleRender)
			{
				m_circleRender->Draw(rc);
			}
		}
	}
}
