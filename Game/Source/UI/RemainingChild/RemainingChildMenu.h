/**
 * @file RemainingChildMenu.h
 * @brief 子ペンギンの残り数表示クラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "RemainingAnimStatus.h"
#include "Source/UI/Modules/InGameStartingAnimLogic/InGameStartingAnimLogic.h"
#include "Source/Vfx/HomingParticleRender.h"


namespace app
{
	namespace ui
	{
		class RemainingChildMenu : public MenuBase
		{
		public:
			RemainingChildMenu();

			void Update() override;
			void InitializeLogic() override;
			void Render(RenderContext& rc) override;


		public:
			/**
			 * @biref 子ペンギンの救助数をアニメーションで設定
			 * @param num 救助数
			 */
			void SetChildNum(const int num);

			/**
			 * @brief 総子ペンギン数をアニメーションで設定
			 * @param num 総数
			 */
			void SetTotalNum(const int num);

			/**
			 * @brief 子ペンギンのポインタを設定
			 * @param childPenguin 子ペンギンのポインタ
			 */
			void SetTarget(actor::ChildPenguin* childPenguin);


		private:
			/** ゲーム開始時のアニメーションを更新する */
			void UpdateGameStartingAnimation();


		private:
			/** アニメーションのシーケンス */
			enum class SeqType : uint8_t
			{
				RemainPlus,
				RemainMinus,
				TotalPlus,
				TotalMinus,
				Max
			};


			/** アニメーションシーケンスの配列 */
			std::array<UIAnimationSequence, static_cast<int>(SeqType::Max)> m_sequences;
			/** std::unique_ptrでステータスを所有 */
			std::unique_ptr<RemainingAnimStatus> m_remainAnimStatus;
			/** パーティクルエフェクトレンダーをユニークポインタで所有 */
			std::unique_ptr<HomingParticleRender> m_homingRender;
			/** 集めたペンギン数 */
			int m_childNum;
			/** ステージ上の総ペンギン数 */
			int m_totalNum;
			/** 救助数の初回反映が済んだかどうか（ゲーム開始直後の初期表示を増加演出扱いしないためのガード） */
			bool m_isChildNumInitialized;
			/** 総数の初回反映が済んだかどうか（ゲーム開始直後の初期表示を増加演出扱いしないためのガード） */
			bool m_isTotalNumInitialized;
			/** パーティクルのターゲット位置 */
			Vector3 m_targetPosition;
			/** ゲーム開始時のアニメーションロジック */
			InGameStartingAnimLogic m_startingAnimLogic;
		};
	}
}