/**
 * @file RemainingChildMenu.cpp
 * @brief 子ペンギンの残り数表示クラス
 * @author 忽那
 */
#include "stdafx.h"
#include "graphics/Camera/CameraSystem.h"
#include "RemainingChildMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Vfx/HomingParticleRender.h"


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
			auto* text = GetUI<UIText>(Hash32("RemainingNum"));
			if (text) {
				text->m_isDraw = isVisible;
				text->SetText(std::to_string(m_childNum));
			}

			// ステージ上の総ペンギン数更新
			auto* totalText = GetUI<UIText>(Hash32("TotalNum"));
			if (totalText) {
				totalText->m_isDraw = isVisible;
				totalText->SetText(std::to_string(m_totalNum));
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

			// パーティクルエフェクトを更新。
			if (m_homingRender)
			{
				m_homingRender->Update();
			}

			// Menuの更新。
			MenuBase::Update();
		}


		void RemainingChildMenu::Render(RenderContext& rc)
		{
			// メニューの描画。
			MenuBase::Render(rc);

			// パーティクルエフェクトの描画。
			if (m_homingRender)
			{
				m_homingRender->Render(rc);
			}
		}


		void RemainingChildMenu::SetChildNum(const int num)
		{
			// UITextを取得。
			auto* remainText = GetUI<UIText>(Hash32("RemainingNum"));


			if (!remainText) return;

			// 今の救助数と異なっていたら
			if (m_childNum != num)
			{
				/** 増えるアニメーション */
				if (m_childNum < num)
				{
					// enumの0番目にアクセスする。
					auto& seq = m_sequences[static_cast<int>(SeqType::RemainPlus)];

					// 救助数増加アニメーションを消去。
					remainText->RemoveAnimation(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);
					remainText->RemoveAnimation(animKey::RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY);

					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(remainText, animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY);
					// 座標アニメーションを登録。(バウンドあり)
					UIAnimationFactory::Attach<UITranslateAnimation>(remainText, animKey::RESCUE_REMAIN_BOUNCE_DOWN_ANIM_KEY);

					// シーケンスをクリア。
					seq.Clear();

					// メソッドチェーンシーケンスにアニメーションとSEを追加。
					seq
						// アニメーション開始時にSEを再生する。
						.Add(animKey::RESCUE_REMAIN_TLANSLATE_UP_ANIM_KEY, 0.0f
							, [remainText, this]()
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
					seq.Play(remainText);
				}

				/** 減るアニメーション */
				else if (m_childNum > num)
				{
					// 1番目にアクセス。
					auto& seq = m_sequences[static_cast<int>(SeqType::RemainMinus)];

					// 救助数減少アニメーションを消去。
					remainText->RemoveAnimation(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);
					remainText->RemoveAnimation(animKey::RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY);

					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(remainText, animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY);
					// 座標アニメーションを登録。(バウンドなし)
					UIAnimationFactory::Attach<UITranslateAnimation>(remainText, animKey::RESCUE_REMAIN_BOUNCE_DOWN_UP_ANIM_KEY);


					// シーケンスをクリア。
					seq.Clear();

					// メソッドチェーンシーケンスにアニメーションとSEを追加。
					seq
						// アニメーション開始時にSEを再生する。
						.Add(animKey::RESCUE_REMAIN_SINK_DOWN_ANIM_KEY, 0.0f
							, [remainText]()
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
					seq.Play(remainText);
				}

				m_childNum = num;
			}
		}


		void RemainingChildMenu::SetTotalNum(const int num)
		{
			auto* totalText = GetUI<UIText>(Hash32("TotalNum"));

			if (!totalText) return;

			// 今の総数と異なっていたら
			if (m_totalNum != num)
			{
				/** 減るアニメーション */
				if (m_totalNum > num)
				{
					// 2番目にアクセスする。
					auto& seq = m_sequences[static_cast<int>(SeqType::TotalMinus)];

					// 使うアニメーションキーを消去。
					totalText->RemoveAnimation(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);
					totalText->RemoveAnimation(animKey::RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY);

					// 座標アニメーションを登録。
					UIAnimationFactory::Attach<UITranslateAnimation>(totalText, animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY);
					// 座標アニメーションを登録。(バウンドなし)
					UIAnimationFactory::Attach<UITranslateAnimation>(totalText, animKey::RESCUE_TOTAL_BOUNCE_DOWN_UP_ANIM_KEY);

					// シーケンスを削除。
					seq.Clear();

					// メソッドチェーンシーケンスにアニメーションを追加する。
					seq
						// アニメーション開始時にSEを再生する。
						.Add(animKey::RESCUE_TOTAL_SINK_DOWN_ANIM_KEY, 0.0f
							, [totalText]()
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
					seq.Play(totalText);
				}

				m_totalNum = num;
			}
		}


		void RemainingChildMenu::SetTarget(actor::ChildPenguin* childPenguin)
		{
			m_homingRender->AddTarget(childPenguin);
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
			// パーティクルエフェクトレンダーを生成・初期化する。
			m_homingRender = std::make_unique<HomingParticleRender>();
			m_homingRender->Initialize();

			// 生成直後は全て非表示にする（UIBaseのデフォルトがm_isDraw=trueのwため）
			auto* icon = GetUI<UIIcon>(Hash32("ChildPenguinIcon"));
			if (icon) icon->m_isDraw = false;

			auto* slashIcon = GetUI<UIIcon>(Hash32("SlashIcon"));
			if (slashIcon) slashIcon->m_isDraw = false;

			auto* bgIcon = GetUI<UIIcon>(Hash32("BgIcon"));
			if (bgIcon) bgIcon->m_isDraw = false;

			auto* text = GetUI<UIText>(Hash32("RemainingNum"));
			if (text)
			{
				text->m_isDraw = false;
				m_homingRender->SetGoalPosition(text->m_transform.m_localTransform.m_position);
			}

			auto* totalText = GetUI<UIText>(Hash32("TotalNum"));
			if (totalText) totalText->m_isDraw = false;

			//auto* flashEffectA = GetUI<UIDummy>(Hash32("YellowFlash"));
			//if (flashEffectA) flashEffectA->m_isDraw = false;
		}
	}
}