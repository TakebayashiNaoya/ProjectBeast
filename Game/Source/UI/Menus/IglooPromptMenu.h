/**
 * @file IglooPromptMenu.h
 * @brief かまくら入口でAボタンアイコンを表示するクラス
 */
#pragma once
#include "Source/UI/Menu.h"


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
			enum class PromptType
			{
				None,
				Enter,
				Exit
			};


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

			/**
			 * @brief プロンプトの種類を設定する
			 * @param type プロンプトの種類
			 */
			inline void SetPromptType(PromptType type) { m_promptType = type; }


		private:
			/** 表示対象のワールド座標（親ペンギンの位置） */
			Vector3 m_targetPosition = Vector3::Zero;

			/** 描画フラグ */
			bool m_isDraw = false;

			PromptType m_promptType = PromptType::None;
		};
	}
}