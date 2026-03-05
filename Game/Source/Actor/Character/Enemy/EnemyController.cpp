/**
 * @file EnemyController.cpp
 * @brief エネミーのコントローラー
 * @author 立山
 */
#include "stdafx.h"
#include "EnemyController.h"

#include "Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"


namespace app
{
	namespace actor
	{
		EnemyController::EnemyController()
		{
		}


		EnemyController::~EnemyController()
		{
		}


		bool EnemyController::Start()
		{
			return true;
		}


		void EnemyController::Update()
		{
			auto* targetStateMachine = m_target->GetEnemyStateMachine();

			if (IsInputStickL())
			{
				// 左スティックの方向

				//左スティックの入力量を取得

			}
			// スティックの入力量を設定する
			// 0～1の範囲で入力量を取得

		}


		void EnemyController::Render(RenderContext& renderContext)
		{
		}


		Vector3 EnemyController::GetStickL()
		{
			// 左スティックの入力量を取得
			Vector3 stickL;
			stickL.x = GetPad()->GetLStickXF();
			stickL.y = GetPad()->GetLStickYF();

			// カメラの前方向と右方向のベクトルを取得
			Vector3 forward = g_camera3D->GetForward();
			Vector3 right = g_camera3D->GetRight();

			//y方向には移動しない
			forward.y = 0.0f;
			right.y = 0.0f;

			//左スティックの入力量を加算
			right *= stickL.x;
			forward *= stickL.y;

			Vector3 direction = right + forward;
			//正規化
			direction.Normalize();

			return direction;
		}


		Quaternion EnemyController::ComputeRotation()
		{
			//スティックの方向
			Vector3 direction = GetStickL();
			// スティック入力を使ってY軸回転の情報を得る
			Quaternion qRot;
			qRot.SetRotationYFromDirectionXZ(direction);

			return qRot;
		}
	}
}