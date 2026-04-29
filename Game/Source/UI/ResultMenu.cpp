/**
 * @file ResultMenu.cpp
 * @brief リザルトのUI動的処理・アニメーション管理クラス
 * @author 立山
 */
#include "stdafx.h"

#include "ResultMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/CRC32.h"


namespace
{
	// ---------------------------------------------------------
	// 色設定（フォントカラー）
	// ---------------------------------------------------------
	const Vector4 COLOR_DIGIT_TIME_SCORE = { 0.0f, 0.3f, 1.0f, 1.0f }; // シアン（例）
	const Vector4 COLOR_DIGIT_TOTAL = { 1.0f, 1.0f, 0.0f, 1.0f }; // 黄色

	// アニメーション用タイマー
	constexpr float CHECK_REVEAL_DELAY = 1.0f;
	constexpr float CHECK_REVEAL_INTERVAL = 1.0f;
	constexpr float TOTAL_REVEAL_DELAY = 1.0f;
	constexpr float TITLE_BUTTON_DELAY = 1.5f;

	// アチーブメント動的UI生成用のベース座標とオフセット（※動的生成のためここに定義）
	const Vector3 ACHIEVE_START_POS = { -200.0f, 150.0f, 0.0f };
	constexpr float ACHIEVE_OFFSET_X_CHECK = -250.0f;
	constexpr float ACHIEVE_OFFSET_X_NAME = 210.0f;
	constexpr float ACHIEVE_OFFSET_X_BACK = 210.0f;
	constexpr float ACHIEVE_OFFSET_Y = -80.0f;

	constexpr float ACHIEVE_NAME_OFFSETS_X[] = {
		300.0f, // 0番目: 子ペンギンを50匹以上集めた
		300.0f, // 1番目: 2頭以上のシロクマに追われた
		270.0f, // 2番目: 3頭のシロクマそれぞれに追われた
		300.0f  // 3番目: 眠っているシロクマを起こした
	};

	constexpr float ACHIEVE_NAME_W = 570.0f;
	constexpr float ACHIEVE_NAME_H = 40.0f;
	constexpr float ACHIEVE_BACK_W = 780.0f;
	constexpr float ACHIEVE_BACK_H = 120.0f;
	constexpr float ACHIEVE_BOX_W = 60.0f;
	constexpr float ACHIEVE_BOX_H = 60.0f;
	constexpr float ACHIEVE_CHECK_W = 60.0f;
	constexpr float ACHIEVE_CHECK_H = 60.0f;

	//上部スコア動的生成用
	constexpr float TOP_DIGIT_W = 64.0f;
	constexpr float TOP_DIGIT_H = 80.0f;
	constexpr float TIME_COLON_W = 15.0f;
	constexpr float TIME_COLON_H = 50.0f;
	constexpr float TIME_DIGIT_CENTER_X = -320.0f;  // クリアタイムの基準X
	constexpr float SCORE_DIGIT_CENTER_X = 320.0f;  // 助けた数の基準X
	constexpr float TOP_DIGIT_Y = 280.0f;           // 共通のY座標

	// 合計スコア動的生成用
	constexpr float TOTAL_DIGIT_W = 80.0f;
	constexpr float TOTAL_DIGIT_H = 100.0f;
	constexpr float TOTAL_DIGIT_CENTER_X = 0.0f;
	constexpr float TOTAL_DIGIT_Y = -300.0f;

	// ---------------------------------------------------------
	// 内部計算用定数（マジックナンバー排除用）
	// ---------------------------------------------------------
	constexpr int   SECONDS_PER_MINUTE = 60;   // 1分間の秒数
	constexpr int   SECONDS_DIGIT_COUNT = 2;    // 秒の表示桁数（常に2桁）
	constexpr int   DECIMAL_BASE = 10;   // 10進数計算の基数
	constexpr float CENTER_DIVISOR = 2.0f; // 中央揃え用の分割値
	constexpr float HALF_OFFSET_RATIO = 0.5f; // UI配置用の半幅オフセット
}



