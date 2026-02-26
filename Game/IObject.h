/**
 * @file IObject.h
 * @brief ゲームオブジェクトの基底クラス
 * @author 立山
 */
#pragma once


namespace app
{
	/**
	 * @brief ゲームで必要になるオブジェクトの基底クラス
	 */
	class IObject : public Noncopyable
	{
	public:


	public:
		virtual void Start() = 0;
		virtual void Update() = 0;
		virtual void Render(RenderContext& renderContext) = 0;


		/** 下の関数を自分で呼んでください！ */
	public:
		void StartWrapper()
		{
			if (m_isActive) {
				Start();
			}
		}


		void UpdateWrapper()
		{
			if (m_isActive && !m_isPause)
			{
				Update();
			}
		}


		void RenderWrapper(RenderContext& renderContext)
		{
			if (m_isActive) {
				Render(renderContext);
			}
		}


		/**
		 * @brief Activeフラグの設定
		 * @param isActive Activeフラグ
		 */
		void SetActive(const bool isActive)
		{
			m_isActive = isActive;
		}


		/**
		 * @brief Pauseフラグの設定
		 * @param isPause Pauseフラグ
		 */
		void SetPause(const bool isPause)
		{
			m_isPause = isPause;
		}


	protected:
		/** Activeフラグ */
		bool m_isActive = true;
		/** Pauseフラグ */
		bool m_isPause = false;
	};
}