/**
 * @file InGameButtonMenu.cpp
 * @brief インゲーム中にボタンメニューを表示するクラス
 * @author 立山
 */
#include "stdafx.h"
#include "InGameButtonMenu.h"
#include "Source/Util/CRC32.h"

namespace app
{
	namespace ui
	{
		InGameButtonMenu::InGameButtonMenu()
			: m_isStartedGameStartingAnimation(false)
			, m_isPlayingGameStartingAnimation(false)
			, m_isFinishedGameStartingAnimation(false)
		{}

		void InGameButtonMenu::Update()
		{
			// ゲーム開始時のアニメーション
			if (!m_isFinishedGameStartingAnimation)
			{
				UpdateGameStartingAnimation();
			}

			ButtonIconUpdate();
			MenuBase::Update();
		}


		void InGameButtonMenu::UpdateGameStartingAnimation()
		{
			const char* iconNames[] = {
				"NotInputJumpIcon", "NotInputSneakIcon", "NotInputSlideIcon", "NotInputOrderIcon",
				"InputJumpIcon",    "InputSneakIcon",    "InputSlideIcon",    "InputOrderIcon",
				"NotInputAbuttonIcon", "NotInputBbuttonIcon", "NotInputXbuttonIcon", "NotInputYbuttonIcon",
				"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon"
			};

			std::array<UIBase*, std::size(iconNames)> uiParts;

			for (size_t i = 0; i < std::size(iconNames); ++i)
			{
				uiParts.at(i) = GetUI<UIIcon>(Hash32(iconNames[i]));
			}

			auto ForEach = [&](auto&& func)
				{
					for (const auto& ui : uiParts)
					{
						if (ui == nullptr)continue;
						func(ui);
					}
				};


			// 状態に応じた処理

			// アニメーションが開始されていなければ
			if (!m_isStartedGameStartingAnimation)
			{
				constexpr float posZ = 0.0f;

				ForEach([&](UIBase* ui)
					{
						// アニメーションのオフセット値
						// これだけ右側からアニメーションスタートする
						constexpr float offsetX = 300.0f;

						// jsonから登録済みのポジションを取得
						const Vector3 jsonPos = ui->m_transform.m_localTransform.m_position;

						// jsonの設定位置より右側からアニメーションスタート
						const Vector3 startPos = Vector3(jsonPos.x + offsetX, jsonPos.y, posZ);
						// 最終的にjsonの設定位置が終点
						const Vector3 endPos = jsonPos;
						// 各UIに対して、フェードインアニメーションを登録して再生させる。
						auto trsAnim = std::make_unique<UITranslateAnimation>();
						trsAnim->SetParameter(
							startPos,
							endPos,
							1.0f,
							util::EasingType::EaseInOut,
							util::LoopMode::Once
						);
						trsAnim->SetFunc([ui](const Vector3& pos)
							{
								ui->m_transform.m_localTransform.m_position = pos;
							});
						ui->AddAnimation(Hash32("GameStartFadeIn"), std::move(trsAnim));
						ui->PlayAnimation();
					});

				m_isStartedGameStartingAnimation = true;
				m_isPlayingGameStartingAnimation = true;
			}
			// アニメーションが再生中であれば
			else if (m_isPlayingGameStartingAnimation)
			{
				bool allFinished = true;
				ForEach([&](UIBase* ui)
					{
						if (ui->IsPlayAnimation())
							allFinished = false;
					});

				if (allFinished)
				{
					m_isPlayingGameStartingAnimation = false;
					m_isFinishedGameStartingAnimation = true;
				}
			}
			// アニメーションが終了していれば
			else if (m_isFinishedGameStartingAnimation)
			{
				// アニメーションが終了したら、通常の更新処理に移行する。
				return;
			}
			else
			{
				assert(false && "想定されない状態");
				return;
			}
		}


		void InGameButtonMenu::ButtonIconUpdate()
		{
			// UI表示を切り替えるラムダ式（ローカル関数）
			auto updateUI = [&](bool isInput, const char* notInputAct, const char* inputAct, const char* notInputBtn, const char* inputBtn) {
				if (auto* ui = GetUI<UIIcon>(Hash32(notInputAct))) ui->m_isDraw = !isInput;
				if (auto* ui = GetUI<UIIcon>(Hash32(inputAct)))    ui->m_isDraw = isInput;
				if (auto* ui = GetUI<UIIcon>(Hash32(notInputBtn))) ui->m_isDraw = !isInput;
				if (auto* ui = GetUI<UIIcon>(Hash32(inputBtn)))    ui->m_isDraw = isInput;
				};

			// 各ボタンに対して一括処理
			updateUI(IsInputAButton(), "NotInputJumpIcon", "InputJumpIcon", "NotInputAbuttonIcon", "InputAbuttonIcon");
			updateUI(IsInputBButton(), "NotInputSneakIcon", "InputSneakIcon", "NotInputBbuttonIcon", "InputBbuttonIcon");
			updateUI(IsInputXButton(), "NotInputSlideIcon", "InputSlideIcon", "NotInputXbuttonIcon", "InputXbuttonIcon");
			updateUI(IsInputYButton(), "NotInputOrderIcon", "InputOrderIcon", "NotInputYbuttonIcon", "InputYbuttonIcon");
		}

		void InGameButtonMenu::InitializeLogic()
		{
			// 初期化対象のアイコン名リスト
			const char* iconNames[] = {
				"NotInputJumpIcon", "NotInputSneakIcon", "NotInputSlideIcon", "NotInputOrderIcon",
				"InputJumpIcon",    "InputSneakIcon",    "InputSlideIcon",    "InputOrderIcon",
				"NotInputAbuttonIcon", "NotInputBbuttonIcon", "NotInputXbuttonIcon", "NotInputYbuttonIcon",
				"InputAbuttonIcon",    "InputBbuttonIcon",    "InputXbuttonIcon",    "InputYbuttonIcon"
			};

			// ループですべての該当アイコンを非表示(false)にする
			for (const char* name : iconNames)
			{
				if (auto* ui = GetUI<UIIcon>(Hash32(name)))
				{
					ui->m_isDraw = false;
				}
			}
		}

		bool InGameButtonMenu::IsInputAButton() const
		{
			return g_pad[0]->IsPress(enButtonA);
		}

		bool InGameButtonMenu::IsInputBButton() const
		{
			return g_pad[0]->IsPress(enButtonB);
		}

		bool InGameButtonMenu::IsInputXButton() const
		{
			return g_pad[0]->IsPress(enButtonX);
		}

		bool InGameButtonMenu::IsInputYButton() const
		{
			return g_pad[0]->IsPress(enButtonY);
		}
	}
}