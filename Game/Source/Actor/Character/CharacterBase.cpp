/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CharacterBase.h"
#include "CharacterStateMachine.h"
#include "CharacterStatus.h"
#include "Source/Effect/DecalManager.h"
#include "Source/Nature/Ocean.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			constexpr float GRAVITY = -9.8f * 150; // 重力の値
			constexpr float FOOTPRINT_LIFE_SECONDS = 3.5f;     // 足跡の残存時間
			constexpr float FOOTPRINT_FADE_OUT_SECONDS = 1.0f; // 足跡のフェードアウト時間
		}


		void CharacterBase::UpdateModelOnly()
		{
			// モデルの非同期ロードの更新
			ModelLoadUpdate();

			// ロード完了済み → 行列のみ更新（AIやステートマシンは動かさない）
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			m_modelRender.Update();
		}


		CharacterBase::CharacterBase()
			: m_animationClips(nullptr)
			, m_clipNum(0)
			, m_upAxis(EnModelUpAxis::enModelUpAxisY)
			, m_modelReady(false)
			, m_characterStateMachine(nullptr)
		{}


		CharacterBase::~CharacterBase()
		{
			OcclusionDitherManager::Get().Unregister(&m_modelRender);

			// PhysicsWorldからRigidBodyを明示的に除去してから破棄する
			m_characterController.RemoveRigidBoby();
		}


		void CharacterBase::Start()
		{
			// ステータスを取得
			const auto* status = GetStatus<CharacterStatus>();
			// キャラクターコントローラーを初期化
			m_characterController.Init(status->GetRadius(), status->GetHeight(), m_transform.m_position);
			// 重力を設定
			m_characterController.SetGravity(GRAVITY);
			// ステートマシンの座標を Actor の座標と同期する
			m_characterStateMachine->SetPosition(m_transform.m_position);
		}


		void CharacterBase::Update()
		{
			// モデルの非同期ロードの更新
			ModelLoadUpdate();

			m_transform.m_position = m_characterStateMachine->GetTransform().m_position;
			m_transform.m_rotation = m_characterStateMachine->GetTransform().m_rotation;
			m_transform.m_scale = m_characterStateMachine->GetTransform().m_scale;

			// モデルの行列を更新
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			m_modelRender.Update();

			// 足跡の更新（Penguin/Enemy共通。違いは仮想関数側で吸収する）
			UpdateFootprints();
		}


		void CharacterBase::Render(RenderContext& rc)
		{
			// モデルレンダーを描画（準備完了時）
			if (m_modelReady)
			{
				m_modelRender.Draw(rc);
			}
		}


		void CharacterBase::Init(const ModelData& data)
		{
			// クリップ数とUpAxisを保存
			m_clipNum = data.clipNum;
			m_upAxis = data.upAxis;

			// アニメーションクリップ配列を確保
			m_animationClips = std::make_unique<AnimationClip[]>(data.clipNum);

			// tkaパス配列を準備
			std::vector<const char*> tkaPaths;
			tkaPaths.reserve(data.clipNum);
			for (int i = 0; i < data.clipNum; ++i)
			{
				m_animationClips[i].SetLoopFlag(data.animationData[i].isLoop); // ループフラグだけ事前設定
				tkaPaths.push_back(data.animationData[i].fileName);
			}

			// tksパス生成
			std::string tksPath = data.fileName;
			size_t pos = tksPath.find(".tkm");
			if (pos != std::string::npos)
			{
				tksPath.replace(pos, 4, ".tks");
			}

			// 非同期リクエスト
			m_assetsLoader.Request(ResourceManager::GetInstance(), data.fileName, tksPath.c_str(), tkaPaths.data(), data.clipNum);
		}


		void CharacterBase::ModelLoadUpdate()
		{
			// ロード完了待ち（Update()と同じ処理）
			if (!m_modelReady)
			{
				if (!m_assetsLoader.IsReady()) return;

				nsK2EngineLow::ModelInitData initData;
				m_assetsLoader.Finalize(initData, &m_skeleton, m_animationClips.get());
				m_modelRender.Init(initData.m_tkmFilePath, m_animationClips.get(), m_clipNum, true, m_upAxis);
				m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();

				// ディザリングマネージャーに登録
				OcclusionDitherManager::Get().Register(&m_modelRender);

				m_modelReady = true;

				m_characterStateMachine->ReEnterCurrentState();
				return;
			}
		}


		void CharacterBase::UpdateFootprints()
		{
			if (ShouldSuppressFootprint())
			{
				m_lastFootprintPos = m_transform.m_position;
				return;
			}

			Vector3 currentPos = m_transform.m_position;

			Vector3 diff;
			diff.Subtract(currentPos, m_lastFootprintPos);
			diff.y = 0.0f;

			const float stepDistance = GetFootprintStepDistance();
			if (diff.LengthSq() > stepDistance * stepDistance)
			{
				Vector3 forward = Vector3::AxisZ;
				m_transform.m_rotation.Apply(forward);
				float yaw = atan2f(forward.x, forward.z);

				Vector3 right = Vector3::AxisX;
				m_transform.m_rotation.Apply(right);

				float offsetAmount = GetFootprintStanceWidth();
				if (!m_isRightFoot) offsetAmount *= -1.0f;
				right.Scale(offsetAmount);

				Vector3 spawnPos;
				spawnPos.Add(currentPos, right);

				app::effect::DecalManager::Get().SpawnFootprint(
					spawnPos, yaw, GetFootprintKind(),
					GetFootprintSize(), FOOTPRINT_LIFE_SECONDS, FOOTPRINT_FADE_OUT_SECONDS, GetFootprintColor(),
					GetFootprintAutoDetectSurface(),
					GetFootprintPriority()
				);

				m_lastFootprintPos = currentPos;
				m_isRightFoot = !m_isRightFoot;
			}
		}
	}
}