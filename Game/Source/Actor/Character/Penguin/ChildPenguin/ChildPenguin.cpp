/**
 * @file ChildPenguin.cpp
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"


namespace app
{
	namespace actor
	{

		namespace
		{
			const ModelData MODEL_DATA =
			{
				"Assets/modelData/penguin/childPenguin/ChildPenguin.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};
		}


		ChildPenguin::ChildPenguin()
		{
			Init(MODEL_DATA);

			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			m_characterStateMachine = m_stateMachine.get();

			m_status = std::make_unique<ChildPenguinStatus>();
			m_status->Setup();

		}


		void ChildPenguin::SetDaddyPenguin(DaddyPenguin* daddyPenguin)
		{
			m_daddyPenguin = daddyPenguin;

			// 親ペンギンが設定されたら、タイプに応じたAIコントローラーを作成
			switch (m_type)
			{
			case EnChildPenguinType::Serious:
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this, daddyPenguin);
				break;
			case EnChildPenguinType::Clingy:
				m_aiController = std::make_unique<ClingyChildPenguinAI>(this, daddyPenguin);
				break;
			default:
				// まだ実装されていないタイプはSeriousとして動作
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this, daddyPenguin);
				break;
			}
		}


		void ChildPenguin::SetChildPenguinType(EnChildPenguinType type)
		{
			m_type = type;

			// タイプ変更に伴いステートマシンを作成
			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			m_characterStateMachine = m_stateMachine.get();

			// 既に親ペンギンが設定されている場合はAIコントローラーを再作成
			if (m_daddyPenguin != nullptr)
			{
				SetDaddyPenguin(m_daddyPenguin);
			}
		}


		void ChildPenguin::Start()
		{
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			// AIコントローラーがあれば更新
			if (m_aiController)
			{
				m_aiController->Update();
			}

			// ステートマシン更新
			m_stateMachine->Update();

			PenguinBase::Update();
		}


		void ChildPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}
	}
}