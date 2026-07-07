/**
 * @file FrontChecker.h
 * @brief 前方チェッククラス
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace ui
	{
		/**
		 * @brief 前方チェッククラス
		 * @details 前方にいるかどうかを判定する
		 */
		class FrontChecker
		{
		public:
			/**
			 * @brief 前方チェックを行う
			 * @param basePosition 基準となるオブジェクトの位置
			 * @param baseRotation 基準となるオブジェクトの回転
			 * @param targetPosition 判定対象のオブジェクトの位置
			 * @param dotThreshold 内積の閾値（これ以上なら前方とみなす）
			 * @return 前方にいる場合はtrue、そうでない場合はfalse
			 */
			static bool IsInFront(
				const Vector3& basePosition,
				const Quaternion& baseRotation,
				const Vector3& targetPosition,
				float dotThreshold = 0.0f
			);


		private:
			// インスタンス化させない
			FrontChecker() = delete;
			~FrontChecker() = delete;
		};
	}
}