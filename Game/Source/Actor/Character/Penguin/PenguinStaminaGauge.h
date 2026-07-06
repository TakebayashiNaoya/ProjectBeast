/**
 * @file PenguinStaminaGauge.h
 * @brief ジャンプ・スライド共通で使う、スタミナ(オーバーヒート式)ゲージクラス
 * @author 忽那
 */
#pragma once


namespace app
{
	namespace actor
	{
		/**
		 * @brief スタミナ(オーバーヒート式)ゲージクラス
		 * @details
		 *  ・使用中はゲージが減少し、未使用中は回復する。
		 *  ・ゲージが0まで枯渇すると「ロック状態」になり、
		 *    最大値まで回復しきるまで再使用不可になる。
		 *  ・0になる前に使用をやめた場合はロックされず、
		 *    その時点の値から通常通り回復・再使用が可能。
		 *  （APEXのLスターのオーバーヒートと同様の挙動）
		 */
		class PenguinStaminaGauge
		{
		public:
			/**
			 * @brief コンストラクタ
			 * @param maxValue ゲージの最大値
			 * @param decreaseSpeed 使用中の減少速度（1秒あたり）
			 * @param recoverSpeed 未使用中の回復速度（1秒あたり）
			 */
			PenguinStaminaGauge(float maxValue, float decreaseSpeed, float recoverSpeed);
			~PenguinStaminaGauge() = default;

			/**
			 * @brief 毎フレーム呼び出す更新処理
			 * @param isUsing 現在使用中かどうか
			 * @param deltaTime フレームの経過時間
			 */
			void Update(bool isUsing, float deltaTime);

			/**
			 * @brief ゲージを即座に全消費する（ジャンプのような単発消費のアクション用）
			 * @note すでにロック中の場合は何もしない
			 */
			void ConsumeAll();

			/**
			 * @brief 使用可能かどうかを取得する
			 * @return ロック中でなければtrue
			 */
			bool CanUse() const { return !m_isLocked; }

			/**
			 * @brief ロック中(枯渇後、満タンになるまで使用不可の状態)かどうかを取得する
			 * @return ロック中かどうか
			 */
			bool IsLocked() const { return m_isLocked; }

			/**
			 * @brief 現在のゲージ値を取得する
			 * @return 現在値(0〜最大値)
			 */
			float GetCurrentValue() const { return m_currentValue; }

			/**
			 * @brief ゲージの最大値を取得する
			 * @return 最大値
			 */
			float GetMaxValue() const { return m_maxValue; }

			/**
			 * @brief UI表示用：現在値を0〜1に正規化した割合を取得する
			 * @return 0.0(空)〜1.0(満タン)の割合
			 */
			float GetRatio() const { return (m_maxValue > 0.0f) ? (m_currentValue / m_maxValue) : 0.0f; }

			/**
			 * @brief 満タン・ロック解除の初期状態にリセットする
			 */
			void Reset();


		private:
			/** ゲージの最大値 */
			float m_maxValue;
			/** 現在のゲージ値（0〜m_maxValue） */
			float m_currentValue;
			/** 使用中の減少速度（1秒あたり） */
			float m_decreaseSpeed;
			/** 未使用中の回復速度（1秒あたり） */
			float m_recoverSpeed;
			/** ロック中（枯渇後、満タンまで使用不可）かどうか */
			bool m_isLocked;
		};
	}
}
