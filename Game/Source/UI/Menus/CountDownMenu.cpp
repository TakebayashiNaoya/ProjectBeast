/**
 * @file CountDownMenu.cpp
 * @brief カウントダウンの動的処理クラス
 */
#include "stdafx.h"
#include "CountDownMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			struct CountDownInfo
			{
				uint32_t key;
				EnCountDownType type;
			};

			const Vector4 START_COLOR_VALUE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
			const Vector4 END_COLOR_VALUE = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
			const Vector3 START_SCALE_VALUE = Vector3::Zero;
			const Vector3 END_SCALE_VALUE = Vector3(1.5f, 1.5f, 1.5f);
			constexpr float ANIM_FIRST = 0.2f;
			constexpr float ANIM_SECOND = 1.0f;
			constexpr float ANIM_THIRD = 1.0f;


			// 割るだけの値。
			constexpr float TIME_VALUE = 10.0f;
			// カウントダウンの時間の変化の値。
			constexpr float COUNT_DOWN_DELTA_VALUE = 4.0f;

			// カウントダウン前の遅延時間。
			constexpr float COUNT_DOWN_DELAY_TIME = 3.0f;

			// 初期化時の値。
			constexpr float TIME_VALUE_FOUR = 4.0f;

			// 3秒。
			constexpr float TIME_VALUE_TRHIRD = 3.0f;
			// 2秒。
			constexpr float TIME_VALUE_SECOND = 2.0f;
			// 1秒。
			constexpr float TIME_VALUE_FIRST = 1.0f;
			// 終了の値。
			constexpr float COUNT_TIME_UP = 0.0f;

			// 要素数。
			constexpr int COUNT_DOWN_ICON_SIZE = static_cast<int>(EnCountDownType::Max);
			constexpr CountDownInfo COUNT_DOWN_ICON_KEYS[COUNT_DOWN_ICON_SIZE] =
			{
					{ Hash32("ThirdCountDownTypeIcon"), EnCountDownType::Third  }  // 3
				,	{ Hash32("SecondCountDownTypeIcon"),EnCountDownType::Second }  // 2
				,	{ Hash32("FirstCountDownTypeIcon"), EnCountDownType::First  }  // 1
				,	{ Hash32("GoCountDownTypeIcon"),	EnCountDownType::GO     }  // GO
			};
		}


		CountDownIcon::CountDownIcon(EnCountDownType type)
			: m_icon(nullptr)
			, m_type(type)
		{}


		CountDownIcon::~CountDownIcon()
		{}


		void CountDownIcon::Update()
		{
			if (!m_icon)
			{
				return;
			}
		}


		void CountDownIcon::SetUIIcon(UIIcon* icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗です。");
		}





		/********************************************/


		CountDownMenu::CountDownMenu()
			: m_currentCountType(EnCountDownType::None)
			, m_time(TIME_VALUE_FOUR)
			, m_countDownStartFlag(false)
			, m_countDownFinishedFlag(false)
			, m_isDelayStart(false)
			, m_delayTime(0.0f)
		{}


		void CountDownMenu::Update()
		{
			// ディレイ中はカウントダウンの処理は走らせない。
			if (m_isDelayStart)
			{
				// 1フレームの経過時間を取得。
				m_delayTime += g_gameTime->GetFrameDeltaTime();
				// ディレイ時間が経過したら、カウントダウンを開始。
				if (m_delayTime >= COUNT_DOWN_DELAY_TIME)
				{
					m_isDelayStart = false;
					m_countDownStartFlag = true;
					m_delayTime = 0.0f;
				}
				else
				{
					for (const auto& icon : m_countDownMap)
					{
						icon.second->SetIsDraw(false);
					}
					return;
				}
			}

			if (!m_countDownStartFlag)
			{
				for (const auto& icon : m_countDownMap)
				{
					icon.second->SetIsDraw(false);
				}
				m_time = TIME_VALUE_FOUR;
				return;
			}

			//　時間が減る処理。
			CalcCount();

			// 更新前の状態を保存。
			EnCountDownType previewType = m_currentCountType;
			// ここで新しい状態に更新。
			m_currentCountType = GetCurrentCountType();

			for (const auto& icon : m_countDownMap)
			{
				icon.second->SetIsDraw(icon.second->GetType() == m_currentCountType);
				// カウントダウンアイコンの更新。
				icon.second->Update();
			}

			// 状態が切り替わった瞬間に
			if (previewType != m_currentCountType && m_currentCountType != EnCountDownType::None)
			{
				// 切り替わった今の状態に対応するキーを
				for (const auto& info : COUNT_DOWN_ICON_KEYS)
				{
					// 今のタイプと同じなら
					if (info.type == m_currentCountType)
					{
						auto* icon = GetUI<UIIcon>(info.key);
						if (icon)
						{
							// 登録されているアニメーションを最初から再生する。
							icon->PlayAnimation();
						}
						// 見つかったらループを抜ける。
						break;
					}
				}
			}
			// Menuの更新。
			CountDownClass::Update();
		}


		void CountDownMenu::CalcCount()
		{
			// 1フレームの経過時間を取得。
			float deltaTime = g_gameTime->GetFrameDeltaTime();
			// カウントダウンの時間を減らす処理。
			m_time -= deltaTime;
			// 時間の制限の設定。
			m_time = util::clamp(m_time, COUNT_TIME_UP, TIME_VALUE_FOUR);
		}


		void CountDownMenu::ResetCountDown()
		{
			m_time = TIME_VALUE_FOUR;
			m_isDelayStart = false;
			m_countDownStartFlag = false;
			m_countDownFinishedFlag = false;
			m_currentCountType = EnCountDownType::None;
		}


		EnCountDownType CountDownMenu::GetCurrentCountType()
		{
			if (!m_countDownStartFlag)
			{
				return EnCountDownType::None;
			}
			if (m_time > TIME_VALUE_TRHIRD)return EnCountDownType::Third;
			if (m_time > TIME_VALUE_SECOND)return EnCountDownType::Second;
			if (m_time > TIME_VALUE_FIRST)return EnCountDownType::First;
			if (m_time > COUNT_TIME_UP)return EnCountDownType::GO;

			m_countDownStartFlag = false;
			m_countDownFinishedFlag = true;
			return EnCountDownType::None;
		}


		void CountDownMenu::InitializeLogic()
		{
			// ダンぐリングポインタを防ぐために、マップをクリア。
			m_countDownMap.clear();
			// カウントダウンアイコンの数だけ、マップの容量を確保。
			m_countDownMap.reserve(COUNT_DOWN_ICON_SIZE);

			for (const auto& info : COUNT_DOWN_ICON_KEYS)
			{
				// カウントダウンアイコンの生成。
				Icon countIcon = std::make_unique<CountDownIcon>(info.type);
				// UIからIconを取得。
				auto* icon = GetUI<UIIcon>(info.key);
				// UIからIconを取得して、カウントダウンアイコンに設定。
				countIcon->SetUIIcon(GetUI<UIIcon>(info.key));
				// マップにカウントダウンアイコンを追加。
				m_countDownMap.emplace(info.key, std::move(countIcon));


				if (icon)
				{
					if (info.type == EnCountDownType::GO)
					{
						// 拡縮のアニメーションを作成。
						auto scaleAnim = std::make_unique<app::ui::UIScaleAnimation>();

						// アニメーションのパラメーターを設定。
						// 最初は大きさが変わらない。
						Vector3 startScale(START_SCALE_VALUE);
						// 最終的な大きさ。
						Vector3 endScale(END_SCALE_VALUE);
						// 間隔。
						float duration = ANIM_FIRST;

						// アニメーションの設定。
						scaleAnim->SetParameter(
							startScale
							, endScale
							, duration
							, util::EasingType::Linear
							, util::LoopMode::Once
						);

						// GOアイコンにアニメーションを登録。
						icon->AddAnimation(Hash32("ScaleAnim"), std::move(scaleAnim));

						// フェードアウトするアニメーションを作成。
						auto colorAnim = std::make_unique<UIColorAnimation>();

						Vector4 startColor(START_COLOR_VALUE);
						Vector4 endColor(END_COLOR_VALUE);
						float durationColor = ANIM_THIRD;

						// アニメーションの設定。
						colorAnim->SetParameter(
							startColor
							, endColor
							, durationColor
							, util::EasingType::Linear
							, util::LoopMode::Once
						);

						// GOアイコンにアニメーションを登録。
						icon->AddAnimation(Hash32("ColorAnim"), std::move(colorAnim));
					}

					else
					{
						// シーンのフェードを使わないで、UIAnimationだけでフェードを行う。
						auto colorAnim = std::make_unique<UIColorAnimation>();
						Vector4 startColor(START_COLOR_VALUE);
						Vector4 endColor(END_COLOR_VALUE);
						float durationColor = ANIM_THIRD;

						// アニメーションの設定。
						colorAnim->SetParameter(
							startColor
							, endColor
							, durationColor
							, util::EasingType::Linear
							, util::LoopMode::Once
						);
						// 3,2,1のアイコンにアニメーションを登録。
						icon->AddAnimation(Hash32("ColorAnimation"), std::move(colorAnim));
					}
				}
			}
		}
	}
}

