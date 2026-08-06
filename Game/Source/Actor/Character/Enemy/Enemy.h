/**
 * @file Enemy.h
 * @brief エネミークラス
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"
#include "Source/Actor/IK/LegIKComponent.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class EnemyStateMachine;


		/**
		 * @brief エネミークラス
		 */
		class Enemy :public CharacterBase
		{
		public:
			/**
			 * @brief 巣の座標を取得
			 * @return 巣の座標
			 */
			inline Vector3 GetHomePosition() const { return m_homePosition; }
			/**
			 * @brierf 巣の座標を設定
			 * @param pos 巣の座標
			 */
			inline void SetHomePosition(const Vector3& pos) { m_homePosition = pos; }

			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			EnemyStateMachine* GetEnemyStateMachine() { return m_stateMachine.get(); }

			inline int  GetLogId() const { return m_logId; }
			inline void SetLogId(int id) { m_logId = id; }


		public:
			Enemy();
			~Enemy() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc)override final;

			/** 脚IKのセットアップ（ボーン名はここで指定する。モデルロード完了後に1回だけ呼ぶ） */
			bool InitLegIK();

			/**
			 * @brief 地形の傾きに合わせて胴体を傾ける描画補正を行う
			 * @details PenguinBase::UpdateSlideTilt()と同じ考え方。
			 *          物理・ステート判定用のm_transformには触れず、
			 *          描画（＝スケルトンのワールド行列）だけを傾ける。
			 *          脚IKより先に呼ぶことで、IKは残差の微調整だけを担当する。
			 */
			void UpdateGroundTilt();

			// シロクマ用の足跡パラメータ
			float GetFootprintSize() const override { return 30.0f; }         // 体格に合わせて調整
			int   GetFootprintPriority() const override { return 2; }         // 通常のペンギンより消されにくくする
			float GetFootprintStanceWidth() const override { return 20.0f; }  // 左右の足跡の間隔を広げる
			float GetFootprintStepDistance() const override { return 40.0f; } // 歩幅（前後の間隔）も広めに
			bool  ShouldSuppressFootprint() const override;

			// 肉球専用テクスチャを常に使う（地形による自動切り替えはしない）
			app::effect::DecalKind GetFootprintKind() const override { return app::effect::DecalKind::BearFootprint; }
			bool GetFootprintAutoDetectSurface() const override { return false; }
			Vector4 GetFootprintColor() const override { return { 0.55f, 0.55f, 0.60f, 1.0f }; } // 雪面に残る影っぽいグレー

		private:
			/** ステートマシン */
			std::unique_ptr<EnemyStateMachine>m_stateMachine;
			/** 自分の巣の座標 */
			Vector3 m_homePosition;
			/** ログ用の連番ID（EnemyManager が生成順に割り当てる） */
			int m_logId = -1;

			/** 脚IK(4本分) */
			app::actor::ik::LegIKComponent m_legIK;
			/** InitLegIK()を呼び終えたかのフラグ */
			bool m_legIKInited = false;

			/** 地形傾斜に合わせた胴体描画用回転（補間済み） */
			Quaternion m_groundTiltRotation = Quaternion::Identity;
		};
	}
}
