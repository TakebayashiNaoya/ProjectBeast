/**
 * @file FormationDebugMonitor.h
 * @brief 陣形の状態をImGuiデバッグウィンドウで監視するクラス
 */
#pragma once

namespace app
{
	namespace actor
	{
		class ChildPenguinManager;

		/**
		 * @brief 陣形（フォーメーション）の状態を app::DebugWindow に表示するクラス
		 * @details ChildPenguinManager が所有し、コンストラクタ/デストラクタで
		 *          DebugWindow への登録・解除を行う（デバッグビルドのみ生成される）
		 */
		class FormationDebugMonitor
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param manager 監視対象の ChildPenguinManager（非所有）
			 */
			explicit FormationDebugMonitor(const ChildPenguinManager* manager);
			~FormationDebugMonitor();

		private:
			/** DebugWindow に登録する描画関数本体 */
			void Draw() const;

			/** 監視対象（非所有） */
			const ChildPenguinManager* m_manager;
		};
	}
}
