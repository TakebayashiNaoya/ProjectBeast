/**
 * @file ScorePopupAnimatorMenu.h
 * @brief スコア加算ポップアップのアニメーションを担当するクラス
 * @author 立山
 */
#pragma once
#include "Source/UI/Model/ScorePopupAnimStatus.h"
#include "Source/UI/UIParts.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief スコア加算ポップアップのアニメーションを担当するクラス
		 * @details
		 *   内部にUITextを1つ持つ。色のフェードはUIAnimationFactoryで
		 *   生成したUIColorAnimationをFadeIn用→FadeOut用に差し替えながら
		 *   再生し、色の補間自体はUIColorAnimation側に任せる。
		 *   こちら側はフェードイン→待機→フェードアウトの
		 *   タイミング管理と、上に浮き上がる移動の更新だけを行う。
		 */
		class ScorePopupAnimator
		{
		public:
			ScorePopupAnimator();
			~ScorePopupAnimator();

			void Update();
			void Render(RenderContext& rc);

			void Initialize(const Vector3& basePosition);
			void Play(const int addScore);

			bool IsPlaying() const { return m_isPlaying; }


		private:
			/** 表示する数字テキスト本体（UIPartsのUITextをそのまま利用） */
			UIText m_text;

			/** JSONをUIAnimationParameterへ読み込ませる役割を持つ（生の値も取得可能） */
			std::unique_ptr<ScorePopupAnimStatus> m_animStatus;

			/** 待機状態（非表示）時の基準座標 */
			Vector3 m_basePosition;

			bool m_isPlaying;
			bool m_isFadeInFinished;
			bool m_isFadeOutStarted;

			float m_elapsedTime; // Play()からの経過時間（浮き上がり演出に使用）
			float m_waitTimer;   // フェードイン終了後の待機経過時間
		};
	}
}
