/**
 * @file NaughtyChildPenguinStateMachine.h
 * @brief ヤンチャペンギンのステートマシン
 * @author 立山
 */
#pragma once
#include "ChildPenguinStateMachine.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace actor
	{
		class ChildPenguin;

		class NaughtyChildPenguinStateMachine : public ChildPenguinStateMachine
		{
		public:
			inline ChildPenguin* GetOwnerChildPenguin() const { return m_ownerChildPenguin; }

			inline void SetIsGoingToWakeBear(bool v) { m_isGoingToWakeBear = v; }
			inline bool GetIsGoingToWakeBear() const { return m_isGoingToWakeBear; }

			inline void SetIsAtBear(bool v) { m_isAtBear = v; }
			inline bool GetIsAtBear() const { return m_isAtBear; }

			inline void SetBearTargetPos(const Vector3& pos) { m_bearTargetPos = pos; }
			inline const Vector3& GetBearTargetPos() const { return m_bearTargetPos; }

		public:
			NaughtyChildPenguinStateMachine(ChildPenguin* ownerChildPenguin);
			~NaughtyChildPenguinStateMachine() = default;

		protected:
			core::IState* GetTypeSpecificChangeState() override;

		private:
			ChildPenguin* m_ownerChildPenguin;

			/** シロクマを起こしに行くフラグ（AIがtrueにし、WakeBear終了時にリセット） */
			bool m_isGoingToWakeBear = false;
			/** シロクマへの到達フラグ（AIがtrueにし、WakeBear終了時にリセット） */
			bool m_isAtBear = false;
			/** シロクマの座標（AIがセット） */
			Vector3 m_bearTargetPos;
		};
	}
}