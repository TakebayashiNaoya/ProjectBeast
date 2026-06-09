/**
 * @file VideoPlayer.cpp
 * @brief 映像再生制御クラスの実装
 * @author 竹林
 */
#include "BeastEnginePreCompile.h"
#include "VideoPlayer.h"
#include "VideoClip.h"


namespace nsBeastEngine
{
	void VideoPlayer::SetClip(VideoClip* clip)
	{
		m_clip = clip;
		m_elapsed = 0.0f;
		m_currentFrame = 0;
		m_isFinished = false;
		m_isPlaying = false;
	}


	void VideoPlayer::Play()
	{
		if (!m_clip || !m_clip->IsValid()) return;
		m_isPlaying = true;
		m_isFinished = false;
	}


	void VideoPlayer::Pause()
	{
		m_isPlaying = false;
	}


	void VideoPlayer::Stop()
	{
		m_isPlaying = false;
		m_elapsed = 0.0f;
		m_currentFrame = 0;
		m_isFinished = false;
	}


	void VideoPlayer::Update(float deltaTime)
	{
		if (!m_isPlaying || !m_clip || m_isFinished) return;

		m_elapsed += deltaTime * m_speed;

		const float frameDuration = 1.0f / m_clip->GetFPS();
		while (m_elapsed >= frameDuration)
		{
			m_elapsed -= frameDuration;
			m_currentFrame++;

			if (m_currentFrame >= m_clip->GetFrameCount())
			{
				if (m_loop)
				{
					m_currentFrame = 0;
				}
				else
				{
					m_currentFrame = m_clip->GetFrameCount() - 1;
					m_isFinished = true;
					m_isPlaying = false;
					if (onFinished) onFinished();
					return;
				}
			}
		}
	}
}
