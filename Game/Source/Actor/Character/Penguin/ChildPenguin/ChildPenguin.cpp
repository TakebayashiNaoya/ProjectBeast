/**
 * @file ChildPenguin.cpp
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Core/ParameterManager.h"


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


		float ChildPenguin::GetJoinDistance() const
		{
			if (m_aiController)
			{
				return m_aiController->GetJoinDistance();
			}
			return 0.0f;
		}

		void ChildPenguin::SetChildPenguinType(EnChildPenguinType type)
		{
			m_type = type;
			m_colorApplied = false;

			/** タイプ別乗算カラーをJSONパラメーターから設定 */
			const int typeIndex = static_cast<int>(type);
			const auto* param = core::ParameterManager::Get()->GetParameter<MasterChildPenguinParameter>(typeIndex);
			const auto& td = param->typeData[typeIndex];
			m_typeColor = Vector4(td.colorR, td.colorG, td.colorB, td.colorA);

			/** タイプ変更に伴いステートマシンを作成 */
			/** おっちょこちょいタイプは固有ステートを持つため専用クラスを使う */
			if (m_type == EnChildPenguinType::Clumsy)
			{
				m_stateMachine = std::make_unique<ClumsyChildPenguinStateMachine>(this);
			}
			else
			{
				m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			}
			m_characterStateMachine = m_stateMachine.get();

			CreateAIController();
		}


		void ChildPenguin::CreateAIController()
		{
			/** 親ペンギンが設定されたら、タイプに応じたAIコントローラーを作成 */
			switch (m_type)
			{
			case EnChildPenguinType::Serious:
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clingy:
				m_aiController = std::make_unique<ClingyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Naughty:
				m_aiController = std::make_unique<NaughtyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clumsy:
				m_aiController = std::make_unique<ClumsyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Caring:
				m_aiController = std::make_unique<CaringChildPenguinAI>(this);
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
			/** スケールを初期化 */
			m_transform.m_scale = CHILD_PENGUIN_SCALE;
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			/** モデルロード完了後、一度だけカラーを適用 */
			if (m_modelReady && !m_colorApplied)
			{
				m_modelRender.SetMulColor(m_typeColor);
				m_modelRender.Update();
				m_colorApplied = true;
			}

			/** AIコントローラーがあれば更新 */
			if (m_aiController)
			{
				m_aiController->Update();
			}

			/** ステートマシン更新 */
			m_stateMachine->Update();

			PenguinBase::Update();

			/** 泳ぎ中はモデルの描画位置のみY座標をオフセットする */
			/** 物理・ステート判定には影響を与えない */
			if (m_stateMachine->IsSwimming() && m_modelReady)
			{
				Vector3 renderPos = m_transform.m_position;
				renderPos.y += SWIM_Y_OFFSET;
				m_modelRender.SetTRS(renderPos, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();
			}
		}


		void ChildPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}
	}
}