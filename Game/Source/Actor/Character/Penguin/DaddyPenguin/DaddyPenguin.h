/**
 * @file DaddyPenguin.h
 * @brief 親ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class DaddyPenguinStateMachine;
		class DaddyPenguinController;


		/**
		 * @brief 親ペンギンクラス
		 */
		class DaddyPenguin : public PenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline DaddyPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }
			/**
			 * @brief Activeフラグの設定
			 * @param isActive Activeフラグ
			 */
			inline void SetActive(const bool isActive)
			{
				m_isActive = isActive;
			}


		public:
			DaddyPenguin();
			virtual ~DaddyPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		private:
			/** ステートマシン */
			std::unique_ptr<DaddyPenguinStateMachine> m_stateMachine;
			/** プレイヤーコントローラー */
			std::unique_ptr<DaddyPenguinController> m_controller;
		};
	}
}