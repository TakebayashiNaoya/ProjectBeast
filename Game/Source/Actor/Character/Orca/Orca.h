/**
 * @file Orca.h
 * @brief シャチ（敵）
 */
#pragma once
#include "Source/Actor/Character/CharacterBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class OrcaStateMachine;
		class OrcaSeaGrid;


		/**
		 * @brief シャチ（敵）
		 * @details 海の中だけを泳ぐ。物理ボディは持たず、位置は OrcaSeaGrid で海の中に留める。
		 *          シロクマ（Enemy）とは別のクラスで、Enemy のコードには触れない。
		 */
		class Orca : public CharacterBase
		{
		public:
			/** @brief ステートマシン */
			OrcaStateMachine* GetOrcaStateMachine() { return m_stateMachine.get(); }
			const OrcaStateMachine* GetOrcaStateMachine() const { return m_stateMachine.get(); }

			/** @brief 泳げる場所の地図を設定する（StartWrapper より前に呼ぶ） */
			void SetSeaGrid(const OrcaSeaGrid* grid);

			/** @brief 真上の水面の点（UI・音・視界の基準に使う） */
			Vector3 GetSurfacePosition() const;
			/** @brief 口先の点（攻撃距離・子の逃げる基準に使う。Y は水面） */
			Vector3 GetMouthPosition() const;

			/** @brief 海から出かけて引き戻した回数（0 であるべき） */
			int GetSeaViolationCount() const { return m_seaViolationCount; }

			inline int  GetLogId() const { return m_logId; }
			inline void SetLogId(int id) { m_logId = id; }


		public:
			Orca();
			~Orca() override;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;

			/** 海を泳ぐので足跡は出さない */
			bool ShouldSuppressFootprint() const override { return true; }

			/**
			 * @brief フレームの終わりに、海の中にいるかを確かめる
			 * @details ResolveMove を通っていれば破れることは無い。破れていたら最寄りの航行可能点へ戻し、回数を数える
			 */
			void CheckInsideSea();


		private:
			std::unique_ptr<OrcaStateMachine> m_stateMachine;
			const OrcaSeaGrid* m_seaGrid = nullptr;
			/** 出現した連結域（引き戻すときはここへ戻す） */
			int m_homeComponent = -1;
			/** 海から出かけて引き戻した回数 */
			int m_seaViolationCount = 0;
			/** ログ用の連番ID（OrcaManager が生成順に割り当てる） */
			int m_logId = -1;
		};
	}
}
