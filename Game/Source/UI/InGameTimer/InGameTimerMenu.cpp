/**
 * @file InGameTimerMenu.cpp
 * @brief インゲームタイマーの動的処理クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "InGameTimerMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// マジックナンバー対策。
			const Vector4 BLINK_TEXT_COLOR = { 20.0f, 0.0f, 0.0f, 1.0f };
			const Vector4 CLOCK_ROT_COLOR = { 1.0f, 0.0f, 0.0f, 1.0f };
			const Vector3 CLOCK_SCALE_MAX = { 1.2f, 1.2f, 1.2f };
			constexpr float CLOCK_SCALE_DURATION = 0.5f;


			// タイムが現在のタイム以下の時点滅させる闘値。
			constexpr float BLINK_THRESHOLD = 31.0f;
			// 点滅の間隔。
			constexpr float BLINK_INTERVAL = 0.5f;
			// 最大傾き角度。
			constexpr float MAX_LIMITE_DEG = 30.0f;
			// 傾きの速度。
			constexpr float SLOPE_SPEED = 8.0f;
			// 傾きの振幅。
			constexpr float SLOPE_AMP = 15.0f;
			// 10秒を切った時に拡縮を行うための条件式の秒数。
			constexpr float LAST_SECONDS = 11.0f;



			// UIの種類の数。UIの種類が増えたらこの数も増やすこと。
			constexpr uint8_t UI_TYPE_COUNT = 7;
		}




		/************************************************/


		InGameTimerMenu::InGameTimerMenu()
			: m_currentTime(0.0f)
			, m_blinkTime(0.0f)
			, m_isBlink(true)
			, m_clockAngle(0.0f)
			, m_slopeTimer(0.0f)
			, m_isBlinkAnimationPlaying(false)
			, m_isRotAnimationPlaying(false)
			, m_isScaleAnimPlaying(false)
		{}


		void InGameTimerMenu::Update()
		{
			/** 表示状態にする */
			const std::vector<std::string> iconNames =
			{
				"InGameTimerFrameIcon",
				"InGameTimerFrameBackGroundIcon",
				"TimeClockIcon",
				"CloneIcon"
			};

			for (const auto& iconName : iconNames)
			{
				if (auto* icon = GetUI<UIIcon>(Hash32(iconName.c_str())))
				{
					icon->m_isDraw = true;
				}
			}


			const std::vector<std::string> digitNames =
			{
				"MinutesDigits",
				"TensPlaceDigits",
				"OnesPlaceDigits",
			};


			if (!m_gameStartingAnimLogic.IsAnimationStarted())
			{
				m_gameStartingAnimLogic.Initialize(
					this,
					iconNames,
					digitNames,
					Vector3(0.0f, 200.0f, 0.0f)
				);
			}
			if (!m_gameStartingAnimLogic.IsAnimationFinished())
			{
				m_gameStartingAnimLogic.Update();
			}


			/** ゲーム開始時のアニメーション更新 */

			/** 分/秒に変換して表示(例: 0:00形式) */
			UpdateTimerDigits();

			/** 時計アイコンのZ軸回転演出 */
			UpdateClockRotation();

			/** MenuBaseの更新処理 */
			InGameTimerClass::Update();
		}


		void InGameTimerMenu::InitializeLogic()
		{
			/** 生成直後は全て非表示にする（UIBaseのデフォルトがm_isDraw=trueのため） */
			constexpr const char* iconNames[] =
			{
				"InGameTimerFrameBackGroundIcon",
				"InGameTimerFrameIcon",
				"TimeClockIcon",
				"CloneIcon",
			};

			for (const auto& iconName : iconNames)
			{
				if (auto* icon = GetUI<UIIcon>(Hash32(iconName)))
				{
					icon->m_isDraw = false;
				}
			}


			constexpr const char* digitNames[] =
			{
				"MinutesDigits",
				"CloneIcon",
				"TensPlaceDigits",
				"OnesPlaceDigits",
			};

			for (const auto& digitName : digitNames)
			{
				if (auto* digit = GetUI<UIDigit>(Hash32(digitName)))
				{
					digit->m_isDraw = false;
				}
			}
		}


		void InGameTimerMenu::UpdateTimerDigits()
		{
			// 現在のタイムを整数に変換。
			const int totalSec = static_cast<int>(m_currentTime);
			// 分と秒を計算。
			const int minutes = totalSec / 60;
			// 秒の10の位を計算。
			const int tensPlace = (totalSec % 60) / 10;
			// 秒の1の位を計算。
			const int onesPlace = (totalSec % 60) % 10;


			auto* minutesDigits = GetUI<UIDigit>(Hash32("MinutesDigits"));
			auto* tensPlaceDigits = GetUI<UIDigit>(Hash32("TensPlaceDigits"));
			auto* onesPlaceDigits = GetUI<UIDigit>(Hash32("OnesPlaceDigits"));

			/** 数値の設定 */
			if (minutesDigits) { minutesDigits->m_isDraw = true; minutesDigits->SetNumber(minutes); }
			if (tensPlaceDigits) { tensPlaceDigits->m_isDraw = true; tensPlaceDigits->SetNumber(tensPlace); }
			if (onesPlaceDigits) { onesPlaceDigits->m_isDraw = true; onesPlaceDigits->SetNumber(onesPlace); }


			// 残り30秒以下の時に赤く点滅させる。
			if (m_currentTime <= BLINK_THRESHOLD && m_currentTime > 0.0f)
			{
				// 点滅アニメーションが再生されていないときに初回のみ登録して再生させる。
				if (!m_isBlinkAnimationPlaying)
				{
					Vector4 startColor = Vector4::White;
					Vector4 endColor(BLINK_TEXT_COLOR);
					// 各Digitにカラーアニメーションを登録して再生させる。
					auto SetColorAnim = [&](UIDigit* digit)
						{
							if (digit == nullptr)return;
							auto colorAnim = std::make_unique<UIColorAnimation>();
							colorAnim->SetParameter(
								startColor
								, endColor
								, BLINK_INTERVAL
								, util::EasingType::EaseOut
								, util::LoopMode::PingPong
							);
							colorAnim->SetFunc([digit](const Vector4& color)
								{
									digit->m_color = color;
								});
							digit->AddAnimation(Hash32("TimerColorAnim"), std::move(colorAnim));
							digit->PlayAnimation();
						};
					SetColorAnim(minutesDigits);
					SetColorAnim(tensPlaceDigits);
					SetColorAnim(onesPlaceDigits);
					m_isBlinkAnimationPlaying = true;
				}
			}
			else
			{
				/** 通常時はアニメーションを停止して白色にする */
				if (m_isBlinkAnimationPlaying)
				{
					const Vector4 white = Vector4::White;
					auto StopColorAnim = [&](UIDigit* digit)
						{
							if (digit == nullptr)return;
							digit->StopAnimation();
							digit->m_color = white;
						};

					StopColorAnim(minutesDigits);
					StopColorAnim(tensPlaceDigits);
					StopColorAnim(onesPlaceDigits);
					m_blinkTime = 0.0f;
					m_isBlink = true;
					m_isBlinkAnimationPlaying = false;
				}
			}
		}


		void InGameTimerMenu::UpdateClockRotation()
		{
			// 傾けさせるアイコンを取得。
			auto* clock = GetUI<UIIcon>(Hash32("TimeClockIcon"));
			// nullptrチェック。アイコンが存在しないときは処理しない。
			if (clock == nullptr)return;


			// Managerから最大タイムを取得する。
			const float maxTime = TimeManager::GetInstance().GetMaxTime();
			// 最大タイムが0以下のときは処理しない。
			if (maxTime <= 0.0f)return;


			// 残り時間の割合を計算。
			const float ratio = m_currentTime / maxTime;
			// 時計の回転角度を計算(最大45度の傾き)。
			const float baseAngle = MAX_LIMITE_DEG * (1.0f - ratio);
			// 傾きの角度を計算して、sin関数を使って揺れを表現してみた。
			const float slopeAngle = sinf(m_slopeTimer * SLOPE_SPEED) * Math::DegToRad(SLOPE_AMP);


			// 残り30秒を切ったら点滅アニメーションを初回のみ登録して再生させる。
			if (m_currentTime <= BLINK_THRESHOLD && m_currentTime > 0.0f)
			{
				if (!m_isRotAnimationPlaying)
				{
					auto rotAnim = std::make_unique<UIRotationAnimation>();

					rotAnim->SetParameter(
						baseAngle - SLOPE_AMP
						, baseAngle + SLOPE_AMP
						, 1.0f / SLOPE_SPEED
						, util::EasingType::Linear
						, util::LoopMode::PingPong
					);

					// 回転アニメーションの登録。
					clock->AddAnimation(Hash32("ClockRotAnim"), std::move(rotAnim));
					// 回転アニメーションの再生。
					clock->PlayAnimation();

					// アニメーション再生中は赤色に固定させる。
					const Vector4 redColor(CLOCK_ROT_COLOR);
					clock->m_color = redColor;
					m_isRotAnimationPlaying = true;
				}
				// 10秒を切ったら拡縮アニメーションを登録して再生。
				if (m_currentTime <= LAST_SECONDS && !m_isScaleAnimPlaying)
				{
					// 拡大アニメーションも同時に登録して再生させる。
					auto SetScaleAnim = [&](UIIcon* icon)
						{
							// nullptrチェック。
							if (icon == nullptr)return;

							Vector3 startScale = Vector3::One;
							Vector3 endScale(CLOCK_SCALE_MAX);
							float scaleDuration = CLOCK_SCALE_DURATION;
							auto scaleAnim = std::make_unique<UIScaleAnimation>();
							scaleAnim->SetParameter(
								startScale
								, endScale
								, scaleDuration
								, util::EasingType::Linear
								, util::LoopMode::PingPong
							);

							// UIScaleAnimationの更新関数に回転アニメーションと同じタイマーを参照するラムダをセットする。
							scaleAnim->SetFunc([this, icon](const Vector3& scale)
								{
									if (this->m_currentTime <= LAST_SECONDS && this->m_currentTime > 0.0f)
									{
										icon->m_transform.m_localTransform.m_scale = scale;
									}
								});
							clock->AddAnimation(Hash32("ClockScaleAnim"), std::move(scaleAnim));
							clock->PlayAnimation();
						};
					SetScaleAnim(clock);
					m_isScaleAnimPlaying = true;
				}
			}
			else
			{
				//通常時にアニメーションを停止して通常の傾きに戻す。
				if (m_isRotAnimationPlaying)
				{
					clock->StopAnimation();
					m_slopeTimer = 0.0f;
					m_isRotAnimationPlaying = false;
				}

				// 通常時は残り時間の割合に応じた傾きを直接セットする。
				Quaternion rot;
				rot.SetRotationZ(Math::DegToRad(baseAngle));
				clock->m_color = Vector4::White;
				clock->m_transform.m_localTransform.m_rotation = rot;

				// 通常時に拡縮アニメーションを停止して通常の大きさに戻す。
				if (m_isScaleAnimPlaying)
				{
					clock->StopAnimation();
					clock->m_transform.m_localTransform.m_scale = Vector3::One;
					Vector4 white = Vector4::White;
					clock->m_color = white;
					m_isScaleAnimPlaying = false;
				}
			}
		}
	}
}