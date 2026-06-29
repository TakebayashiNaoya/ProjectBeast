/**
 * @file BattleManager.h
 * @brief バトルの管理をするクラス（クラス間の情報受け渡し）
 * @author 竹林
 */
#pragma once
#include "Source/UI/MiniMap/MiniMapTypes.h"


namespace app
{
	/** 前方宣言 */
	namespace actor
	{
		class ChildPenguin;
		class DaddyPenguin;
	}


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
		BattleManager() = default;
		~BattleManager() = default;




		//============================================//
		// アクティブ状態
		//============================================//

	public:
		/**
		 * @brief バトルがアクティブかどうかを設定
		 */
		inline void SetIsActive(const bool isActive) { m_isActive = isActive; }
		/**
		 * @brief バトルがアクティブかどうかを取得
		 */
		inline bool IsActive() const { return m_isActive; }


	private:
		/** バトルがアクティブかどうか（ゲーム開始前などは非アクティブ） */
		bool m_isActive = false;




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
			Finished   /** ゲーム終了（タイムアップ or 全員救助）	*/
		};

		/**
		 * @brief バトルの状態を取得
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
		// UI通知用function
		//============================================//

	public:
		/**
		 * @brief タイマーUI通知functionを設定
		 * @param func 引数：現在タイム（秒）
		 */
		inline void SetOnTimeChanged(std::function<void(float)> func)
		{
			m_onTimeChanged = std::move(func);
		}

		/**
		 * @brief 残り子ペンギン数UI通知functionを設定
		 * @param func 引数：救助済み数、ステージ上の総数
		 */
		inline void SetOnRescuedNumChanged(std::function<void(int, int)> func)
		{
			m_onRescuedNumChanged = std::move(func);
		}

		/**
		 * @brief 睡眠中クマUI通知functionを設定
		 * @detail 探索ロジックはlambda内で完結させる（BattleManagerはプレイヤー座標を知らない）
		 * @param func 引数なし。lambda内でEnemyManagerへの探索と、UIへのセットを行う
		 */
		inline void SetOnSleepingEnemyChanged(std::function<void()> func)
		{
			m_onSleepingEnemyChanged = std::move(func);
		}


		inline void SetOnBearReactionChanged(std::function<void()> func)
		{
			m_onBearReactionChanged = std::move(func);
		}


	public:
		/**
		 * @brief ミニマップUI通知functionを設定
		 * @param func 引数なし。lambda内でUIへのセットを行う
		 */
		inline void SetOnMiniMapChanged(std::function<void(const ui::ActorPositions&)> func)
		{
			m_onMiniMapChanged = std::move(func);
		}

		/**
		 * @brief 全UI通知functionをリセット（InGameUIManager破棄時に呼ぶ）
		 */
		void ResetObservers();


	private:
		/** タイマーUI更新通知 */
		std::function<void(float)> m_onTimeChanged;

		/** 残り子ペンギン数UI更新通知 */
		std::function<void(int, int)> m_onRescuedNumChanged;

		/**
		 * 睡眠中クマUI更新通知
		 * 探索・UIセットはlambda内で完結する
		 */
		std::function<void()> m_onSleepingEnemyChanged;

		/** クマのリアクションUI更新通知 */
		std::function<void()> m_onBearReactionChanged;


		/** ミニマップUI更新通知 */
		std::function<void(const ui::ActorPositions&)> m_onMiniMapChanged;




		//============================================//
		// サブカメラ関連
		//============================================//

	private:
		/**
		 * @brief サブカメラの更新処理
		 * @details 攻撃中のシロクマを監視し、サブカメラのON/OFFと座標を制御する
		 */
		void UpdateSubCamera();


	private:
		/** サブカメラが起動中かどうか */
		bool m_isSubCameraActive = false;

		/**
		 * 最後に特定できた攻撃ターゲットの子ペンギン。
		 * EnemyController::EnterAttack()でm_foundPenguinがクリアされるため、
		 * 攻撃フェーズ中はここにキャッシュした値でカメラを追従し続ける。
		 * サブビュー終了時にnullptrへリセットする。
		 */
		const actor::ChildPenguin* m_lastTargetChild = nullptr;



		//============================================//
		// 親ペンギン関連
		//============================================//
	public:
		/**
		 * @brief 親ペンギンを設定
		 * @param dadyPenguin 親ペンギンのポインタ
		 */
		void SetDaddyPenguin(actor::DaddyPenguin* dadyPenguin) { m_daddyPenguin = dadyPenguin; }


		/**
		 * @brief 親ペンギンを取得
		 * @return 親ペンギンのポインタ
		 */
		actor::DaddyPenguin* GetDaddyPenguin() const { return m_daddyPenguin; }


	private:
		/** 親ペンギンのポインタ */
		actor::DaddyPenguin* m_daddyPenguin = nullptr;




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