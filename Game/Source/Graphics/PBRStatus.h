/**
 * @file PBRStatus.h
 * @brief PBR補正パラメーターのステータスクラス
 */
#pragma once
#include "PBRParameter.h"
#include "Graphics/ModelRender.h"


namespace app
{
	namespace graphics
	{
		/**
		 * @brief PBR補正パラメーターのステータスクラス
		 * @details PBRParameter.jsonを読み込み、名前からPBR補正値を返す。
		 *          シングルトンとして使用する。
		 */
		class PBRStatus
		{
		public:
			PBRStatus();
			~PBRStatus();

			/**
			 * @brief 名前からPBR補正パラメーターを取得する
			 * @details 名前が見つからない場合はデフォルト値を返す
			 * @param name オブジェクト識別名
			 * @return PBR補正パラメータ
			 */
			nsBeastEngine::PBRParam GetPBRParam(const std::string& name) const;

			/**
			 * @brief シングルトンインスタンスを生成
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new PBRStatus();
				}
			}

			/**
			 * @brief シングルトンインスタンスを取得
			 * @return シングルトンインスタンスのポインタ
			 */
			static PBRStatus* Get()
			{
				return m_instance;
			}

			/**
			 * @brief シングルトンインスタンスを破棄
			 */
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}

		private:
			/** シングルトンインスタンス */
			static PBRStatus* m_instance;
		};
	}
}