/**
 * @file CharacterBase.cpp
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "CharacterBase.h"
#include "CharacterStateMachine.h"
#include "CharacterStatus.h"
#include "Source/Nature/Ocean.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			constexpr float GRAVITY = -9.8f * 150; // 重力の値
		}


		void CharacterBase::UpdateModelOnly()
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
				m_modelReady = true;

				m_characterStateMachine->ReEnterCurrentState();
				return;
			}

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
			// 非同期ロード完了待ち
			if (!m_modelReady)
			{
				if (!m_assetsLoader.IsReady())
				{
					return; // まだロード中
				}

				// ロード完了 → Finalize して ModelRender 初期化
				nsK2EngineLow::ModelInitData initData;
				m_assetsLoader.Finalize(initData, &m_skeleton, m_animationClips.get());
				m_modelRender.Init(initData.m_tkmFilePath, m_animationClips.get(), m_clipNum, true, m_upAxis);
				m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();
				m_modelReady = true;

				// モデルロード完了後に現在のステートのアニメーションを再適用する
				m_characterStateMachine->ReEnterCurrentState();
			}
			m_transform.m_position = m_characterStateMachine->GetTransform().m_position;
			m_transform.m_rotation = m_characterStateMachine->GetTransform().m_rotation;
			m_transform.m_scale = m_characterStateMachine->GetTransform().m_scale;

			//Vector3 renderPos = m_transform.m_position;

			//// 海クラスを取得
			//auto* ocean = app::nature::Ocean::GetInstance();

			//// デフォルトの目標は「物理エンジンのY座標（実際の地面の高さ）」にする
			//float targetY = renderPos.y;

			//// 「波追従モード」がONの時だけ、波の高さを目標にする
			//if (m_isWaveFollowMode && ocean != nullptr)
			//{
			//	targetY = ocean->SampleWaveHeight(renderPos.x, renderPos.z) + SWIM_Y_OFFSET;
			//}

			//// 初回実行時、または座標が大きくワープした時は、一気に現在地に合わせる
			//if (m_renderPosY == -9999.0f || std::abs(targetY - m_renderPosY) > 10.0f)
			//{
			//	m_renderPosY = targetY;
			//}

			//// 目標の高さ(targetY)に向かって、毎フレーム 0.2f (20%) ずつ滑らかに近づく
			//m_renderPosY += (targetY - m_renderPosY) * 0.2f;

			//// 計算された滑らかなY座標を描画用ポジションに適用
			//renderPos.y = m_renderPosY;

			// モデルの行列を更新
			m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
			m_modelRender.Update();
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
			if (pos != std::string::npos) {
				tksPath.replace(pos, 4, ".tks");
			}

			// 非同期リクエスト
			m_assetsLoader.Request(ResourceManager::GetInstance(), data.fileName, tksPath.c_str(), tkaPaths.data(), data.clipNum);
		}
	}
}