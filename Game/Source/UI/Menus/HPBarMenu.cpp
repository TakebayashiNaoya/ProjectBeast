/**
 * @file HPBarMenu.cpp
 * @brief 親ペンギンのHPバーの動的処理クラス
 */
#include "stdafx.h"
#include "HPBarMenu.h"
#include "Source/Util/CRC32.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// 最小HP。
			constexpr float MIN_HP = 0.0f;
			// ダメージ量。
			constexpr float DAMAGE = 10.0f;

			// 範囲制限のみ活用。
			constexpr float CLAMP_MAX = 1.0f;

			struct HPInfo
			{
				uint32_t key;
				EnHPType type;
			};
			// 要素数。
			constexpr uint8_t HP_BAR_ICON_SIZE = static_cast<uint8_t>(EnHPType::Max);
			// keyとtypeの配列。
			constexpr HPInfo HP_BAR_ICON_KEYS[HP_BAR_ICON_SIZE] =
			{
				{ Hash32("HPGrayIcon"), EnHPType::Gray },
				{ Hash32("HPRedIcon"),  EnHPType::Red  },
				{ Hash32("HPGreenIcon"),EnHPType::Green}
			};
		}

		HPBarIcon::HPBarIcon(EnHPType type)
			: m_type(type)
			, m_icon(nullptr)
			, m_targetHP(1.0f)
			, m_currentScale(1.0f)
			, m_initialPosX(0.0f)
			, m_width(0.0f)
		{}


		HPBarIcon::~HPBarIcon()
		{}


		void HPBarIcon::Update()
		{
			if (!m_icon)return;
			if (!IsMoving())return;

			float speed = 0.1f;
			if (m_type == EnHPType::Green)speed = 2.0f;
			else if (m_type == EnHPType::Red)speed = 0.08f;
			else if (m_type == EnHPType::Gray)speed = 1.0f;

			if (std::abs(m_currentScale - m_targetHP) > 0.001f)
			{
				const float delta = (m_targetHP - m_currentScale) * speed;
				const float next = m_currentScale + delta;
				if ((m_targetHP - m_currentScale) * (m_targetHP - next) <= 0.0f)
				{
					m_currentScale = m_targetHP;
				}
				else
				{
					m_currentScale = next;
				}
			}
			else
			{
				m_currentScale = m_targetHP;
			}

			m_icon->m_transform.m_localTransform.m_scale.x = m_currentScale;
			float offsetX = (m_width * (1.0f - m_currentScale)) / 2.0f;
			m_icon->m_transform.m_localTransform.m_position.x = m_initialPosX - offsetX;
		}


		void HPBarIcon::SetUIIcon(UIIcon* icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗です。");

			m_initialPosX = m_icon->m_transform.m_localTransform.m_position.x;
			m_width = 420.0f;
		}


		bool HPBarIcon::IsMoving() const
		{
			return std::abs(m_currentScale - m_targetHP) > 0.001f;
		}


		void HPBarIcon::SetTargetHP(float targetHP)
		{
			const float clampedHP = util::clamp(targetHP, MIN_HP, CLAMP_MAX);
			m_targetHP = clampedHP;
		}




		/****************************************/


		HPBarMenu::HPBarMenu()
			: m_currentHPType(EnHPType::Green)
			, m_maxHP(1)
			, m_currentHP(1)
			, m_targetHP(1)
			, m_damage(DAMAGE)
			, m_isGreenMoving(false)
			, m_isRedMoving(false)
			, m_damageRatio(0.0f)
			//, m_prevGreenMoving(false)
			//, m_prevRedMoving(false)
		{}


		HPBarMenu::~HPBarMenu()
		{}


		void HPBarMenu::Update()
		{
			for (auto& icon : m_hpBarIconMap)
			{
				icon.second->Update();
			}

			// 緑のHPバーのキーを変換。
			const uint32_t GreenKey = HP_BAR_ICON_KEYS[(uint8_t)EnHPType::Green].key;
			// 赤のHPバーのキーを変換。
			const uint32_t RedKey = HP_BAR_ICON_KEYS[(uint8_t)EnHPType::Red].key;

			// 緑と赤のHPバーが動いているかどうかを取得。
			bool isGreenNow = m_hpBarIconMap[GreenKey]->IsMoving();
			bool isRedNow = m_hpBarIconMap[RedKey]->IsMoving();

			// 緑のHPバーが動いていない状態から動き始めた場合。
			if (!m_isGreenMoving && isGreenNow)
			{
				m_isGreenMoving = true;
				m_currentHPType = EnHPType::Green;
			}

			// 赤のHPバーが動いていない状態から動き始めた場合。
			if (m_isGreenMoving && !isGreenNow)
			{
				m_isGreenMoving = false;

				auto* red = m_hpBarIconMap[RedKey].get();
				float current = red->GetCurrentScale();
				float target = current - m_damageRatio;
				if (target < 0.0f) target = 0.0f;
				// 赤いバーの目標HPを設定。
				red->SetTargetHP(target);
				// 赤いバーの移動フラグをtrueにする。
				m_isRedMoving = true;
				m_currentHPType = EnHPType::Red;
			}

			// 赤のHPバーが動いていた状態から動き終わった場合。
			if (m_isRedMoving && !isRedNow)
			{
				m_isRedMoving = false;
				// ダメージ量をリセット。
				m_damageRatio = 0.0f;
				m_currentHP = m_targetHP;
			}


			// MenuBaseのUpdateを呼び出す。
			HPBarClass::Update();
		}


		void HPBarMenu::SetTargetHP(float targetHP)
		{
			m_targetHP = targetHP;
		}


		void HPBarMenu::Damage(float damage)
		{
			if (m_isGreenMoving || m_isRedMoving) return;

			const uint32_t GreenKey = HP_BAR_ICON_KEYS[(uint8_t)EnHPType::Green].key;

			auto* green = m_hpBarIconMap[GreenKey].get();

			float current = green->GetCurrentScale();
			// m_maxHPを使ってダメージ割合を計算。
			float step = damage / static_cast<float>(m_maxHP);

			float next = current - step;
			if (next < 0.0f) next = 0.0f;

			m_damageRatio = step;

			green->SetTargetHP(next);

			m_isGreenMoving = true;
		}


		void HPBarMenu::InitializeLogic()
		{
			m_hpBarIconMap.clear();

			m_hpBarIconMap.reserve(HP_BAR_ICON_SIZE);
			for (const auto& info : HP_BAR_ICON_KEYS)
			{
				Icon hpBarIcon = std::make_unique<HPBarIcon>(info.type);
				auto* icon = GetUI<UIIcon>(info.key);
				hpBarIcon->SetUIIcon(icon);
				icon->m_pivot = Vector2(-1.0f, 0.5f);
				m_hpBarIconMap.emplace(info.key, std::move(hpBarIcon));
			}
		}
	}
}