namespace app
{
	namespace ui
	{
		ResultMenu::ResultMenu()
			: m_clearTime(0.0f)
			, m_collectedPenguin(0)
			, m_totalScore(0.0f)
			, m_totalDigit(nullptr)
			, m_checkRevealTimer(0.0f)
			, m_checkRevealIndex(0)
			, m_allChecksRevealed(false)
			, m_postCheckTimer(0.0f)
			, m_totalScoreShown(false)
			, m_titleButtonShown(false)
			, m_drumRollHandle(app::INVALID_SE_HANDLE)
		{}


		ResultMenu::~ResultMenu()
		{}


		void ResultMenu::InitializeLogic()
		{}


		void ResultMenu::SetResultData(float clearTime, int collectedPenguin, float totalScore, const std::vector<app::achievement::AchievementBase*>& achievements)
		{
			m_clearTime = clearTime;
			m_collectedPenguin = collectedPenguin;
			m_totalScore = totalScore;
			m_allAchievementList = achievements;

			auto* canvas = GetCanvas();
			if (!canvas) return;

			// タイトルへ戻るテキストは非表示にしておく
			auto* titleBackText = GetUI<UIIcon>(Hash32("TitleBackText"));
			if (titleBackText) titleBackText->m_isDraw = false;

			// ---------------------------------------------------------
			// 桁数を計算するローカル関数（変更）
			// ---------------------------------------------------------
			auto GetDigitCount = [](int number) {
				int digitCount = (number == 0) ? 1 : 0;
				int tmp = number;
				while (tmp > 0)
				{
					tmp /= DECIMAL_BASE;
					digitCount++;
				}
				return digitCount;
				};

			// ---------------------------------------------------------
			// クリアタイムの動的生成（M:SS形式）
			// ---------------------------------------------------------
			int totalSec = static_cast<int>(m_clearTime);
			int minutes = totalSec / SECONDS_PER_MINUTE;
			int seconds = totalSec % SECONDS_PER_MINUTE;

			int minutesDigitCount = GetDigitCount(minutes);
			// 秒は常に「06」のように2桁で表示するため、digitCountは 2 固定

			// 「分」「コロン」「秒」を合わせた全体の幅を計算
			float totalW = (minutesDigitCount * TOP_DIGIT_W) + TIME_COLON_W + (SECONDS_DIGIT_COUNT * TOP_DIGIT_W);

			// 全体が中央揃えになるための「左端」のX座標
			float leftX = TIME_DIGIT_CENTER_X - (totalW / CENTER_DIVISOR);

			// 各パーツの中心X座標を計算（UIDigitは一番右の桁を基準に配置される仕様に合わせる）
			float minutesBaseX = leftX + (minutesDigitCount - HALF_OFFSET_RATIO) * TOP_DIGIT_W;
			float colonCenterX = leftX + (minutesDigitCount * TOP_DIGIT_W) + (TIME_COLON_W / CENTER_DIVISOR);
			float secondsBaseX = leftX + totalW - (TOP_DIGIT_W / CENTER_DIVISOR);

			// ①「分」の生成
			uint32_t minKey = Hash32("ResultTimeMinDigit");
			canvas->CreateUI<UIDigit>(minKey);
			auto* minDigit = canvas->FindUI<UIDigit>(minKey);
			if (minDigit)
			{
				minDigit->Initialize(
					"Assets/spriteData/UI/Number/White",
					minutesDigitCount, minutes,
					TOP_DIGIT_W, TOP_DIGIT_H,
					Vector3(minutesBaseX, TOP_DIGIT_Y, 0.0f),
					Vector3::One, Quaternion::Identity
				);
				minDigit->m_isDraw = true;
			}

			// ②「コロン」の生成
			uint32_t colonKey = Hash32("ResultTimeColonIcon");
			canvas->CreateUI<UIIcon>(colonKey);
			auto* colonIcon = canvas->FindUI<UIIcon>(colonKey);
			if (colonIcon)
			{
				// ★ここを見つかったClone.ddsのパスに変更します！
				colonIcon->Initialize(
					"Assets/spriteData/UI/Icon/InGameTimerIcon/Clone.dds",
					TIME_COLON_W, TIME_COLON_H,
					Vector3(colonCenterX, TOP_DIGIT_Y, 0.0f),
					Vector3::One, Quaternion::Identity, Vector4::White
				);
				colonIcon->m_isDraw = true;
			}

			// ③「秒」の生成
			uint32_t secKey = Hash32("ResultTimeSecDigit");
			canvas->CreateUI<UIDigit>(secKey);
			auto* secDigit = canvas->FindUI<UIDigit>(secKey);
			if (secDigit)
			{
				secDigit->Initialize(
					"Assets/spriteData/UI/Number/White",
					SECONDS_DIGIT_COUNT, seconds, // 常に2桁を指定することで勝手に0埋めされます
					TOP_DIGIT_W, TOP_DIGIT_H,
					Vector3(secondsBaseX, TOP_DIGIT_Y, 0.0f),
					Vector3::One, Quaternion::Identity
				);
				secDigit->m_isDraw = true;
			}

			// ---------------------------------------------------------
			// 助けた数の動的生成
			// ---------------------------------------------------------
			uint32_t scoreKey = Hash32("ResultScoreDigit");
			canvas->CreateUI<UIDigit>(scoreKey);
			auto* scoreDigit = canvas->FindUI<UIDigit>(scoreKey);
			if (scoreDigit)
			{
				int digitCount = GetDigitCount(m_collectedPenguin);
				float baseX = SCORE_DIGIT_CENTER_X + static_cast<float>(digitCount - 1) * TOP_DIGIT_W / CENTER_DIVISOR;

				scoreDigit->Initialize(
					"Assets/spriteData/UI/Number/White",
					digitCount, m_collectedPenguin, // ← 固定の 3 ではなく digitCount を渡す！
					TOP_DIGIT_W, TOP_DIGIT_H,
					Vector3(baseX, TOP_DIGIT_Y, 0.0f),
					Vector3::One, Quaternion::Identity
				);
				scoreDigit->m_isDraw = true;
			}

			SetupAchievementUI();

			m_drumRollHandle = SoundManager::Get().PlaySE(enSoundKind_DrumRoll, false);
		}


