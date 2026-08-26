/**
 * @file InGameHintMenu.h
 * @brief インゲームの初回操作ヒント（ポップ表示）
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ヒントの種類
		 */
		enum class EnHintType : uint8_t
		{
			Regroup,	/** Y: 自分の隊列がクマに襲われた（よびもどし） */
			Ult,		/** LT/RT: ウルトが初めて満タンになった */
			Sneak,		/** B: 寝ているシロクマに近づいた（そっと歩く） */
			Slide,		/** X: 下り坂にさしかかった（すべると速い） */
			Num
		};


		/**
		 * @brief インゲームの初回操作ヒントを表示するMenuクラス
		 * @details 「その操作を使うべき状況が初めて発生した瞬間」に、ボタンアイコン＋短文を
		 *          画面中央上部へ数秒ポップ表示する。チュートリアルステージの代替となる仕組み。
		 *          表示は全難易度共通で、1プレイ（ステージ入場）につき各ヒント1回。
		 */
		class InGameHintMenu : public MenuBase
		{
		public:
			InGameHintMenu();
			~InGameHintMenu() override = default;
			void Update() override;
			void InitializeLogic() override;


		private:
			/** 表示ステート */
			enum class EnShowState : uint8_t
			{
				Hidden,
				PopIn,
				Hold,
				FadeOut,
			};


		private:
			/**
			 * @brief 各ヒントの発生条件を監視し、初回発生でキューに積む
			 */
			void UpdateTriggers();

			/**
			 * @brief ヒントをキューに積む（表示済み・積み済みなら何もしない）
			 * @param type 積むヒントの種類
			 */
			void Enqueue(EnHintType type);

			/**
			 * @brief 表示中ヒントのアニメーション（ポップイン→保持→フェード）を進める
			 */
			void UpdateShowAnim();

			/**
			 * @brief ヒントの表示を開始する
			 * @param type 表示するヒントの種類
			 */
			void Show(EnHintType type);

			/**
			 * @brief 表示中ヒントの全パーツへスケールとアルファを反映する
			 * @param scale ポップのスケール
			 * @param alpha 全体のアルファ（0〜1）
			 */
			void ApplyVisual(float scale, float alpha);


		private:
			/** 表示ステート */
			EnShowState m_showState = EnShowState::Hidden;
			/** 表示中のヒント */
			EnHintType m_currentType = EnHintType::Num;
			/** 表示ステートの経過時間（秒） */
			float m_showTimer = 0.0f;

			/** 表示済みフラグ（1プレイにつき各ヒント1回） */
			std::array<bool, static_cast<uint8_t>(EnHintType::Num)> m_isShown = {};
			/** 表示待ちキュー（同時発生時は先着順に1つずつ出す） */
			std::vector<EnHintType> m_queue;

			/** テキストのJSON上の基準位置（Xはヒントごとの定義値で上書きする） */
			Vector3 m_textBasePosition = Vector3::Zero;
			/** アイコンのJSON上の基準位置（UI名ハッシュ→位置。中央寄せシフトの基準） */
			std::unordered_map<uint32_t, Vector3> m_iconBasePositions;
			/** 表示中ヒントの帯の横スケール（帯幅を文の長さに合わせる） */
			float m_bandScaleX = 1.0f;
		};
	}
}
