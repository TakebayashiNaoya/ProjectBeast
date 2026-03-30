/**
 * @file BattleManager.h
 * @brief バトルの管理をするクラス（クラス間の情報受け渡し）
 * @author 立山
 */
#pragma once


namespace app
{
	/**
	 * @brief バトルの情報受け渡しクラス
	 * @detail ゲームのフェーズ管理は行わない。
	 *         各クラスが必要な情報をここから取得・設定する。
	 */
	class BattleManager
	{
	public:
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new BattleManager();
			}
		}

		static BattleManager& GetInstance()
		{
			return *m_instance;
		}

		static void DestroyInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}


		//============================================//
		// ゲームアクティブ状態
		// InGameScene が Playing 中のときのみ true をセットする。
		// プレイヤー・AI・シロクマ等は IsGameActive() を参照して
		// 行動してよいかを判断できる。
		//============================================//

		/**
		 * @brief ゲームがアクティブ（プレイ中）かどうかを取得
		 */
		bool IsGameActive() const { return m_isGameActive; }

		/**
		 * @brief ゲームアクティブ状態を設定
		 * @param isActive true = Playing中、false = それ以外
		 */
		void SetGameActive(bool isActive) { m_isGameActive = isActive; }


		//============================================//
		// ゲーム結果
		// 終了判定が確定した時点で InGameScene がセットする。
		//============================================//

		/**
		 * @brief クリアかどうかを取得
		 */
		bool IsClear() const { return m_isClear; }

		/**
		 * @brief クリア結果を設定
		 * @param isClear true = クリア、false = ゲームオーバー
		 */
		void SetClear(bool isClear) { m_isClear = isClear; }


	private:
		BattleManager();
		~BattleManager();

	private:
		/** ゲームがアクティブ（プレイ中）かどうか */
		bool m_isGameActive = false;
		/** クリアフラグ */
		bool m_isClear = false;

	private:
		static BattleManager* m_instance;
	};
}