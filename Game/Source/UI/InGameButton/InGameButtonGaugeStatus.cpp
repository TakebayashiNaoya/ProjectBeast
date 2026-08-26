/**
 * @file InGameButtonGaugeStatus.cpp
 * @brief インゲームボタンのスタミナゲージ専用のステータスクラス
 */
#include "stdafx.h"
#include "InGameButtonGaugeStatus.h"
#include "MasterInGameButtonGaugeParameter.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace ui
	{
		namespace
		{
			// JSONファイルのパス。
			const char* JSON_PATH = "Assets/parameter/UI/inGameButton/InGameButtonGaugeParameter.json";

			// JSON側の値が0以下（未読み込み・読み込み失敗など）の場合に使う、安全なフォールバック追従速度。
			constexpr float DEFAULT_FOLLOW_SPEED = 2.0f;
		}


		InGameButtonGaugeStatus::InGameButtonGaugeStatus()
			: m_jumpFollowSpeed(0.0f)
			, m_slideFollowSpeed(0.0f)
		{
			// JSONファイルからパラメーターを読み込む。
			core::ParameterManager::Get()->LoadParameter<MasterInGameButtonGaugeParameter>(JSON_PATH, [](const nlohmann::json& j, MasterInGameButtonGaugeParameter& parameter)
				{
					parameter.jumpFollowSpeed = util::JsonConverter::ToFloat(j, "jumpFollowSpeed");
					parameter.slideFollowSpeed = util::JsonConverter::ToFloat(j, "slideFollowSpeed");
				});
		}


		InGameButtonGaugeStatus::~InGameButtonGaugeStatus()
		{
			core::ParameterManager::Get()->UnloadParameter<MasterInGameButtonGaugeParameter>();
		}


		void InGameButtonGaugeStatus::SetUp()
		{
			const auto* parameter = core::ParameterManager::Get()->GetParameter<MasterInGameButtonGaugeParameter>();
			if (!parameter) return;

			// JSON側の値が0以下（未読み込み・読み込み失敗など）の場合は、追従が完全に止まってしまうのを防ぐため
			// 安全な最低値にフォールバックする。
			m_jumpFollowSpeed = (parameter->jumpFollowSpeed > 0.0f) ? parameter->jumpFollowSpeed : DEFAULT_FOLLOW_SPEED;
			m_slideFollowSpeed = (parameter->slideFollowSpeed > 0.0f) ? parameter->slideFollowSpeed : DEFAULT_FOLLOW_SPEED;
		}


		void InGameButtonGaugeStatus::Update()
		{
			SetUp();
		}
	}
}
