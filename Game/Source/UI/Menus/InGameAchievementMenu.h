/**
 * @file InGameAchievementMenu.h
 * @brief インゲーム中にアチーブメントの一覧と達成状況を表示するクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Achivement/AchievementManager.h"
#include "Source/UI/Menu.h"
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

			/**
			 * @brief 表示位置のYオフセットを設定する（チュートリアル等、特定ステージのみ使用）
			 * @details JSON上の初期座標を基準にオフセットを加算するため、
			 *          何度呼んでも位置が多重にずれることはない。
			 * @param offsetY 加算するYオフセット（未設定時は0＝JSON通りの位置）
			 */
			void SetPositionOffsetY(float offsetY);

			/**
			 * @brief このメニュー全体の表示/非表示を切り替える
			 * @details MenuBaseに汎用のSetDrawが無いため、このクラス自身で実装する。
			 *          非表示中はチェックアイコンの達成演出等も含めてすべて隠す。
			 * @param isDraw true=表示 / false=非表示
			 */
			void SetDraw(bool isDraw);


		private:
			/** 1アチーブメントぶんの表示情報 */
			struct AchievementUIEntry
			{
				app::achievement::AchievementBase* achieve = nullptr;
				UIText* nameText = nullptr;
				UIIcon* boxIcon = nullptr;
				UIIcon* checkIcon = nullptr;
				bool wasAchieved = false;
			};

			/** アチーブメントごとのUI情報リスト */
			std::vector<AchievementUIEntry> m_entries;

			/** 最後に確認したリロードバージョン */
			int m_lastReloadVersion = -1;

			/** 表示位置に加算するYオフセット（チュートリアルのみ非0を設定） */
			float m_positionOffsetY = 0.0f;

			/** JSON上の初期ローカル座標（オフセット計算の基準。初回のみ記憶する） */
			std::vector<Vector3> m_baseNamePos;
			std::vector<Vector3> m_baseBoxPos;
			std::vector<Vector3> m_baseCheckPos;
			bool m_basePositionsCaptured = false;

			UIIcon* m_backgroundIcon = nullptr;
			Vector3 m_baseBackgroundPos;

			bool m_isDraw = true;
		};
	}
}