		void ResultMenu::Update()
		{
			auto* canvas = GetCanvas();
			if (canvas)
			{
				// 変更箇所：分割した3つのパーツと助けた数を取得
				auto* minDigit = canvas->FindUI<UIDigit>(Hash32("ResultTimeMinDigit"));
				auto* colonIcon = canvas->FindUI<UIIcon>(Hash32("ResultTimeColonIcon"));
				auto* secDigit = canvas->FindUI<UIDigit>(Hash32("ResultTimeSecDigit"));

				auto* scoreDigit = canvas->FindUI<UIDigit>(Hash32("ResultScoreDigit"));

				int totalSec = static_cast<int>(m_clearTime);

				if (minDigit)
				{
					minDigit->m_color = COLOR_DIGIT_TIME_SCORE;
					minDigit->SetNumber(totalSec / SECONDS_PER_MINUTE);
				}
				if (colonIcon)
				{
					colonIcon->m_color = COLOR_DIGIT_TIME_SCORE;
				}
				if (secDigit)
				{
					secDigit->m_color = COLOR_DIGIT_TIME_SCORE;
					secDigit->SetNumber(totalSec % SECONDS_PER_MINUTE);
				}

				if (scoreDigit)
				{
					scoreDigit->m_color = COLOR_DIGIT_TIME_SCORE;
					scoreDigit->SetNumber(m_collectedPenguin);
				}
			}

			UpdateRevealSequence();

			MenuBase::Update();
		}


