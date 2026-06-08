/**
 * @file IStatus.h
 * @brief ステータス基底クラス
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace core
	{
		/** 基底クラス。必ず継承すること！ */
		struct IStatus : public Noncopyable
		{
			IStatus();
			virtual ~IStatus() = default;


		public:
			/** セットアップ処理 */
			virtual void SetUp() = 0;
			/** 更新処理 */
			virtual void Update() = 0;


		public:
			/** セットアップが完了しているかを返す */
			bool IsSetUp() const { return m_isSetUp; }


		protected:
			/** セットアップが完了しているか */
			bool m_isSetUp = false;
		};
	}
}