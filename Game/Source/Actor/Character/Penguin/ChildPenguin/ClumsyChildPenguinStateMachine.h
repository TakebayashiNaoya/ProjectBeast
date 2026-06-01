/**
 * @file ClumsyChildPenguinStateMachine.h
 * @brief おっちょこちょいペンギンのステートマシン
 * @author 竹林
 */
#pragma once
#include "ChildPenguinStateMachine.h"

#include "Source/Effect/EffectManager.h"

namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;


		/**
		 * @brief おっちょこちょいペンギンのステートマシンクラス
		 * @details 転倒・スリップ・起き上がりの固有ステートを持つ
		 */
		class ClumsyChildPenguinStateMachine : public ChildPenguinStateMachine
		{
		public:
			/**
			 * @brief オーナーの子ペンギンを取得する
			 * @return 子ペンギンのポインタ
			 * @note ClumsyChildPenguinIState から ChildPenguinManager へ通知する際に使用する
			 */
			inline ChildPenguin* GetOwnerChildPenguin() const
			{
				return m_ownerChildPenguin;
			}

			/**
			 * @brief 転倒フラグを設定する
			 * @param isTripped 転倒フラグ
			 */
			inline void SetIsTripped(const bool isTripped)
			{
				m_isTripped = isTripped;
			}

			/**
			 * @brief スリップフラグを設定する
			 * @param isSlipped スリップフラグ
			 */
			inline void SetIsSlipped(const bool isSlipped)
			{
				m_isSlipped = isSlipped;
			}

			/**
			 * @brief 世話焼きペンギンに助けられたフラグを設定する
			 * @param isHelped 助けられフラグ
			 * @note ClumsyChildPenguinAIがtrueにし、StandUpState::Exit()でfalseにリセットされる
			 */
			inline void SetIsHelped(const bool isHelped)
			{
				m_isHelped = isHelped;
			}


			inline void SetCryEffectHandle(const EffectHandle handle)
			{
				m_cryEffectHandle = handle;
			}


			inline EffectHandle GetCryEffectHandle() const
			{
				return m_cryEffectHandle;
			}


			/**
			 * @brief 転倒中かどうかを取得する
			 * @return 転倒中ならtrue
			 */
			inline bool GetIsTripped() const
			{
				return m_isTripped;
			}

			/**
			 * @brief スリップ中かどうかを取得する
			 * @return スリップ中ならtrue
			 */
			inline bool GetIsSlipped() const
			{
				return m_isSlipped;
			}

			/**
			 * @brief 世話焼きペンギンに助けられたかどうかを取得する
			 * @return 助けられたならtrue
			 */
			inline bool GetIsHelped() const
			{
				return m_isHelped;
			}


		public:
			ClumsyChildPenguinStateMachine(ChildPenguin* ownerChildPenguin);
			~ClumsyChildPenguinStateMachine() = default;


		protected:
			/**
			 * @brief タイプ固有のステート遷移（オーバーライド）
			 * @return 遷移先ステート。遷移不要ならnullptr
			 */
			core::IState* GetTypeSpecificChangeState() override;


		private:
			/** オーナーの子ペンギン（IState から Manager へ通知するために保持） */
			ChildPenguin* m_ownerChildPenguin;
			/** 転倒フラグ（AIコントローラーがtrueにし、TripState::Exit()でfalseにリセット） */
			bool m_isTripped = false;
			/** スリップフラグ（AIコントローラーがtrueにし、SlipState::Exit()でfalseにリセット） */
			bool m_isSlipped = false;
			/** 世話焼きペンギンに助けられたフラグ（転倒・スリップ中の即復帰に使う） */
			bool m_isHelped = false;
			/** おっちょこちょいペンギンのエフェクトのハンドル */
			EffectHandle m_cryEffectHandle = INVALID_EFFECT_HANDLE;
		};
	}
}