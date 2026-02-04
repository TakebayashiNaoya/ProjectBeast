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
		class RenderContext;


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
			if (m_isActive) {
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


	protected:
		/** Activeフラグ */
		bool m_isActive = true;
	};
}