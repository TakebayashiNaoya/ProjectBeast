/**
 * @file SoundOptionMenu.cpp
 * @brief サウンドのオプションの動的処理群
 * @author 忽那
 */
#include "stdafx.h"
#include "SoundOptionMenu.h"
#include "Source/Sound/SoundManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// keyとenum classの構造体
			struct SoundInfo
			{
				uint32_t key;
				SoundType type;
			};


			/** サウンドオプションの数 */
			constexpr int SOUND_SIZE = 4;
			/** サウンドアイコンの配列 */
			constexpr std::array<SoundInfo, SOUND_SIZE> SOUND_ICON_KEYS =
			{
				{
					{ Hash32("SoundKnobMasterIcon"), SoundType::Master }
				,	{ Hash32("SoundKnobVoiceIcon"),  SoundType::Voice  }
				,	{ Hash32("SoundKnobSEIcon"),      SoundType::SE     }
				,	{ Hash32("SoundKnobBGMIcon"),     SoundType::BGM    }
				}
			};

			/** サウンドDigitの配列 */
			constexpr std::array<SoundInfo, SOUND_SIZE> SOUND_DIGIT_KEYS =
			{
				{
					{ Hash32("SoundMasterDigit"), SoundType::Master }
				,	{ Hash32("SoundVoiceDigit"),  SoundType::Voice  }
				,	{ Hash32("SoundSEDigit"),      SoundType::SE     }
				,	{ Hash32("SoundBGMDigit"),     SoundType::BGM    }
				}
			};

			constexpr std::array<SoundInfo, SOUND_SIZE> SOUND_FRAME_KEYS =
			{
				{
					{ Hash32("SoundMasterFrame"), SoundType::Master }
				,	{ Hash32("SoundVoiceFrame"),  SoundType::Voice  }
				,	{ Hash32("SoundSEFrame"),      SoundType::SE     }
				,	{ Hash32("SoundBGMFrame"),     SoundType::BGM    }
				}
			};


			/** カラーのα値 */
			const Vector4 COLOR_ALPHA = { 0.0f,0.0f,0.0f,0.5f };
			/** カラーアニメーションのスパン */
			constexpr float COLOR_ANIM_DURATION = 0.8f;
			/** 内部的には0 */
			constexpr float MIN_VALUE = 0.0f;
			/** 内部的には100 */
			constexpr float MAX_VALUE = 100.0f;
			/** 全体の音量のリセット値 */
			constexpr float RESET_VOLUME_VALUE = 0.1f;
			/** 音量の最小値 */
			constexpr float MIN_VOLUME_VALUE = 0.0f;
			/** 音量の最大値 */
			constexpr float MAX_VOLUME_VALUE = 1.0f;
			/** 0から1の値を0から100の値に変換するための定数 */
			constexpr float VOLUME_VALUE_CONVERTER = 100.0f;
			/** ポジションの右側の制限値 */
			constexpr float RIGHT_LIMITE = 580.0f;
			/** ポジションの左側の制限値 */
			constexpr float LEFT_LIMITE = -420.0f;
			/** 座標の幅 */
			const float POSITION_RANGE = fabsf(RIGHT_LIMITE - LEFT_LIMITE);
			/** 音量の変化に対するポジションの変化の値 */
			const float VOLUME_STEP = POSITION_RANGE / VOLUME_VALUE_CONVERTER;

			/**
			 * @brief タイプに対応する SoundManager の現在音量を取得する
			 * @param type 音量タイプ
			 * @return 現在の音量（0.0f 〜 1.0f）
			 */
			float GetCurrentVolumeFromManager(SoundType type)
			{
				const auto& sound = SoundManager::Get();
				switch (type)
				{
				case SoundType::Master: return sound.GetMasterVolume();
				case SoundType::Voice:  return sound.GetVoiceVolume();
				case SoundType::SE:     return sound.GetSEVolume();
				case SoundType::BGM:    return sound.GetBGMVolume();
				default:
					K2_ASSERT(false, "無効なタイプです。");
					return MIN_VOLUME_VALUE;
				}
			}


			/**
			 * @brief タイプに対応する SoundManager の音量を設定する
			 * @param type 音量タイプ
			 * @param volume 音量（0.0f 〜 1.0f）
			 */
			void SetVolumeToManager(SoundType type, float volume)
			{
				volume = util::clamp<float>(volume, MIN_VOLUME_VALUE, MAX_VOLUME_VALUE);

				auto& sound = SoundManager::Get();
				switch (type)
				{
				case SoundType::Master: sound.SetMasterVolume(volume); break;
				case SoundType::Voice:  sound.SetVoiceVolume(volume);  break;
				case SoundType::SE:     sound.SetSEVolume(volume);     break;
				case SoundType::BGM:    sound.SetBGMVolume(volume);    break;
				default:
					K2_ASSERT(false, "無効なタイプです。");
					break;
				}
			}


			/**
			 * @brief 音量（0.0f 〜 1.0f）をノブの X 座標に変換する
			 * @param volume 音量
			 * @return ノブの X 座標
			 */
			float VolumeToKnobPosX(float volume)
			{
				return LEFT_LIMITE + POSITION_RANGE * volume;
			}
		}


		SoundOptionMenu::SoundOptionMenu()
			: m_currentSoundType(SoundType::Master)
			, m_isBack(false)
			, m_currentValue(0.0f)
		{
			// シーン遷移後も現在の音量設定が維持されるよう、SoundManagerの現在値で初期化する。
			m_currentValue = GetCurrentVolumeFromManager(m_currentSoundType);
		}


		void SoundOptionMenu::Update()
		{
			// 現在のタイプを保存。
			const uint8_t currentType = static_cast<uint8_t>(m_currentSoundType);

			uint8_t add = 0;

			// Xボタンに入力があったら、リセットする。
			if (g_pad[0]->IsTrigger(enButtonX))
			{
				Reset();
			}

			// 上方向に入力があったら
			if (g_pad[0]->IsTrigger(enButtonUp))   add = 3;

			// 下方向に入力があったら
			if (g_pad[0]->IsTrigger(enButtonDown))  add = 1;

			// addが0でなければ
			if (add != 0)
			{
				// タイプを変更する。
				m_currentSoundType = static_cast<SoundType>((currentType + add) % SOUND_SIZE);

				// 変更したタイプに対応するフレームのカラーアニメーションを更新する。
				UpdateColorAnim();
			}

			// 十字キーの左右どちらかの入力フラグ。
			const bool isTrigger = g_pad[0]->IsTrigger(enButtonLeft) || g_pad[0]->IsTrigger(enButtonRight);

			// 現在選択中のノブアイコンを取得する。
			const uint32_t knobKey = SOUND_ICON_KEYS[static_cast<uint8_t>(m_currentSoundType)].key;
			if (auto* knobIcon = GetUI<UIIcon>(knobKey))
			{
				if (isTrigger)
				{
					// 左右入力中はアニメーション停止して白色に戻す。
					knobIcon->StopAnimation();
					knobIcon->m_color = Vector4::White;
				}
				else if (add == 0)
				{
					// 上下入力も左右入力もないときはカラーアニメーションを再開する。
					UpdateColorAnim();
				}
			}

			// 現在選択中のノブの入力処理と音量反映。
			UpdateKnob();

			// 全てのDigitを更新する。
			UpdateDigits();

			// ここでキャンバスの更新が行われる
			SoundClass::Update();
		}


		void SoundOptionMenu::InitializeLogic()
		{
			InitializeIcon();
			InitializeFrame();
			InitializeDigit();
			InitializeIconAnim();
			UpdateColorAnim();
		}


		void SoundOptionMenu::UpdateKnob()
		{
			const SoundType type = m_currentSoundType;
			// ノブアイコンのキーを取得。
			const uint32_t key = SOUND_ICON_KEYS[static_cast<uint8_t>(type)].key;
			// フレームアイコンのキーを取得。
			const uint32_t secondKey = SOUND_FRAME_KEYS[static_cast<uint8_t>(type)].key;

			auto* knobIcon = GetUI<UIIcon>(key);
			auto* frameIcon = GetUI<UIIcon>(secondKey);

			// どちらかのアイコンが存在しない場合は処理しない。
			if (!knobIcon || !frameIcon) return;

			// 現在の音量を0から100の整数に変換する。
			float currentVolume = GetCurrentVolumeFromManager(type);
			int volumeInt = static_cast<int>(currentVolume * 100.0f);
			bool isChange = false;

			// 十字キー右を押したとき
			if (g_pad[0]->IsTrigger(enButtonRight))
			{
				volumeInt += VOLUME_STEP;
				isChange = true;
			}
			// 十字キー左を押したとき
			else if (g_pad[0]->IsTrigger(enButtonLeft))
			{
				volumeInt -= VOLUME_STEP;
				isChange = true;
			}

			// ノブのX座標とフレームのX座標への参照。
			float& knobPosX = knobIcon->m_transform.m_localTransform.m_position.x;
			float& framePosX = frameIcon->m_transform.m_localTransform.m_position.x;
			
			if (isChange)
			{
				volumeInt = util::clamp<int>(volumeInt, MIN_VALUE, MAX_VALUE);
				currentVolume = static_cast<float>(volumeInt) / VOLUME_VALUE_CONVERTER;

				// ノブの座標を更新する。
				knobPosX = VolumeToKnobPosX(currentVolume);
				// フレームの座標を更新する。
				framePosX = VolumeToKnobPosX(currentVolume);

				// SoundManagerに音量を反映する。
				SetVolumeToManager(type, currentVolume);
			}
		}


		void SoundOptionMenu::UpdateDigits()
		{
			for (const auto& info : SOUND_DIGIT_KEYS)
			{
				auto* digit = GetUI<UIDigit>(info.key);
				if (!digit) continue;

				// SoundManagerから取得した音量を0〜100の整数に変換して表示する。
				const float volume = GetCurrentVolumeFromManager(info.type);
				digit->SetNumber(static_cast<int>(volume * VOLUME_VALUE_CONVERTER));
			}
		}


		void SoundOptionMenu::UpdateColorAnim()
		{
			// 現在選択されているタイプに対応するフレームキーを取得する。
			const uint32_t selectedKey = SOUND_FRAME_KEYS[static_cast<uint8_t>(m_currentSoundType)].key;

			for (const auto& info : SOUND_FRAME_KEYS)
			{
				auto* icon = GetUI<UIIcon>(info.key);
				if (!icon) continue;

				if (info.key == selectedKey)
				{
					// 選択中のフレームはアニメーションを再生する。
					icon->PlayAnimation();
				}
				else
				{
					// 非選択のフレームは停止して白色に戻す。
					icon->StopAnimation();
					icon->m_color = Vector4::White;
				}
			}
		}


		void SoundOptionMenu::Reset()
		{
			for (int i = 0; i < SOUND_SIZE; ++i)
			{
				// ノブアイコンを取得する。
				auto* knobIcon = GetUI<UIIcon>(SOUND_ICON_KEYS[i].key);
				if (knobIcon)
				{
					// ノブアイコンの位置をリセットする。
					knobIcon->m_transform.m_localTransform.m_position.x = VolumeToKnobPosX(RESET_VOLUME_VALUE);
				}

				// フレームアイコンを取得する。
				auto* frameIcon = GetUI<UIIcon>(SOUND_FRAME_KEYS[i].key);
				if (frameIcon)
				{
					// フレームアイコンの位置をリセットする。
					frameIcon->m_transform.m_localTransform.m_position.x = VolumeToKnobPosX(RESET_VOLUME_VALUE);
				}

				// SoundManagerの音量をリセットする。
				SetVolumeToManager(SOUND_ICON_KEYS[i].type, RESET_VOLUME_VALUE);
			}

			// Digitの表示も即時反映する。
			UpdateDigits();
		}


		void SoundOptionMenu::InitializeIcon()
		{
			for (const auto& info : SOUND_ICON_KEYS)
			{
				auto* knobIcon = GetUI<UIIcon>(info.key);
				if (!knobIcon) continue;

				// シーン遷移後も音量設定が維持されるよう、SoundManagerの現在値でノブ位置を初期化する。
				const float volume = GetCurrentVolumeFromManager(info.type);
				knobIcon->m_transform.m_localTransform.m_position.x = VolumeToKnobPosX(volume);
			}
		}


		void SoundOptionMenu::InitializeFrame()
		{
			for (const auto& info : SOUND_FRAME_KEYS)
			{
				auto* frameIcon = GetUI<UIIcon>(info.key);
				if (!frameIcon) continue;

				// SoundManagerの現在値でフレーム位置を初期化する。
				const float volume = GetCurrentVolumeFromManager(info.type);
				frameIcon->m_transform.m_localTransform.m_position.x = VolumeToKnobPosX(volume);
			}
		}


		void SoundOptionMenu::InitializeIconAnim()
		{
			for (const auto& info : SOUND_FRAME_KEYS)
			{
				auto* icon = GetUI<UIIcon>(info.key);
				if (!icon) continue;

				// 古いアニメーションを削除する。
				icon->RemoveAnimation(Hash32("ColorAnim"));

				// カラーアニメーションを作成する。
				auto colorAnim = std::make_unique<UIColorAnimation>();

				// 開始色。
				const Vector4 startColor = Vector4::White;
				// 終了色。
				const Vector4 endColor = COLOR_ALPHA;
				// 間隔。
				float duration = COLOR_ANIM_DURATION;

				// アニメーションのパラメータを設定する。
				colorAnim->SetParameter(
					startColor
					, endColor
					, duration
					, util::EasingType::EaseInOut
					, util::LoopMode::PingPong
				);

				// アイコンにカラーアニメーションを登録する。
				icon->AddAnimation(Hash32("ColorAnim"), std::move(colorAnim));
			}
		}


		void SoundOptionMenu::InitializeDigit()
		{
			// 初期表示をSoundManagerの現在値で設定する。
			UpdateDigits();
		}
	}
}