		void ResultMenu::UpdateRevealSequence()
		{
			const float dt = g_gameTime->GetFrameDeltaTime();

			// フェーズ1：チェックマークを順番に表示
			if (!m_allChecksRevealed)
			{
				if (m_checkRevealIndex < static_cast<int>(m_checkIconList.size()))
				{
					m_checkRevealTimer += dt;
					float threshold = CHECK_REVEAL_DELAY + (CHECK_REVEAL_INTERVAL * static_cast<float>(m_checkRevealIndex));

					if (m_checkRevealTimer >= threshold)
					{
						m_checkIconList[m_checkRevealIndex]->SetIsDraw(true);
						SoundManager::Get().PlaySE(enSoundKind_ResultStamp);
						m_checkRevealIndex++;
					}
				}
				else
				{
					m_allChecksRevealed = true;
					m_postCheckTimer = 0.0f;
				}
				return;
			}

			// フェーズ2：トータルスコアを表示
			if (!m_totalScoreShown)
			{
				m_postCheckTimer += dt;
				if (m_postCheckTimer >= TOTAL_REVEAL_DELAY)
				{
					auto* canvas = GetCanvas();
					if (canvas && !m_totalDigit)
					{
						uint32_t key = Hash32("TotalDigit");
						canvas->CreateUI<UIDigit>(key);
						m_totalDigit = canvas->FindUI<UIDigit>(key);
						if (m_totalDigit)
						{
							int score = static_cast<int>(m_totalScore);
							int digitCount = (score == 0) ? 1 : 0;
							int tmp = score;
							while (tmp > 0)
							{
								tmp /= DECIMAL_BASE;
								digitCount++;
							}

							float baseX = TOTAL_DIGIT_CENTER_X + static_cast<float>(digitCount - 1) * TOTAL_DIGIT_W / CENTER_DIVISOR;

							m_totalDigit->Initialize(
								"Assets/spriteData/UI/Number/White",
								digitCount, score,
								TOTAL_DIGIT_W, TOTAL_DIGIT_H,
								Vector3(baseX, TOTAL_DIGIT_Y, 0.0f),
								Vector3::One, Quaternion::Identity
							);
							m_totalDigit->m_color = COLOR_DIGIT_TOTAL;
							m_totalDigit->m_isDraw = true;
						}
					}

					if (m_drumRollHandle != app::INVALID_SE_HANDLE)
					{
						SoundManager::Get().StopSE(m_drumRollHandle);
						m_drumRollHandle = app::INVALID_SE_HANDLE; // 停止後は無効値に戻しておく
					}

					SoundManager::Get().PlaySE(enSoundKind_Cymbals);
					m_totalScoreShown = true;
					m_postCheckTimer = 0.0f;
				}
				return;
			}

			// フェーズ3：Aボタンガイドを表示（JSONで用意したTitleBackTextのアルファを上げる）
			if (!m_titleButtonShown)
			{

				if (m_totalDigit)
				{
					m_totalDigit->m_color = COLOR_DIGIT_TOTAL;
					m_totalDigit->SetNumber(static_cast<int>(m_totalScore));
					m_totalDigit->m_isDraw = true;
				}

				m_postCheckTimer += dt;
				if (m_postCheckTimer >= TITLE_BUTTON_DELAY)
				{
					auto* titleBackText = GetUI<UIIcon>(Hash32("TitleBackText"));
					if (titleBackText)
					{
						titleBackText->m_isDraw = true;
					}
					m_titleButtonShown = true;
				}
			}
		}


