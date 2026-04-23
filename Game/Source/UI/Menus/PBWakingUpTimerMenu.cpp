/**
 * @file PBWakingUpTimerMenu.cpp
 * @brief PB起床タイマーの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "PBWakingUpTimerMenu.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// 30~20の間で色が変わるタイマーの値(緑色)。
			constexpr float TIMER_FIRST_VALUE = 30.0f;
			// 20~10の間で色が変わるタイマーの値(黄色)。
			constexpr float TIMER_SECOND_VALUE = 20.0f;
			// 10未満で色が変わるタイマーの値(赤色)。
			constexpr float TIMER_THIRD_VALUE = 10.0f;
			// タイマーの値が0以下の値。
			constexpr float TIMER_END_VALUE = 0.0f;

			// タイマーの色(緑色)。
			const Vector4 GREEN_COLOR = { 0.0f, 0.8f,0.0f,1.0f };
			// タイマーの色(黄色)。
			const Vector4 YELLOW_COLOR = { 1.0f,1.0f,0.0f,1.0f };
			// タイマーの色(赤色)。
			const Vector4 RED_COLOR = { 1.0f,0.0f,0.0f,1.0f };

			// 半透明の値。
			constexpr float SKELTON_VALUE = 0.5f;

			// タイマーの進行度の比率0.0f~1.0f。
			constexpr float RATIO_PROGRESS = 1.0f;

			// タイマーの円の最大角度(360度で一周)。
			constexpr float DEGREE_MAX_VALUE = 360.0f;

			// タイマーの円が一周するまでの時間(30秒)。
			constexpr float DEGREE_VALUE = 30.0f;

			// タイマーのオフセット位置（エネミーの頭上）
			constexpr float OFFSET_Y = 250.0f;

			// シロクマの頭上の初期位置。
			constexpr float INITIALIZE_POS_Z = 0.0f;
		}


		PBWakingUpTimerMenu::PBWakingUpTimerMenu()
			: m_currentPBTime(0.0f)
			, m_targetPosition(Vector3::Zero)
			, m_isDraw(false)
			, m_isYellowPlayed(false)
			, m_isRedPlayed(false)
		{
			//core::ParameterManager::Get()->LoadParameter<>
		}


		PBWakingUpTimerMenu::~PBWakingUpTimerMenu()
		{}


		void PBWakingUpTimerMenu::Update()
		{
			// 全てのUIを非表示にする。
			if (!m_isDraw)
			{
				auto* cirGaugeA = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeA"));
				if (cirGaugeA)cirGaugeA->m_isDraw = false;
				
				auto* cirGaugeB = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeB"));
				if(cirGaugeB) cirGaugeB->m_isDraw = false;

				auto digitA = GetUI<UIDigit>(Hash32("PBTimerDigitA"));
				if(digitA)digitA->m_isDraw = false;

				PBWakingUpTimerClass::Update();
				return;
			}

			auto* canvas = GetCanvas();
			if (canvas) {
				// 時計回りに回転させるサークルゲージの処理を呼び出す。
				CircleTimer();
			}

			// UIDigitを取得。
			auto* digitA = GetUI<UIDigit>(Hash32("PBTimerDigitA"));
			// ワールド座標をスクリーン座標に変換してDigitの位置をエネミーの頭上に設定。
			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, m_targetPosition);
			digitA->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y, 0.0f);

			// タイマー値をint型にキャストして表示。
			digitA->SetNumber(static_cast<int>(m_currentPBTime));
			digitA->m_isDraw = true;
			
			// MenuBaseの更新処理
			PBWakingUpTimerClass::Update();
		}


		void PBWakingUpTimerMenu::CircleTimer()
		{
			// サークルゲージAが内側の円、サークルゲージBが外側の円とする。
			auto* cirGaugeA = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeA"));
			auto* cirGaugeB = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeB"));
			// サークルゲージAとサークルゲージBが存在しない場合は処理を中断する。
			if (!cirGaugeA || !cirGaugeB)return;

			// ワールド座標をスクリーン座標に変換してアイコンの位置をシロクマの頭上に設定。
			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, m_targetPosition);

			// アイコンAとアイコンBの位置をエネミーの頭上に設定。
			cirGaugeA->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y, INITIALIZE_POS_Z);
			cirGaugeB->m_transform.m_localTransform.m_position = Vector3(screenPos.x, screenPos.y + OFFSET_Y, INITIALIZE_POS_Z);


			// 残り時間を0~1に正規化する。
			const float rotRatio = m_currentPBTime / DEGREE_VALUE;
			// サークルゲージAの進行度を設定する。
			cirGaugeA->SetProgress(rotRatio);
			// サークルゲージBの進行度を設定する。
			cirGaugeB->SetProgress(RATIO_PROGRESS);

			// 緑色から黄色にじわじわと変化させるアニメーションを作成する。
			auto greenLerpAnim = std::make_unique<UIColorAnimation>();
			Vector4 firstStartColor = GREEN_COLOR;
			Vector4 firstEndColor = YELLOW_COLOR;

			// イージングされた値をサークルゲージの色に設定させる。
			greenLerpAnim->SetParameter(
					firstStartColor
				,	firstEndColor
				,	1.0f
				,	util::EasingType::EaseOut
				,	util::LoopMode::Once
			);
			
			// 黄色から赤色にじわじわと変化させるアニメーションを作成する。
			auto yellowLerpAnim = std::make_unique<UIColorAnimation>();
			Vector4 secondStartColor = YELLOW_COLOR;
			Vector4 secondEndColor = RED_COLOR;
			
			// イージングされた値をサークルゲージの色に設定させる。
			yellowLerpAnim->SetParameter(
					secondStartColor
				,	secondEndColor
				,	1.0f
				,	util::EasingType::EaseOut
				,	util::LoopMode::Once
			);

			auto redAnim = std::make_unique<UIColorAnimation>();
			Vector4 thirdStartColor = RED_COLOR;
			Vector4 thirdEndColor = RED_COLOR;
			redAnim->SetParameter(
					thirdStartColor
				,	thirdEndColor
				,	1.0f
				,	util::EasingType::EaseOut
				,	util::LoopMode::Once
			);

			// イージングされた値をサークルゲージの色に設定させる。
			greenLerpAnim->SetFunc([this, cirGaugeA](Vector4 v)
			{
				// イージングされた値をサークルゲージの色に設定させる。
				if (cirGaugeA)cirGaugeA->SetGaugeColor(v);
			});

			yellowLerpAnim->SetFunc([this, cirGaugeA](Vector4 v)
			{
				// イージングされた値をサークルゲージの色に設定させる。
				if (cirGaugeA)cirGaugeA->SetGaugeColor(v);
			});


			cirGaugeA->AddAnimation(Hash32("redAnim"), std::move(redAnim));
			cirGaugeA->AddAnimation(Hash32("yellowLerpAnim"), std::move(yellowLerpAnim));
			cirGaugeA->AddAnimation(Hash32("greenLerpAnim"), std::move(greenLerpAnim));
			
			// 最初に登録されたアニメーションがnullptrかつyellowLerpフラグがfalseの時、次のアニメーションを登録させる。
			//if (cirGaugeA->FindAnimation(Hash32("greenLerpAnim")) == nullptr && !m_isYellowPlayed)
			//{
			//	cirGaugeA->AddAnimation(Hash32("yellowLerpAnim"), std::move(yellowLerpAnim));
			//}
			//else if (cirGaugeA->FindAnimation(Hash32("yellowLerpAnim")) == nullptr && !m_isRedPlayed)
			//{
			//	cirGaugeA->AddAnimation(Hash32("redpAnim"), std::move(redAnim));
			//}


			// 1 ~ 2番目をクランプする。
			const float first = util::clamp(m_currentPBTime, TIMER_SECOND_VALUE, TIMER_FIRST_VALUE);
			// lerpAnimationをさせるためのt(時間)を計算。
			const float t = (first - TIMER_SECOND_VALUE) / (TIMER_FIRST_VALUE - TIMER_SECOND_VALUE);

			// 2 ~ 3番目をクランプする。
			const float second = util::clamp(m_currentPBTime, TIMER_THIRD_VALUE, TIMER_SECOND_VALUE);
			// 2番目に流すアニメーションの再生時間を計算。
			const float t2 = (second - TIMER_THIRD_VALUE) / (TIMER_SECOND_VALUE - TIMER_THIRD_VALUE);


			// 現在のタイマーが30秒から20秒の間には、緑色から黄色に変化させるアニメーションを再生する。
			if (m_currentPBTime == TIMER_FIRST_VALUE && m_currentPBTime >= TIMER_SECOND_VALUE)
			{
				// まだ再生されていないならば
				if (!m_isYellowPlayed)
				{
					// サークルゲージAに緑色から黄色に変化させるアニメーションを検索。
					cirGaugeA->FindAnimation(Hash32("greenLerpAnim"));
					// 最初に登録されたアニメーションを再生する。
					cirGaugeA->PlayAnimation();
					cirGaugeA->SetGaugeColor(GREEN_COLOR * t);
					cirGaugeA->SetBgColor(GREEN_COLOR * SKELTON_VALUE);
					m_isYellowPlayed = true;
				}
			}
			// 現在のタイマーが20から10秒の間は、黄色から赤色に変化させるアニメーションを再生する。
			else if (m_currentPBTime == TIMER_SECOND_VALUE && m_currentPBTime >= TIMER_THIRD_VALUE)
			{
				// lerpAnimationがtrueで、フラグがfalseの時、アニメーションを再生する。
				if (m_isYellowPlayed && !m_isRedPlayed)
				{
					// サークルゲージAに緑から黄色に変化させるアニメーションを削除。
					cirGaugeA->RemoveAnimation(Hash32("greenLerpAnim"));
					// nullptrを代入して、アニメーションが削除されたことを示す。
					greenLerpAnim = nullptr;
					// サークルゲージAに黄色から赤色に変化させるアニメーションを探す。
					cirGaugeA->FindAnimation(Hash32("yellowLerpAnim"));
					// 登録されたアニメーションを再生する。
					cirGaugeA->PlayAnimation();
					cirGaugeA->SetGaugeColor(YELLOW_COLOR);
					cirGaugeA->SetBgColor(YELLOW_COLOR * SKELTON_VALUE);
					m_isRedPlayed = true;
				}
			}
			// 10~0秒の間は、赤色のアニメーションを再生する。
			else if(m_currentPBTime <= TIMER_THIRD_VALUE && m_currentPBTime >= TIMER_END_VALUE)
			{
				cirGaugeA->RemoveAnimation(Hash32("yellowLerpAnim"));
				yellowLerpAnim = nullptr;
				cirGaugeA->FindAnimation(Hash32("redAnim"));
				cirGaugeA->PlayAnimation();
				cirGaugeA->SetGaugeColor(RED_COLOR);
				cirGaugeA->SetBgColor(RED_COLOR * SKELTON_VALUE);
			}

			// サークルゲージの描画を有効にする。
			cirGaugeA->m_isDraw = true;
			cirGaugeB->m_isDraw = true;
		}


		void PBWakingUpTimerMenu::InitializeLogic()
		{
			// 初期状態では全てのUIを非表示にする。
			auto* digitA = GetUI<UIDigit>(Hash32("PBTimerDigitA"));
			if (digitA) digitA->m_isDraw = false;

			auto* cirGaugeA = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeA"));
			if (cirGaugeA)cirGaugeA->m_isDraw = false;

			auto* cirGaugeB = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeB"));
			if (cirGaugeB)cirGaugeB->m_isDraw = false;

			// 初期状態ではサークルゲージの色を緑色に設定する。
			auto colorAnim = std::make_unique<UIColorAnimation>();
			colorAnim->SetFunc([this](Vector4 v)
			{
				// サークルゲージAを取得。
				auto* cirGaugeA = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeA"));
				// イージングされた値をサークルゲージの色に設定させる。
				if (cirGaugeA)cirGaugeA->SetGaugeColor(v);
			});
		}
	}
}