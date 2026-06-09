/**
 * @file RemainingChildMenu.cpp
 * @brief 子ペンギンの残り数表示クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "RemainingChildMenu.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		RemainingChildMenu::RemainingChildMenu()
			: m_childNum(0)
			, m_totalNum(0)
		{
			// アニメーションステータスを生成。
			m_remainAnimStatus = std::make_unique<RemainingAnimStatus>();

			// アニメーションステータスのセットアップUIを呼び出す。
			m_remainAnimStatus->SetUp();
		}


		void RemainingChildMenu::Update()
		{
			if (!m_startingAnimLogic.IsAnimationStarted())
			{
				// ゲーム開始時のアニメーションを更新する。
				m_startingAnimLogic.Initialize(
					this,
					{ "ChildPenguinIcon", "SlashIcon", "BgIcon" },
					{ "RemainingNum", "TotalNum" },
					Vector3(-400.0f, 0.0f, 0.0f)
				);
			}
			if (!m_startingAnimLogic.IsAnimationFinished())
			{
				m_startingAnimLogic.Update();
			}

			// 開始アニメーション中は非表示のまま保つ。
			// アニメーション完了前に救助が発生しても、UIが突然現れるバグを防ぐ。
			const bool isVisible = m_startingAnimLogic.IsAnimationFinished();

			auto* icon = GetUI<UIIcon>(Hash32("ChildPenguinIcon"));
			if (icon) icon->m_isDraw = isVisible;

			auto* slashIcon = GetUI<UIIcon>(Hash32("SlashIcon"));
			if (slashIcon) slashIcon->m_isDraw = isVisible;

			auto* bgIcon = GetUI<UIIcon>(Hash32("BgIcon"));
			if (bgIcon) bgIcon->m_isDraw = isVisible;

			// 残り子ペンギンの数更新
			auto* digit = GetUI<UIDigit>(Hash32("RemainingNum"));
			if (digit) {
				digit->m_isDraw = isVisible;
				digit->SetNumber(m_childNum);
			}

			// ステージ上の総ペンギン数更新
			auto* totalDigit = GetUI<UIDigit>(Hash32("TotalNum"));
			if (totalDigit) {
				totalDigit->m_isDraw = isVisible;
				totalDigit->SetNumber(m_totalNum);
			}

			// 配列の中のアニメーションシーケンスを更新する。
			for (auto& seq : m_sequences)
			{
				// シーケンスが再生中なら更新。
				if (seq.IsPlaying())
				{
					seq.Update(g_gameTime->GetFrameDeltaTime());
				}
			}

			// Menuの更新。
			MenuBase::Update();
		}


		void RemainingChildMenu::SetChildNum(const int num)
		{
			// UIDigitを取得。
			auto* remainDigit = GetUI<UIDigit>(Hash32("RemainingNum"));

			// Digitがなかったら処理しない。
			if (!remainDigit) return;

			// 今の救助数と異なっていたら
			if (m_childNum != num)
			{
				/** 増えるアニメーション */
				if (m_childNum < num)
				{
					// enumの0番目にアクセスする。
					auto& seq = m_sequences[static_cast<int>(SeqType::RemainPlus)];

					// 救助数増加アニメーションを消去。
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY);

					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);
					// 座標アニメーションを登録。(バウンドあり)
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY);


					// シーケンスをクリア。
					seq.Clear();

					// メソッドチェーンシーケンスにアニメーションとSEを追加。
					seq
						// アニメーション開始時にSEを再生する。
						.Add(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY, 0.0f
							, [remainDigit]()
							{
								// SEを再生。
								SoundManager::Get().PlaySE(enSoundKind_RemainPlus);
							}
							, []() {})
						// アニメーション完了後にSEを停止する。
						.Add(animKey::RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY, 0.0f
							, []() {}
							, []() {}
						);

					// シーケンスを再生。
					seq.Play(remainDigit);
				}

				/** 減るアニメーション */
				else if (m_childNum > num)
				{
					// 1番目にアクセス。
					auto& seq = m_sequences[static_cast<int>(SeqType::RemainMinus)];

					// 救助数減少アニメーションを消去。
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY);

					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);
					// 座標アニメーションを登録。(バウンドなし)
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY);


					// シーケンスをクリア。
					seq.Clear();

					// メソッドチェーンシーケンスにアニメーションとSEを追加。
					seq
						// アニメーション開始時にSEを再生する。
						.Add(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY, 0.0f
							, [remainDigit]()
							{
								// SEを再生。
								SoundManager::Get().PlaySE(enSoundKind_RemainORTotalMinus);
							},
							[]() {})
						// アニメーション完了後にSEを停止する。
						.Add(animKey::RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY, 0.0f
							, []() {}
							, []() {}
						);

					// シーケンスを再生。
					seq.Play(remainDigit);
				}

				m_childNum = num;
			}
		}


		void RemainingChildMenu::SetTotalNum(const int num)
		{
			// 総数のDigitを取得。
			auto* totalDigit = GetUI<UIDigit>(Hash32("TotalNum"));

			// Digitがないなら処理しない。
			if (!totalDigit) return;

			// 今の総数と異なっていたら
			if (m_totalNum != num)
			{
				/** 減るアニメーション */
				if (m_totalNum > num)
				{
					// 2番目にアクセスする。
					auto& seq = m_sequences[static_cast<int>(SeqType::TotalMinus)];

					// 使うアニメーションキーを消去。
					totalDigit->RemoveAnimation(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);
					totalDigit->RemoveAnimation(animKey::RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY);

					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(totalDigit, animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);
					// 座標アニメーションを登録。(バウンドなし)
					UIAnimationFactory::Attach<UITranslateAnimation>(totalDigit, animKey::RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY);

					// シーケンスを削除。
					seq.Clear();

					// メソッドチェーンシーケンスにアニメーションを追加する。
					seq
						// アニメーション開始時にSEを再生する。
						.Add(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY, 0.0f
							, [totalDigit]()
							{
								// SEを再生。
								SoundManager::Get().PlaySE(enSoundKind_RemainORTotalMinus);
							}
							, []() {})
						// アニメーション完了後にSEを停止する。
						.Add(animKey::RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY, 0.0f
							, []() {}
							, []() {}
						);

					// シーケンスを再生。
					seq.Play(totalDigit);
				}

				m_totalNum = num;
			}
		}


		void RemainingChildMenu::UpdateGameStartingAnimation()
		{
			const char* iconNames[] = {
				"ChildPenguinIcon",
				"SlashIcon",
				"BgIcon"
			};

			const char* digitNames[] = {
				"RemainingNum",
				"TotalNum"
			};
		}


		void RemainingChildMenu::InitializeLogic()
		{
			// 生成直後は全て非表示にする（UIBaseのデフォルトがm_isDraw=trueのwため）
			auto* icon = GetUI<UIIcon>(Hash32("ChildPenguinIcon"));
			if (icon) icon->m_isDraw = false;

			auto* slashIcon = GetUI<UIIcon>(Hash32("SlashIcon"));
			if (slashIcon) slashIcon->m_isDraw = false;

			auto* bgIcon = GetUI<UIIcon>(Hash32("BgIcon"));
			if (bgIcon) bgIcon->m_isDraw = false;

			auto* digit = GetUI<UIDigit>(Hash32("RemainingNum"));
			if (digit) digit->m_isDraw = false;

			auto* totalDigit = GetUI<UIDigit>(Hash32("TotalNum"));
			if (totalDigit) totalDigit->m_isDraw = false;
		}
	}
}