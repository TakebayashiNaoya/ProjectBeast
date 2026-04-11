/**
 * @file IglooManager.h
 * @brief かまくらを管理するクラス
 * @author 立山
 */
#pragma once


namespace app
{
	namespace actor
	{
		class ChildPenguin;

		class IglooManager {
		public:
			// シングルトンインスタンスの取得
			static IglooManager& GetInstance() {
				static IglooManager instance;
				return instance;
			}

			// かまくらの中にペンギンを追加
			void AddPenguin(ChildPenguin* penguin);
			// 外に出る時に全員のリストをクリアする
			void ClearPenguins();

			// 中にいるペンギンのリストを取得
			const std::vector<ChildPenguin*>& GetInsidePenguins() const { return m_insidePenguinList; }

		private:
			IglooManager() = default;
			~IglooManager() = default;
			IglooManager(const IglooManager&) = delete;
			IglooManager& operator=(const IglooManager&) = delete;

			std::vector<ChildPenguin*> m_insidePenguinList;
		};
	}
}