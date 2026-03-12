/**
 * @file SoundOptionMenu.h
 * @brief サウンドのオプションの動的処理群
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"
#include <unordered_map>
#include <memory>


namespace app
{
	namespace ui
	{
		enum class SoundType : uint8_t
		{
			Master,
			Voice,
			SE,
			BGM
		};


		class SoundIcon
		{
		public:
			SoundIcon(SoundType type);
			~SoundIcon();
			void Update();
			void CalcRatio();
			void SetUIIcon(UIIcon* icon);


		private:
			float m_currentValue;
			float m_deltaValue;
			SoundType m_type;
			UIIcon* m_icon;
			GamePad* m_gamePad;
		};


		class SoundDigit
		{
		public:
			SoundDigit(SoundType tpye);
			~SoundDigit();
			void Update();
			void Calc();
			void SetUIDigit(UIDigit* digit);
			void SetSoundIcon(SoundIcon* soundIcon);
			

		private:
			SoundType m_type;
			UIDigit* m_digit;
			SoundIcon* m_soundIcon;
		};


		class SoundOptionMenu : public MenuBase
		{
			using SoundClass = MenuBase;


		public:
			SoundOptionMenu();

			void Update()override;
			void InitializeLogic()override;


			/**
			 * @brief サウンドメニューで扱うアイコンの情報を初期化する
			 */
			void InitializeIcon();
			
			/**
			 * @brief サウンドメニューで扱う数値の情報を初期化する
			 */
			void InitializeDigit();

			
		private:
			SoundType m_currentSoundType;


			using Icon = std::unique_ptr<SoundIcon>;
			using Digit = std::unique_ptr<SoundDigit>;
			using Key = uint32_t;
			std::unordered_map<Key, Icon>m_soundIconMap;
			std::unordered_map<Key, Digit> m_soundDigitMap;
		};
	}
}
