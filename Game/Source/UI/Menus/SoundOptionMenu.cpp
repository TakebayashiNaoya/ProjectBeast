/**
 * @file SoundOptionMenu.cpp
 * @brief サウンドのオプションの動的処理群
 * @author 忽那
 */
#include "stdafx.h"
#include "SoundOptionMenu.h"
#include "Source/Sound/SoundManager.h"
#include "Source/Util/Curve.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// keyとenum classとフラグの構造体
			struct SoundInfo
			{
				uint32_t key;
				SoundType type;
			};


			/** サウンドオプションの数 */
			constexpr int SOUND_SIZE = 4;
			/** サウンドアイコンの配列 */
			constexpr std::array<SoundInfo,SOUND_SIZE>SOUND_ICON_KEYS =
			{
				{
					{ Hash32("SoundKnobMasterIcon"),SoundType::Master }
				,	{ Hash32("SoundKnobVoiceIcon"),SoundType::Voice }
				,	{ Hash32("SoundKnobSEIcon"),SoundType::SE }
				,	{ Hash32("SoundKnobBGMIcon"),SoundType::BGM }
				}
			};


			/** サウンドディジットの配列 */
			constexpr std::array<SoundInfo,SOUND_SIZE> SOUND_DIGIT_KEYS =
			{
				{
					{ Hash32("SoundMasterDigit"), SoundType::Master }
				,	{ Hash32("SoundVoiceDigit"), SoundType::Voice }
				,	{ Hash32("SoundSEDigit"), SoundType::SE }
				,	{ Hash32("SoundBGMDigit"), SoundType::BGM }
				}
			};



			/** 音量の初期値 */
			constexpr float INITIALIZE_VOLUME_VALUE = 0.5f;
			/** 全体の音量の初期値 */
			constexpr float RESET_VOLUME_VALUE = 0.1f;
			/** 音量の最小値 */
			constexpr float MIN_VOLUME_VALUE = 0.0f;
			/** 音量の最大値 */
			constexpr float MAX_VOLUME_VALUE = 1.0f;
			/** 1フレームで変化する音量の値 */
			constexpr float DELTA_VOLUME_VALUE = 0.002f;
			//0から1の値を0から100の値に変換するための定数
			constexpr float VOLUME_VALUE_CONVERTER = 100.0f;


			/** 1フレームで変化するポジションの値 */
			constexpr float DELTA_POSITION_VALUE = 2.0f;
			/** ポジションの右側の制限値 */
			constexpr float RIGHT_LIMITE_POSITION_VALUE = 580.0f;
			/** ポジションの左側の制限値 */
			constexpr float LEFT_LIMITE_POSITION_VALUE = -420.0f;

			
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
					return INITIALIZE_VOLUME_VALUE;
				}
			}


			/**
			 * @brief 音量（0.0f 〜 1.0f）をノブの X 座標に変換する
			 * @param volume 音量
			 * @return ノブの X 座標
			 */
			float VolumeToKnobPosX(float volume)
			{
				/** スライダーの全幅に対する音量の比率でノブ位置を逆算する */
				const float range = RIGHT_LIMITE_POSITION_VALUE - LEFT_LIMITE_POSITION_VALUE;
				return LEFT_LIMITE_POSITION_VALUE + range * volume;
			}
		}


		SoundIcon::SoundIcon(SoundType type)
			: m_deltaValue(DELTA_VOLUME_VALUE)
			, m_icon(nullptr)
			, m_type(type)
			, m_gamePad(g_pad[0])
		{
			/** シーン遷移後も音量設定が維持されるよう、SoundManager の現在値で初期化する */
			m_currentValue = GetCurrentVolumeFromManager(type);
		}


		SoundIcon::~SoundIcon()
		{}


		void SoundIcon::Update()
		{
			CalcRatio();
		}


		void SoundIcon::CalcRatio()
		{
			// ポジションの変化の値を保存する変数。
			float deltaPositionValue = 0.0f;
			// 音量の変化の値を保存する変数。
			float deltaVolumeValue = 0.0f;

			// 変化したかどうかのフラグ。
			bool isChange = false;

			// 十時キー右を押したときに移動又はフラグを真にする。
			if (m_gamePad->IsPress(enButtonRight))
			{
				deltaVolumeValue = m_deltaValue;
				deltaPositionValue = DELTA_POSITION_VALUE;
				isChange = true;
			}
			// 十時キー左を押したときに移動又はフラグを偽にする。
			else if (m_gamePad->IsPress(enButtonLeft))
			{
				deltaVolumeValue = -m_deltaValue;
				deltaPositionValue = -DELTA_POSITION_VALUE;
				isChange = true;
			}

			float& posX = m_icon->m_transform.m_localTransform.m_position.x;

			// xの値を変化させる。
			if (isChange)
			{
				posX += deltaPositionValue;
				m_currentValue += deltaVolumeValue;
			}

			// 左の制限値と右の制限値の間に収める。
			posX = util::clamp<float>(posX, LEFT_LIMITE_POSITION_VALUE, RIGHT_LIMITE_POSITION_VALUE);
			// 最小値と最大値の間に収める。
			m_currentValue = util::clamp<float>(m_currentValue, MIN_VOLUME_VALUE, MAX_VOLUME_VALUE);


			// 現在の値を別の変数に保存。
			const float result = m_currentValue;
			// SoundManagerのインスタンスを取得。
			auto& sound = SoundManager::Get();

			// タイプごとに値を設定。
			switch (m_type)
			{
			case SoundType::Master:
				sound.SetMasterVolume(result);
				break;
			case SoundType::Voice:
				sound.SetVoiceVolume(result);
				break;
			case SoundType::SE:
				sound.SetSEVolume(result);
				break;
			case SoundType::BGM:
				sound.SetBGMVolume(result);
				break;
			default:
				// 間違いならはじく。
				K2_ASSERT(false, "無効なタイプです。");
				break;
			}
		}


		void SoundIcon::SetUIIcon(UIIcon* icon)
		{
			m_icon = icon;
			K2_ASSERT(m_icon != nullptr, "登録失敗！");

			/** UIが設定されたタイミングで、現在の音量に合わせてノブ位置を初期化する */
			m_icon->m_transform.m_localTransform.m_position.x = VolumeToKnobPosX(m_currentValue);
		}


		void SoundIcon::SoundResetToPos()
		{
			// iconがないなら弾く。
			if (!m_icon) return;

			// 直接値を書き込んで初期値に戻す。
			m_currentValue = RESET_VOLUME_VALUE;
			// アイコンの位置を初期位置に戻す。
			m_icon->m_transform.m_localTransform.m_position.x = VolumeToKnobPosX(m_currentValue);

			// SoundManagerのインスタンスを取得。
			auto& sound = SoundManager::Get();

			// 各タイプごとに値を設定して、初期化する。
			switch (m_type)
			{
			case SoundType::Master: sound.SetMasterVolume(m_currentValue); break;
			case SoundType::Voice:  sound.SetVoiceVolume(m_currentValue); break;
			case SoundType::SE:     sound.SetSEVolume(m_currentValue); break;
			case SoundType::BGM:    sound.SetBGMVolume(m_currentValue); break;
			}
		}





		/********************************************************/


		SoundDigit::SoundDigit(SoundType type)
			: m_type(type)
			, m_currentValue(0.0f)
			, m_digit(nullptr)
			, m_soundIcon(nullptr)
		{}


		SoundDigit::~SoundDigit()
		{}


		void SoundDigit::Update()
		{
			Calc();
		}


		void SoundDigit::Calc()
		{
			const auto& soundGet = SoundManager::Get();
			// 現在の値を所得。
			float currentValue = m_currentValue;
			switch (m_type)
			{
				// 各タイプごとに値を取得。
			case SoundType::Master:
				currentValue = soundGet.GetMasterVolume();
				break;
			case SoundType::Voice:
				currentValue = soundGet.GetVoiceVolume();
				break;
			case SoundType::SE:
				currentValue = soundGet.GetSEVolume();
				break;
			case SoundType::BGM:
				currentValue = soundGet.GetBGMVolume();
				break;
			default:
				// 間違いならはじく。
				K2_ASSERT(false, "無効なタイプです。");
				break;
			}

			// SoundManagerから取得した値を100倍する。
			currentValue *= VOLUME_VALUE_CONVERTER;
			m_digit->SetNumber(static_cast<int>(currentValue));
		}


		void SoundDigit::SetUIDigit(UIDigit* digit)
		{
			m_digit = digit;
			K2_ASSERT(m_digit != nullptr, "登録失敗です。");
		}


		void SoundDigit::SetSoundIcon(SoundIcon* soundIcon)
		{
			m_soundIcon = soundIcon;
			K2_ASSERT(m_soundIcon != nullptr, "登録失敗です。");
		}


		void SoundDigit::ResetToDigit()
		{
			// digitがnullptrの場合警告。
			K2_ASSERT(m_digit != nullptr, "リセット失敗！");

			// 選んでいるタイプに対応するSoundManagerの値を取得して、digitに値を設定する。
			m_digit->SetNumber(GetCurrentVolumeFromManager(m_type));
		}





		/********************************************************/


		SoundOptionMenu::SoundOptionMenu()
			: m_currentSoundType(SoundType::Master)
			, m_isBack(false)
		{}


		void SoundOptionMenu::Update()
		{
			// 戻るフラグがfalseなら、trueにする。
			if (m_isBack) m_isBack = !m_isBack;

			// 現在のタイプを保存。
			const uint8_t currentType = static_cast<uint8_t>(m_currentSoundType);

			// uint8_t型の変数を宣言。
			uint8_t add = 0;

			// Xボタンに入力があったら、リセットする関数を呼び出す。
			if (g_pad[0]->IsTrigger(enButtonX))
			{
				// ここでリセットする関数を呼び出す。
				Reset();
			}

			// 上方向に入力があったら
			if (g_pad[0]->IsTrigger(enButtonUp)) add = 3;

			// 下方向に入力があったら
			if (g_pad[0]->IsTrigger(enButtonDown)) add = 1;

			// addが0でなければ
			if (add != 0)
			{
				// タイプを変更する。
				m_currentSoundType = static_cast<SoundType>((currentType + add) % SOUND_SIZE);
			}


			const uint8_t type = static_cast<uint8_t>(m_currentSoundType);
			const Key key = SOUND_ICON_KEYS[type].key;

			// アイコンマップの更新。
			m_soundIconMap[key].get()->Update();

			// 全てのdigitを更新。
			for (int i = 0; i < SOUND_SIZE; i++)
			{
				// 各タイプのキーを取得して、それぞれのdigitを更新する。
				const Key eachDigitKey = SOUND_DIGIT_KEYS[i].key;
				m_soundDigitMap[eachDigitKey].get()->Update();
			}

			// ここでキャンバスの更新が行われる
			SoundClass::Update();
		}


		void SoundOptionMenu::InitializeLogic()
		{
			InitializeIcon();

			InitializeDigit();
		}


		void SoundOptionMenu::Reset()
		{
			// アイコンとディジットの両方を初期化する。
			for (int i = 0; i < SOUND_SIZE; i++)
			{
				// Iconの配列のキーを取得する。
				const Key key = SOUND_ICON_KEYS[i].key;
				
				auto it = m_soundIconMap.find(key);
				
				// アイコンのマップにキーが存在していて、iconがnullptrでないなら
				if (it != m_soundIconMap.end() && it->second)
				{
					// it->second.get()でiconを取得して、初期化する関数を呼び出す。
					it->second.get()->SoundResetToPos();
				}
				// Digitの配列のキーを取得する。
				const Key digitKey = SOUND_DIGIT_KEYS[i].key;
				// digitの配列キーを取得する。
				auto itD = m_soundDigitMap.find(digitKey);

				// digitのマップにキーが存在していて、digitがnullptrでないなら
				if (itD != m_soundDigitMap.end() && itD->second)
				{
					// it->second.get()でdigitを取得して、初期化する関数を呼び出す。
					itD->second.get()->ResetToDigit();
				}
			}
		}


		void SoundOptionMenu::InitializeIcon()
		{
			// ホットリロード時のダングリングポインタを防ぐためクリア
			m_soundIconMap.clear();

			// サウンドマップの最大サイズを設定
			m_soundIconMap.reserve(SOUND_SIZE);

			for (const auto& info : SOUND_ICON_KEYS)
			{
				// 新しいバーを作成
				Icon newIcon = std::make_unique<SoundIcon>(info.type);
				// UIパーツの設定（SetUIIcon内でノブ位置も現在値から初期化される）
				newIcon->SetUIIcon(GetUI<UIIcon>(info.key));
				// マップにアイコンを追加
				m_soundIconMap.emplace(info.key, std::move(newIcon));
			}
		}


		void SoundOptionMenu::InitializeDigit()
		{
			// マップの情報を初期化
			m_soundDigitMap.clear();
			// マップの最大サイズ数を設定
			m_soundDigitMap.reserve(SOUND_SIZE);

			for (const auto& info : SOUND_DIGIT_KEYS)
			{
				// 新しいディジットを作成
				Digit newDigit = std::make_unique<SoundDigit>(info.type);
				// UIパーツの設定
				newDigit->SetUIDigit(GetUI<UIDigit>(info.key));
				int iconType = static_cast<int>(info.type);
				const Key iconKey = SOUND_ICON_KEYS[iconType].key;
				newDigit->SetSoundIcon(m_soundIconMap[iconKey].get());

				// マップにディジットを追加
				m_soundDigitMap.emplace(info.key, std::move(newDigit));

			}
		}
	}
}