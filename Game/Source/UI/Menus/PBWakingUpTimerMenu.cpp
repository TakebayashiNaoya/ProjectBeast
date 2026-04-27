/**
 * @file PBWakingUpTimerMenu.cpp
 * @brief PB起床タイマーの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "PBWakingUpTimerMenu.h"
#include "Source/UI/Model/PBWakingUpTimerAnimStatus.h"
#include "Source/UI/Animation/UIAnimationFactory.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// 長針の矢印の先端。
			const Vector2 ARROW_PIVOT = { 0.3f,0.0f };

			// タイマーの色(緑色)。
			const Vector4 GREEN_COLOR = { 0.0f, 0.8f,0.0f,1.0f };
			// タイマーの色(黄色)。
			const Vector4 YELLOW_COLOR = { 1.0f,1.0f,0.0f,1.0f };
			// タイマーの色(赤色)。
			const Vector4 RED_COLOR = { 1.0f,0.0f,0.0f,1.0f };

			// 半透明の値。
			constexpr float SKELTON_VALUE = 0.5f;

			// 半透明の緑色。
			const Vector4 SKELTON_GREEN_COLOR = { 0.0f,0.8f,0.0f,0.5f };
			// 半透明の赤色。
			const Vector4 SKELTON_YELLOW_COLOR = { 1.0f,1.0f,0.0f,0.5f };
			// 半透明の赤色。
			const Vector4 SKELTON_RED_COLOR = { 0.8f,0.0f,0.0f,0.5f };


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

			// タイマーのリセット値。
			constexpr float TIMER_RESET_VALUE = 0.0f;

			// タイマーのオフセット位置Y値。
			constexpr float OFFSET_POS_Y = 225.0f;
		}

		PBWakingUpTimerMenu::PBWakingUpTimerMenu()
			: m_currentPBTime(0.0f)
			, m_targetPosition(Vector3::Zero)
			, m_isDraw(false)
			, m_isGreenPlayed(false)
			, m_isYellowPlayed(false)
			, m_isRedPlayed(false)
		{
			// シロクマ専用UIAnimationStatusを生成。
			m_pbAnimStatus = std::make_unique<PBWakingUpTimerAnimStatus>();
			// シロクマ専用のセットアップUIを呼び出す。
			m_pbAnimStatus->SetUpUI();

			// シロクマ専用UIのステータスを生成。
			m_pbTimerStatus = std::make_unique<PBWakingUpTimerStatus>();

			// シロクマ専用UIのセットアップUIを呼び出す。
			m_pbTimerStatus->SetUpUI();
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

				auto* alarmClock = GetUI<UIIcon>(Hash32("PBalarmClock"));
				if (alarmClock)alarmClock->m_isDraw = false;

				auto* longNeedle = GetUI<UIIcon>(Hash32("PBNeedle"));
				if (longNeedle)longNeedle->m_isDraw = false;

				PBWakingUpTimerClass::Update();
				return;
			}

			// キャンバスを取得。
			auto* canvas = GetCanvas();
			// キャンバスがある時、
			if (canvas) {
				// 時計回りに回転させるサークルゲージの処理を呼び出す。
				CircleTimer();
			}

			// MenuBaseの更新処理
			PBWakingUpTimerClass::Update();
		}


		void PBWakingUpTimerMenu::CircleTimer()
		{
			// サークルゲージAが内側の円、サークルゲージBが外側の円とする。
			// (長針の部分がゲージの内径の部分に当たる)
			auto* cirGaugeA = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeA"));
			auto* cirGaugeB = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeB"));
			auto* needle = GetUI<UIIcon>(Hash32("PBNeedle"));
			auto* alarmClock = GetUI<UIIcon>(Hash32("PBalarmClock"));
			// サークルゲージAとサークルゲージBが存在しない場合は処理を中断する。
			if (!cirGaugeA || !cirGaugeB || !needle || !alarmClock)return;

			// ワールド座標をスクリーン座標に変換してアイコンの位置をシロクマの頭上に設定。
			Vector2 screenPos = Vector2::Zero;
			g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, m_targetPosition);

			// タイマーのオフセットY値を取得。
			const float offsetY = m_pbTimerStatus->GetOffsetValueY();
			
			// ゲージAと針を同じ中心座標に配置するために中心座標を計算する。
			Vector3 centerPos = Vector3(screenPos.x, screenPos.y + offsetY, INITIALIZE_POS_Z);

			Vector3 setPos = Vector3(screenPos.x, screenPos.y + OFFSET_POS_Y, INITIALIZE_POS_Z);

			// アイコンAとアイコンBと長針と目覚まし時計の位置をエネミーの頭上に設定。
			cirGaugeA->m_transform.m_localTransform.m_position = centerPos;
			cirGaugeB->m_transform.m_localTransform.m_position = centerPos;
			needle->m_transform.m_localTransform.m_position = centerPos;
			alarmClock->m_transform.m_localTransform.m_position = setPos;


			// 残り時間を0~1に正規化する。
			const float rotRatio = m_currentPBTime / DEGREE_VALUE;
			// サークルゲージAの進行度を設定する。
			cirGaugeA->SetProgressRange(rotRatio,RATIO_PROGRESS);
			// サークルゲージBの進行度を設定する。
			cirGaugeB->SetProgress(RATIO_PROGRESS);
			// 長針にピボットを設定する。
			needle->SetPivot(ARROW_PIVOT);
			Quaternion rot;
			// 長針を回転させる。
			rot.SetRotationDegZ(DEGREE_MAX_VALUE * rotRatio);
			// 長針の回転を適用する。
			needle->m_transform.m_localTransform.m_rotation = rot;

			
			// 現在のタイマーが30秒から20秒の間には、緑色から黄色に変化させるアニメーションを再生する。
			if (m_currentPBTime <= m_pbTimerStatus->GetTimerFirstValue() && m_currentPBTime >= m_pbTimerStatus->GetTimerSecondValue())
			{
				// まだ再生されていないならば
				if (!m_isGreenPlayed)
				{
					// すでに再生されているアニメーションを削除する。
					cirGaugeA->RemoveAnimation(animKey::PB_CIRCLE_COLOR_SECOND_ANIM_KEY);
					cirGaugeA->RemoveAnimation(animKey::PB_CIRCLE_COLOR_THIRD_ANIM_KEY);
					
					const bool colorCheck = UIAnimationFactory::Attach<UIColorAnimation>
						(cirGaugeA, animKey::PB_CIRCLE_COLOR_FIRST_ANIM_KEY);
					
					// 両方取得出来たら、同フレームでアニメーションを再生。
					if (colorCheck) {
						cirGaugeA->SetBgColor(Vector4(0.0f,0.0f,0.0f,0.4f));

						cirGaugeA->FindAnimation(animKey::PB_CIRCLE_COLOR_FIRST_ANIM_KEY);
						cirGaugeA->PlayAnimation();
						m_isGreenPlayed = true;
						m_isYellowPlayed = false;
						m_isRedPlayed = false;
					}
				}
			}
			// 現在のタイマーが20から10秒の間は、黄色から赤色に変化させるアニメーションを再生する。
			else if (m_currentPBTime <= m_pbTimerStatus->GetTimerSecondValue() && m_currentPBTime >= m_pbTimerStatus->GetTimerThirdValue())
			{
				// 緑色から黄色のアニメーションが再生されていて、まだ黄色から赤色のアニメーションが再生されていないならば
				if (!m_isYellowPlayed)
				{
					cirGaugeA->RemoveAnimation(animKey::PB_CIRCLE_COLOR_FIRST_ANIM_KEY);
					cirGaugeA->RemoveAnimation(animKey::PB_CIRCLE_COLOR_THIRD_ANIM_KEY);
					needle->RemoveAnimation(animKey::PB_NEEDLE_ROT_ANIM_KEY);

					const bool colorCheck=UIAnimationFactory::Attach<UIColorAnimation>
						(cirGaugeA, animKey::PB_CIRCLE_COLOR_SECOND_ANIM_KEY);

					if (colorCheck) {
						cirGaugeA->SetBgColor(Vector4(0.0f, 0.0f, 0.0f, 0.4f));
						
						cirGaugeA->FindAnimation(animKey::PB_CIRCLE_COLOR_SECOND_ANIM_KEY);
						cirGaugeA->PlayAnimation();
						m_isYellowPlayed = true;
						m_isGreenPlayed = false;
						m_isRedPlayed = false;
					}
				}
			}
			// 10~0秒の間は、赤色のアニメーションを再生する。
			else if (m_currentPBTime <= m_pbTimerStatus->GetTimerThirdValue() && m_currentPBTime >= m_pbTimerStatus->GetTimerFourthValue())
			{
				// 黄色から赤色のアニメーションが再生されていて、まだ赤色のアニメーションが再生されていないならば
				if (!m_isRedPlayed)
				{
					cirGaugeA->RemoveAnimation(animKey::PB_CIRCLE_COLOR_FIRST_ANIM_KEY);
					cirGaugeA->RemoveAnimation(animKey::PB_CIRCLE_COLOR_SECOND_ANIM_KEY);

					const bool colorCheck = UIAnimationFactory::Attach<UIColorAnimation>
						(cirGaugeA, animKey::PB_CIRCLE_COLOR_THIRD_ANIM_KEY);

					if (colorCheck) {
						cirGaugeA->SetBgColor(Vector4(0.0f, 0.0f, 0.0f, 0.4f));
						
						cirGaugeA->FindAnimation(animKey::PB_CIRCLE_COLOR_THIRD_ANIM_KEY);
						cirGaugeA->PlayAnimation();
						m_isRedPlayed = true;
						m_isYellowPlayed = false;
						m_isGreenPlayed = false;
					}
				}
			}

			// サークルゲージの描画を有効にする。
			cirGaugeA->m_isDraw  = true;
			cirGaugeB->m_isDraw  = true;
			needle->m_isDraw	 = true;
			alarmClock->m_isDraw = true;
			// タイマーが0秒以下になったら、フラグをリセットする。
			if (m_currentPBTime <= TIMER_RESET_VALUE)
			{
				m_isGreenPlayed = false;
				m_isYellowPlayed = false;
				m_isRedPlayed = false;
			}
		}


		void PBWakingUpTimerMenu::InitializeLogic()
		{
			// 初期状態では全てのUIを非表示にする。
			auto* cirGaugeA = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeA"));
			if (cirGaugeA)cirGaugeA->m_isDraw = false;

			auto* cirGaugeB = GetUI<UICircleGauge>(Hash32("PBTimerCircleGaugeB"));
			if (cirGaugeB)cirGaugeB->m_isDraw = false;

			auto* alarmClock = GetUI<UIIcon>(Hash32("PBalarmClock"));
			if (alarmClock) alarmClock->m_isDraw = false;

			auto* needle = GetUI<UIIcon>(Hash32("PBNeedle"));
			if (needle) {
				// 長針の基点を左端に設定する。
				needle->SetPivot(ARROW_PIVOT);
				needle->m_isDraw = false;
			}
		}
	}
}