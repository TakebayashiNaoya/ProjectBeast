/**
 * @file LevelUpIconMenu.h
 * @brief 陣形レベルアップ時に親ペンギンの頭上へ表示するアイコンの演出メニュー
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/UI/Model/LevelUpIconAnimStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 陣形レベルアップ時に親ペンギンの頭上へ表示するアイコンの演出メニュー
		 * @details
		 *   内部にUIIconを1つ持つ。ワールド座標をスクリーン座標へ変換して
		 *   親ペンギンの頭上に追従させつつ、Play()から一定時間かけて
		 *   上昇させながら、透明→不透明→透明とフェードさせる。
		 *   色のフェードはUIAnimationFactoryで生成したUIColorAnimationを
		 *   フェードイン用→フェードアウト用に差し替えながら再生し、
		 *   色の補間自体はUIColorAnimation側に任せる。こちら側はフェードイン→
		 *   待機→フェードアウトのタイミング管理と、上に浮き上がる移動の更新だけを行う。
		 */
		class LevelUpIconMenu : public MenuBase
		{
		public:
			LevelUpIconMenu();
			~LevelUpIconMenu() override;

			void Update() override;
			void InitializeLogic() override;


		public:
			/**
			 * @brief 追従対象のワールド座標を設定する（親ペンギンの座標を毎フレーム渡す）
			 * @param position 追従対象のワールド座標
			 */
			inline void SetTargetPosition(const Vector3& position) { m_targetPosition = position; }

			/**
			 * @brief レベルアップ演出を再生する
			 */
			void Play();


		private:
			/** レベルアップアイコン専用のアニメーションステータス */
			std::unique_ptr<LevelUpIconAnimStatus> m_animStatus;

			/** レベルアップアイコン */
			UIIcon* m_icon;

			/** 追従対象のワールド座標（毎フレーム外部から設定される） */
			Vector3 m_targetPosition;

			bool m_isPlaying;
			bool m_isFadeInFinished;
			bool m_isFadeOutStarted;

			float m_elapsedTime; // Play()からの経過時間（浮き上がり演出に使用）
			float m_waitTimer;   // フェードイン終了後の待機経過時間
		};
	}
}
