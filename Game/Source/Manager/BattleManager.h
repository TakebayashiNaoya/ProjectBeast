/**
 * @file BattleManager.h
 * @brief バトルの管理をするクラス（クラス間の情報受け渡し）
 * @author 立山、竹林
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
		/** 更新処理 */
		void Update();


	private:
		BattleManager();
		~BattleManager();


	public:
		/**
		 * @brief バトルがアクティブかどうかを設定
		 */
		void SetIsActive(const bool isActive) { m_isActive = isActive; }
		/**
		 * @brief バトルがアクティブかどうかを取得
		 */
		bool IsActive() const { return m_isActive; }


	private:
		bool m_isActive = false;	/** バトルがアクティブかどうか（ゲーム開始前やリザルト画面などは非アクティブ） */




		//============================================//
		// ゲームの状態
		//============================================//

	public:
		/**
		 * @brief バトルの状態enum
		 */
		enum class EnBattleState : uint8_t
		{
			Playing,   /** プレイ中			*/
			Clear,     /** クリア			*/
			GameOver   /** ゲームオーバー	*/
		};

		/**
		 * @brief バトルの状態を設定
		 */
		EnBattleState GetBattleState() const { return m_battleState; }


	private:
		/**
		  * @brief バトルの状態を確認して返す
		  */
		EnBattleState CheckBattleState() const;


	private:
		/** バトルの状態 */
		EnBattleState m_battleState = EnBattleState::Playing;




		//============================================//
		// タイム関連
		//============================================//

	public:
		/**
		 * @brief 現在のタイムを設定
		 */
		void SetCurrentTime(const float time) { m_currentTime = time; }
		/**
		 * @brief タイムアップしているかどうかを設定
		 */
		void SetTimeUp(const bool isTimeUp) { m_isTimeUp = isTimeUp; }


	private:
		float m_currentTime = 0.0f;	/** 現在のタイム */
		bool m_isTimeUp = false;	/** タイムアップしているかどうか */




		//============================================//
		// シングルトン関連
		//============================================//

	public:
		/** インスタンスの生成 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new BattleManager();
			}
		}
		/** インスタンスの取得 */
		static BattleManager& GetInstance()
		{
			return *m_instance;
		}
		/** インスタンスの破棄 */
		static void DestroyInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}


	private:
		/** シングルトンインスタンス */
		static BattleManager* m_instance;
	};
}