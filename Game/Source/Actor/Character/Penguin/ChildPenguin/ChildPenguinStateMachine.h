/**
 * @file ChildPenguinStateMachine.h
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/Penguin/PenguinStateMachine.h"
#include "ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStatus;


		/**
		 * @brief 子ペンギンのステートマシンクラス
		 */
		class ChildPenguinStateMachine : public PenguinStateMachine
		{
		public:
			/**
			 * @brief 子ペンギンのステータスを取得
			 * @return 子ペンギンのステータスポインタ
			 */
			const ChildPenguinStatus* GetChildPenguinStatus() const;

			/**
			 * @brief ペンギンのステータスを取得（基底クラスのオーバーライド）
			 * @return ペンギンのステータスポインタ
			 */
			virtual const PenguinStatus* GetPenguinStatus() const override;

			/** ステートの変更先を取得する */
			core::IState* GetChangeState() override;


		public:
			/**
			 * @brief AIコントローラーの入力処理
			 * @note 子ペンギンのAIコントローラーから呼び出される
			 */
			void AIControllerInput(const Vector3& moveDirection, bool isDash, bool isJump, bool isSlide, bool isDive, bool isSeparateWater);
			/**
			 * @brief ダメージ処理
			 */
			void Damage() override;


		public:
			ChildPenguinStateMachine(ChildPenguin* ownerChildPenguin, EnChildPenguinType type);
			~ChildPenguinStateMachine() = default;


		protected:
			/**
			 * @brief タイプ固有のステート遷移
			 * @note 固有ステートへの遷移が必要な派生クラスでオーバーライドする
			 * @return 遷移先ステート。遷移不要ならnullptr
			 */
			virtual core::IState* GetTypeSpecificChangeState() { return nullptr; }


		private:
			/** 子ペンギンのポインタ */
			ChildPenguin* m_ownerChildPenguin;
			/** タイプを保持 */
			EnChildPenguinType m_type;
		};
	}
}

