/**
 * @file StageInfoPanel.h
 * @brief ステージ選択画面の情報パネル（制限時間・クマ数・渦潮数・記録）
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ステージ選択画面の情報パネル
		 * @details 選択中ステージの制限時間・クマ数・渦潮数・ハイスコアを表示する。
		 *          制限時間とステージ名は、インゲームシーンと共有の STAGE_INFO_TABLE から引く。
		 *          クマ数と渦潮数は配置JSONから数えるので、ステージを再生成しても表示が自動で追従する。
		 *          表示内容はステージが変わった時にだけ更新すればよい（毎フレーム呼ぶ必要はない）。
		 */
		class StageInfoPanel
		{
		public:
			StageInfoPanel();

			/**
			 * @brief UIパーツを取得する
			 * @param menu パーツを持っているメニュー
			 * @details レイアウトを読み込み直すたびに呼ぶ
			 */
			void Initialize(MenuBase& menu);

			/**
			 * @brief 指定したステージの内容を表示する
			 * @param stageIndex ステージ番号（EnStageChoices の並び）
			 */
			void SetStage(const int stageIndex);

			/**
			 * @brief パネル全体の表示/非表示を切り替える
			 */
			void SetDraw(const bool isDraw);

			/**
			 * @brief パネル全体の濃さを設定する
			 * @param rate 濃さの倍率（1.0で通常、0.0で透明）
			 * @details 各パーツの元のαに掛けるので、もともと半透明の背景も薄くなる方向にだけ動く
			 */
			void SetAlpha(const float rate);


		private:
			/** @brief クマ数・渦潮数を配置JSONから読み込む（初回のみ） */
			void LoadCountsIfNeeded();


		private:
			/** ステージ数（Easy/Normal/Hardの3ステージ分） */
			static constexpr int STAGE_NUM = 3;

			/** パネルを構成する全パーツ（表示/非表示をまとめて切り替える対象） */
			std::vector<UIBase*> m_parts;
			/** m_parts と同じ並びの、各パーツの元のα（SetAlpha の基準） */
			std::vector<float> m_baseAlphas;
			/** 制限時間のテキスト */
			UIText* m_timeText;
			/** クマ数のテキスト */
			UIText* m_bearText;
			/** 渦潮数のテキスト */
			UIText* m_whirlText;
			/** ハイスコアのテキスト */
			UIText* m_recordText;

			/** 配置JSONから読んだクマの頭数 */
			std::array<int, STAGE_NUM> m_bearCounts;
			/** 配置JSONから読んだ渦潮の数 */
			std::array<int, STAGE_NUM> m_whirlCounts;
			/** クマ数・渦潮数を読み込み済みか */
			bool m_isCountsLoaded;
		};
	}
}