		void ResultMenu::SetupAchievementUI()
		{
			auto* canvas = GetCanvas();
			if (!canvas) return;

			int rowIndex = 0;

			for (size_t i = 0; i < m_allAchievementList.size(); ++i)
			{
				auto* achieve = m_allAchievementList[i];
				if (!achieve) continue;

				float iconOffset = ACHIEVE_OFFSET_Y * static_cast<float>(rowIndex);
				float commonY = ACHIEVE_START_POS.y + iconOffset;

				Vector3 currentIconPos = ACHIEVE_START_POS;
				currentIconPos.x += ACHIEVE_OFFSET_X_CHECK;
				currentIconPos.y = commonY;


				Vector3 currentNamePos = ACHIEVE_START_POS;
				float offsetX = ACHIEVE_OFFSET_X_NAME; // デフォルト値を入れておく
				// 配列の範囲内なら、配列に設定した個別のX座標を使う
				if (i < std::size(ACHIEVE_NAME_OFFSETS_X))
				{
					offsetX = ACHIEVE_NAME_OFFSETS_X[i];
				}
				currentNamePos.x += offsetX;
				currentNamePos.y = commonY;


				Vector3 currentBackPos = ACHIEVE_START_POS;
				currentBackPos.x += ACHIEVE_OFFSET_X_BACK;
				currentBackPos.y = commonY;



				std::string achieveBackKeyName = "AchieveBack_" + std::to_string(i);
				uint32_t achieveBackKey = Hash32(achieveBackKeyName.c_str());
				canvas->CreateUI<UIIcon>(achieveBackKey);
				auto* achieveBack = canvas->FindUI<UIIcon>(achieveBackKey);
				if (achieveBack)
				{
					achieveBack->Initialize("Assets/spriteData/UI/Achievement/achievementBack.DDS", ACHIEVE_BACK_W, ACHIEVE_BACK_H, currentBackPos, Vector3::One, Quaternion::Identity, Vector4::White);
					achieveBack->m_isDraw = true;
				}


				std::string nameAssetPath = "Assets/spriteData/UI/Achievement/AchieveName_/" + achieve->GetSpriteName() + ".DDS";
				std::string nameKeyName = "AchieveName_" + std::to_string(i);
				uint32_t nameKey = Hash32(nameKeyName.c_str());
				canvas->CreateUI<UIIcon>(nameKey);
				auto* nameIcon = canvas->FindUI<UIIcon>(nameKey);
				if (nameIcon)
				{
					nameIcon->Initialize(nameAssetPath.c_str(), ACHIEVE_NAME_W, ACHIEVE_NAME_H, currentNamePos, Vector3::One, Quaternion::Identity, Vector4::White);
					nameIcon->m_isDraw = true;
				}


				std::string checkBoxKeyName = "AchieveCheckBox_" + std::to_string(i);
				uint32_t checkBoxKey = Hash32(checkBoxKeyName.c_str());
				canvas->CreateUI<UIIcon>(checkBoxKey);
				auto* checkBoxIcon = canvas->FindUI<UIIcon>(checkBoxKey);
				if (checkBoxIcon)
				{
					checkBoxIcon->Initialize("Assets/spriteData/UI/Achievement/checkBox.DDS", ACHIEVE_BOX_W, ACHIEVE_BOX_H, currentIconPos, Vector3::One, Quaternion::Identity, Vector4::White);
					checkBoxIcon->m_isDraw = true;
				}



				if (achieve->IsAchieved())
				{
					std::string checkKeyName = "AchieveCheck_" + std::to_string(i);
					uint32_t checkKey = Hash32(checkKeyName.c_str());

					canvas->CreateUI<UIIcon>(checkKey);
					auto* checkIcon = canvas->FindUI<UIIcon>(checkKey);
					if (checkIcon)
					{
						checkIcon->Initialize("Assets/spriteData/UI/Achievement/check.DDS", ACHIEVE_CHECK_W, ACHIEVE_CHECK_H, currentIconPos, Vector3::One, Quaternion::Identity, Vector4::White);
						checkIcon->m_isDraw = false;
						m_checkIconList.push_back(checkIcon);
					}
				}
				rowIndex++;
			}
		}
	}
}