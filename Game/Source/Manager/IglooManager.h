/**
 * @file IglooManager.h
 * @brief かまくらを管理するクラス
 */
#pragma once


namespace app
{
	namespace actor
	{
		class ChildPenguin;
		class DaddyPenguin;

		class IglooManager {
		public:
			/** インスタンスの生成 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new IglooManager();
				}
			}
			/** インスタンスの取得 */
			static IglooManager& GetInstance()
			{
				return *m_instance;
			}
			/** インスタンスの破棄 */
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


		public:
			// かまくらの中にペンギンを追加
			void AddPenguin(ChildPenguin* penguin);
			// 外に出る時に全員のリストをクリアする
			void ClearPenguins();

			// 中にいるペンギンのリストを取得
			const std::vector<ChildPenguin*>& GetInsidePenguins() const { return m_insidePenguinList; }

			/** かまくらが壊れた時に呼ばれる処理 */
			void EjectAllPenguins(const Vector3& iglooPosition);

			// =================================================
			// ★ 追加：親ペンギンの出入りを管理する
			// =================================================
			void SetInsideDaddy(DaddyPenguin* daddy) { m_insideDaddy = daddy; }
			DaddyPenguin* GetInsideDaddy() const { return m_insideDaddy; }


		private:
			IglooManager() = default;
			~IglooManager() = default;


		private:
			static IglooManager* m_instance;

			std::vector<ChildPenguin*> m_insidePenguinList;

			DaddyPenguin* m_insideDaddy = nullptr;
		};
	}
}