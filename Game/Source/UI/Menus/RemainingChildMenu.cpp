/**
 * @file RemainingChildMenu.cpp
 * @brief 子ペンギンの残り数表示クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "RemainingChildMenu.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// 定数。
			const Vector2 DIGIT_PIVOT = Vector2(0.5f, 0.5f);

			// 救助数の座標。
			const Vector3 REMAIN_DIGIT_POS = Vector3(-619.0f, 85.0f, 0.0f);

			// 総数の座標。
			const Vector3 TOTAL_DIGIT_POS = Vector3(-490.0f, 85.0f, 0.0f);
		}

		RemainingChildMenu::RemainingChildMenu()
			: m_childNum(0)
			, m_totalNum(0)
		{
			// アニメーションステータスを生成。
			m_remainAnimStatus = std::make_unique<RemainingAnimStatus>();
			
			// アニメーションステータスのセットアップUIを呼び出す。
			m_remainAnimStatus->SetUpUI();
		}


		void RemainingChildMenu::Update()
		{
			auto* icon = GetUI<UIIcon>(Hash32("ChildPenguinIcon"));
			if (icon) icon->m_isDraw = true;

			auto* slashIcon = GetUI<UIIcon>(Hash32("SlashIcon"));
			if (slashIcon) slashIcon->m_isDraw = true;

			auto* bgIcon = GetUI<UIIcon>(Hash32("BgIcon"));
			if (bgIcon) bgIcon->m_isDraw = true;

			// 残り子ペンギンの数更新
			auto* digit = GetUI<UIDigit>(Hash32("RemainingNum"));
			if (digit) {
				digit->m_isDraw = true;
				digit->SetNumber(m_childNum);
			}

			// ステージ上の総ペンギン数更新
			auto* totalDigit = GetUI<UIDigit>(Hash32("TotalNum"));
			if (totalDigit) {
				totalDigit->m_isDraw = true;
				totalDigit->SetNumber(m_totalNum);
			}

			// 配列の中のアニメーションシーケンスを更新する。
			for (auto& seq : m_sequences)
			{
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
					auto& seq = m_sequences[static_cast<int>(SeqType::Child)];

					// 使う予定のアニメーションを消去。
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_TLANSLATE_DOWN_ANIM_KEY);
					
					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_TLANSLATE_DOWN_ANIM_KEY);

					
					// シーケンスをクリア。
					seq.Clear();

					// シーケンスにアニメーションを追加する。
					seq
						.Add(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY, 0.0f)
						.Add(animKey::RESCUE_REMAIN_TLANSLATE_DOWN_ANIM_KEY, 0.08f);
					
					// シーケンスを再生。
					seq.Play(remainDigit);
				}

				/** 減るアニメーション */
				else if (m_childNum > num)
				{
					// 0番目にアクセス。
					auto& seq = m_sequences[static_cast<int>(SeqType::Child)];

					// アニメーションを消去。
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);
					remainDigit->RemoveAnimation(animKey::RESCUE_REMAIN_SINK_UP_ANIM_KEY);
					
					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);
					UIAnimationFactory::Attach<UITranslateAnimation>(remainDigit, animKey::RESCUE_REMAIN_SINK_UP_ANIM_KEY);


					// シーケンスをクリア。
					seq.Clear();

					// シーケンスにアニメーションを追加。
					seq
						.Add(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY, 0.0f)
						.Add(animKey::RESCUE_REMAIN_SINK_UP_ANIM_KEY, 0.08f);

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
					// 1番目にアクセスする。
					auto& seq = m_sequences[static_cast<int>(SeqType::Total)];

					// アニメーションを消去。
					totalDigit->RemoveAnimation(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);
					totalDigit->RemoveAnimation(animKey::RESCUE_TOTAL_SINK_UP_ANIM_KEY);

					// アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(totalDigit, animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);
					UIAnimationFactory::Attach<UITranslateAnimation>(totalDigit, animKey::RESCUE_TOTAL_SINK_UP_ANIM_KEY);

					// シーケンスを削除。
					seq.Clear();

					seq
						.Add(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY, 0.0f)
						.Add(animKey::RESCUE_TOTAL_SINK_UP_ANIM_KEY, 0.08f);

					// シーケンスを再生。
					seq.Play(totalDigit);
				}

				m_totalNum = num;
			}
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
			if (digit)
			{
				digit->m_isDraw = false;
			}

			auto* totalDigit = GetUI<UIDigit>(Hash32("TotalNum"));
			if (totalDigit)
			{
				totalDigit->m_isDraw = false;
			}
		}
	}
}