/**
 * @file FeverIconMenu.h
 * @brief フィーバータイム開始時に「FEVER」の文字をジャンプで登場・退場させる演出
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/Util/Curve.h"
#include <array>


namespace app
{
	namespace ui
	{
		/**
		 * @brief フィーバータイム開始時、「FEVER」の文字（F/E/V/E/R）を
		 *        Fから順に波打つようにジャンプさせながら登場させ、
		 *        一定時間経過後に同じ順番でジャンプ退場させる演出用メニュー
		 */
		class FeverIconMenu : public MenuBase
		{
		public:
			FeverIconMenu();
			~FeverIconMenu() override = default;

			/** 更新処理 */
			void Update() override;

			/** UIのロジック初期化処理 */
			void InitializeLogic() override;

			/**
			 * @brief 演出を強制的に打ち切り、全文字を即座に非表示にする
			 * @detail ラウンドがFINISH演出に移行するとUpdate()が呼ばれなくなるため、
			 *         登場・退場アニメーションの途中で呼び出し側から明示的に呼ぶ
			 */
			void ForceHide();


		private:
			/** 演出の進行状態 */
			enum class EnState
			{
				Idle,     /** 待機（非表示） */
				Entering, /** Fから順にジャンプ登場中 */
				Holding,  /** 全文字揃って待機中 */
				Exiting,  /** Fから順にジャンプ退場中 */
			};

			/** 文字1つ分のアニメーション状態 */
			struct Letter
			{
				/** 対象のUIアイコン（見つからなければnullptr） */
				UIIcon* icon = nullptr;
				/** 調整済みの最終的な着地座標（JSON上の座標をキャッシュ） */
				Vector3 finalPos = Vector3::Zero;
				/** ジャンプの放物線を描く座標カーブ */
				util::Vector3BezierCurve moveCurve;
				/** 不透明度（フェードイン・フェードアウト）のカーブ */
				util::FloatCurve alphaCurve;
				/** アルファカーブ開始を遅らせる時間（自分の移動開始時刻からの相対時間） */
				float alphaDelay = 0.0f;
				/** 自分の番（ずらし時間）が来て移動カーブを再生し始めたか */
				bool hasMoveStarted = false;
				/** アルファカーブを再生し始めたか */
				bool hasAlphaStarted = false;
			};

			/** 「FEVER」を構成する文字数 */
			static constexpr int LETTER_NUM = 5;


		private:
			/** Fから順にジャンプ登場を開始する */
			void StartEntering();
			/** Fから順にジャンプ退場を開始する */
			void StartExiting();
			/** 各文字のずらしタイミングを見てカーブを再生・座標とアルファへ反映する */
			void UpdateLetters(float deltaTime);
			/** 最後の文字（R）のカーブが再生完了したか */
			bool IsLastLetterFinished() const;
			/** 全文字を非表示にする */
			void HideAllLetters();


		private:
			/** フィーバー中かどうか（前フレームの状態。false→trueのエッジ検出用） */
			bool m_wasFeverActive = false;
			/** 演出の進行状態 */
			EnState m_state = EnState::Idle;
			/** 現在の状態に入ってからの経過時間 */
			float m_stateTimer = 0.0f;

			/** 文字ごとのアニメーション状態（F, E1, V, E2, R の順） */
			std::array<Letter, LETTER_NUM> m_letters;

			/** 全文字が揃ってから退場を始めるまでの待機時間（JSONで上書きされる） */
			float m_holdDuration = 0.5f;
		};
	}
}
