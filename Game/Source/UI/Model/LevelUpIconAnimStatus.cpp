/**
 * @file LevelUpIconAnimStatus.cpp
 * @brief 陣形レベルアップアイコン専用のアニメーションステータスクラス
 * @author 竹林
 */
#include "stdafx.h"
#include "LevelUpIconAnimStatus.h"

#include "Source/UI/Animation/UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			const char* JSON_PATH = "Assets/parameter/UI/levelUp/LevelUpIconAnimParameter.json";
		}


		LevelUpIconAnimStatus::LevelUpIconAnimStatus()
		{
			UIAnimationParameter::Get().Load(JSON_PATH);
		}


		LevelUpIconAnimStatus::~LevelUpIconAnimStatus()
		{}


		void LevelUpIconAnimStatus::SetUp()
		{}


		void LevelUpIconAnimStatus::Update()
		{}
	}
}
