/**
 * @file DaddyPenguinStatus.h
 * @brief 親ペンギンのステータス
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinStatus.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 親ペンギンのステータスクラス
		 */
		class DaddyPenguinStatus : public PenguinStatus
		{
		public:
			// ここに親ペンギン固有のステータス用のゲッター関数を追加していく
			/**
			 * @brief 命令が届く範囲を取得
			 * @return 命令が届く範囲
			 */
			inline float GetEnableCommandRange() const { return m_enableCommandRange; }


		public:
			/*
			 * @brief セットアップ
			 * @note ステータスの持ち主が呼び出す
			 */
			void Setup() override;


			void Update() override;


		public:
			DaddyPenguinStatus();
			~DaddyPenguinStatus() override;


		private:
			// ここに親ペンギン固有のステータスを追加していく
			/** 命令が届く範囲 */
			float m_enableCommandRange;
		};
	}
}

