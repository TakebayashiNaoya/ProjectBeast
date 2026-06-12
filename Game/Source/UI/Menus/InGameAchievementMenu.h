/**
 * @file InGameAchievementMenu.h
 * @brief インゲーム中にアチーブメントの一覧と達成状況を表示するクラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/Achivement/AchievementManager.h"
#include <vector>


namespace app
{
	namespace ui
	{
		/**
		 * @brief インゲーム右上にアチーブメント一覧と達成状況を常時表示するMenuクラス
		 * @details
		 *   JSONのelementsに各アチーブメントの名前画像・チェックボックス・チェックアイコンを列挙する。
		 *   InitializeLogic()でGetUI()により各アイコンのポインタを取得して保持する。
		 *   Update()で毎フレーム達成状態を確認し、達成した瞬間にチェックアイコンを表示する。
		 */
		class InGameAchievementMenu : public MenuBase
		{
		public:
			InGameAchievementMenu();
			~InGameAchievementMenu() override = default;

			/** 更新処理 */
			void Update() override;

			/**
			 * @brief 初期化処理（ホットリロード時も再呼び出しされる）
			 * @details AchievementManagerからアチーブメント一覧を取得し、
			 *          JSONが生成した各UIのポインタを取得して保持する。
			 *          チェックアイコンは達成済みなら表示、未達成なら非表示にする。
			 */
			void InitializeLogic() override;


		private:
			/** 1アチーブメントぶんの表示情報 */
			struct AchievementUIEntry
			{
				app::achievement::AchievementBase* achieve = nullptr;
				UIText* nameText   = nullptr;
				UIIcon* boxIcon    = nullptr;
				UIIcon* checkIcon  = nullptr;
				bool wasAchieved   = false;
			};

			/** アチーブメントごとのUI情報リスト */
			std::vector<AchievementUIEntry> m_entries;

			/** 最後に確認したリロードバージョン */
			int m_lastReloadVersion = -1;
		};
	}
}