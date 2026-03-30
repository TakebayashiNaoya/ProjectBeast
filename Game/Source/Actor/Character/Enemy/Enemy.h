/**
 * @file Enemy.h
 * @brief エネミークラス
 * @author 立山
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


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


		public:
			Enemy();
			~Enemy() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc)override final;


		private:
			/** ステートマシン */
			std::unique_ptr<EnemyStateMachine>m_stateMachine;
			/** 自分の巣の座標 */
			Vector3 m_homePosition;
		};
	}
}
