/**
 * @file IglooPromptMenu.h
 * @brief かまくら入口でAボタンアイコンを表示するクラス
 * @author （担当者名）
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief かまくらの入口付近にいるとき、親ペンギンの頭上にAボタン画像を表示するUI
		 *
		 * 使い方（GameScene等で毎フレーム呼び出す）:
		 *   m_iglooPromptMenu->SetTargetPosition(daddyPenguin->GetTransform().m_position);
		 *   m_iglooPromptMenu->SetDraw(isNearIgloo);
		 */
		class IglooPromptMenu : public MenuBase
		{
		public:
			IglooPromptMenu();
			~IglooPromptMenu() = default;

			void Update() override;
			void InitializeLogic() override;


		public:
			/**
			 * @brief アイコンを表示する対象のワールド座標を設定する（親ペンギンの座標を渡す）
			 * @param position ワールド座標
			 */
			inline void SetTargetPosition(const Vector3& position) { m_targetPosition = position; }

			/**
			 * @brief 描画するかどうかを設定する
			 * @param isDraw true のとき表示、false のとき非表示
			 */
			inline void SetDraw(bool isDraw) { m_isDraw = isDraw; }


		private:
			/** 表示対象のワールド座標（親ペンギンの位置） */
			Vector3 m_targetPosition = Vector3::Zero;

			/** 描画フラグ */
			bool m_isDraw = false;
		};
	}
}