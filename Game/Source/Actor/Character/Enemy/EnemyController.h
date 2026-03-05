/**
 * @file EnemyController.h
 * @brief エネミーのコントローラー
 * @author 立山
 */
#pragma once


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Enemy;

		/**
		 * @brief エネミーのコントローラークラス
		 */
		class EnemyController :public Noncopyable
		{
		public:
			EnemyController();
			~EnemyController();

			bool Start();
			void Update();
			void Render(RenderContext& renderContext);


		public:
			/** 操作対象の設定 */
			void SetTarget(Enemy* target, int controllerIndex)
			{
				m_target = target;
				m_controllerIndex = controllerIndex;
			}


		private:
			/** 左スティックの入力があるか */
			inline bool IsInputStickL()
			{
				//左スティックの入力があるかどうかを判定
				if ((fabsf(GetPad()->GetLStickXF()) >= FLT_EPSILON) || (fabsf(GetPad()->GetLStickYF()) >= FLT_EPSILON))
				{
					return true;
				}
				return false;
			}
			/** 左スティックの入力量を取得 */
			Vector3 GetStickL();
			/** 左スティックによる方向を使って回転を計算 */
			Quaternion ComputeRotation();
			/** 対象のコントローラーを取得 */
			GamePad* GetPad()
			{
				return g_pad[m_controllerIndex];
			}


		private:
			Enemy* m_target;
			int m_controllerIndex = 0;
		};
	}
}

