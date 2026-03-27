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


		void ChildPenguin::SetChildPenguinType(EnChildPenguinType type)
		{
			m_type = type;
			m_colorApplied = false;

			// タイプ別乗算カラーを設定
			switch (type)
			{
			case EnChildPenguinType::Serious:
				m_typeColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);	// デフォルト（白）
				break;
			case EnChildPenguinType::Clingy:
				m_typeColor = Vector4(1.0f, 0.85f, 0.85f, 1.0f);	// ピンク系（甘えん坊）
				break;
			case EnChildPenguinType::naughty:
				m_typeColor = Vector4(1.0f, 0.90f, 0.70f, 1.0f);	// オレンジ系（やんちゃ）
				break;
			case EnChildPenguinType::Clumsy:
				m_typeColor = Vector4(0.85f, 0.90f, 1.0f, 1.0f);	// 青系（おっちょこちょい）
				break;
			case EnChildPenguinType::Caring:
				m_typeColor = Vector4(0.85f, 1.0f, 0.88f, 1.0f);	// 緑系（世話焼き）
				break;
			}

			// タイプ変更に伴いステートマシンを作成
			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			m_characterStateMachine = m_stateMachine.get();

			CreateAIController();
		}


		void ChildPenguin::CreateAIController()
		{
			// 親ペンギンが設定されたら、タイプに応じたAIコントローラーを作成
			switch (m_type)
			{
			case EnChildPenguinType::Serious:
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clingy:
				m_aiController = std::make_unique<ClingyChildPenguinAI>(this);
				break;
			default:
				// まだ実装されていないタイプはSeriousとして動作
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this);
				break;
			}
		}


		ChildPenguin::ChildPenguin()
		{
			Init(MODEL_DATA);

			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			m_characterStateMachine = m_stateMachine.get();

			m_status = std::make_unique<ChildPenguinStatus>();
			m_status->Setup();

			CreateAIController();
		}


		void ChildPenguin::Start()
		{
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			// モデルロード完了後、一度だけカラーを適用
			if (m_modelReady && !m_colorApplied)
			{
				m_modelRender.SetMulColor(m_typeColor);
				m_modelRender.Update();
				m_colorApplied = true;
			}

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