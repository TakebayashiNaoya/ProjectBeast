/**
 * @file OrcaStatus.h
 * @brief シャチのステータス
 */
#pragma once
#include "Source/Actor/Character/CharacterStatus.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		struct MasterOrcaParameter;


		/**
		 * @brief シャチのステータス
		 * @details OrcaParameter.bin の値を持つ。値の読み方は EnemyStatus と同じ
		 *          （毎フレーム Setup() で読み直し、ホットリロードに追従する）。
		 *          スタミナの関数名は EnemyStatus に合わせている。
		 */
		class OrcaStatus : public CharacterStatus
		{
		public:
			/**
			 * @brief 読み込んだパラメーターをそのまま取得する
			 * @return パラメーター。読み込めていなければ nullptr
			 */
			const MasterOrcaParameter* GetParameter() const { return m_parameter; }

			/** @brief 満腹になる数 */
			int GetMaxEat() const;

			/** @brief 現在のスタミナ */
			inline float GetStamina() const { return m_stamina; }
			/** @brief スタミナが無くなったか */
			inline bool IsStaminaEmpty() const { return m_stamina <= 0.0f; }
			/**
			 * @brief スタミナを減らす
			 * @param amount 減らす量
			 */
			void DecreaseStamina(float amount);
			/** @brief スタミナを全回復する */
			void FullRecoverStamina();


		public:
			void Setup() override;
			void Update() override;


		public:
			OrcaStatus();
			~OrcaStatus() override;


		private:
			/** 未初期化を表すスタミナ */
			static constexpr float UNINITIALIZED_STAMINA = -1.0f;

			/** 読み込んだパラメーター */
			const MasterOrcaParameter* m_parameter = nullptr;
			/** 現在のスタミナ */
			float m_stamina = UNINITIALIZED_STAMINA;
		};
	}
}
