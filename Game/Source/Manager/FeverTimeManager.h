/**
 * @file FeverTimeManager.h
 * @brief フィーバータイムを管理するクラス
 * @author 竹林
 */
#pragma once


namespace app
{
	/**
	 * @brief フィーバータイムを管理するクラス
	 * @detail ステージ上の子ペンギンを全て捕獲した瞬間にフィーバータイムへ入り、
	 *         それまでの捕獲数によらず固定数の子ペンギンを上空から降らせる。
	 *         フィーバー中にプレイヤーが子ペンギンを捕獲するたびに、
	 *         捕獲した分だけ投下キューへ追加され連続して降り続けるが、
	 *         1回のフィーバーで投下する総数はfeverDropCountを超えない。
	 *         チュートリアルなどfeverEnabledがfalseのステージではフィーバーは発生しない。
	 */
	class FeverTimeManager
	{
	public:
		/**
		 * @brief 設定JSONを読み込んで初期化する
		 * @param parameterJsonPath フィーバータイム設定JSONのパス
		 */
		void Start(const char* parameterJsonPath);

		/**
		 * @brief 更新処理（Playingフェーズ中に毎フレーム呼ぶ）
		 */
		void Update();

		/**
		 * @brief フィーバータイム中かどうか
		 */
		bool IsActive() const { return m_isActive; }

		/**
		 * @brief まだ投下していない子ペンギンがキューに残っているかどうか
		 * @detail フィーバー中はステージ上の総数がこれから増える予定があるということなので、
		 *         全員救助判定（BattleManager::CheckBattleState）で誤って終了させないために使う
		 */
		bool HasPendingDrops() const { return m_isActive && m_pendingDropCount > 0; }

		/**
		 * @brief フィーバー開始時に投下する固定数を取得
		 * @detail ミニマップのアイコン数など、フィーバーで増える分の枠を事前に確保する側で使用する
		 */
		int GetFeverDropCount() const { return m_feverDropCount; }

		/**
		 * @brief 子ペンギンが1匹捕獲された時に呼ぶ
		 * @detail フィーバータイム中であれば、捕獲された分だけ投下キューに積む。
		 *         ただし1回のフィーバーで投下する総数がfeverDropCountを超える場合は積まない
		 */
		void OnPenguinCaught();

		/**
		 * @brief ステージ上の子ペンギンを全て捕獲した瞬間に呼ぶ
		 * @detail feverEnabledがfalse、または既にこのステージでフィーバーが発生済みの場合は何もしない。
		 *         条件を満たす場合のみフィーバータイムを開始する
		 */
		void TryStartFeverOnAllCaught();


	private:
		/**
		 * @brief フィーバータイムを開始し、投下キューの初期数を積む
		 */
		void StartFever();


	private:
		FeverTimeManager() = default;
		~FeverTimeManager() = default;


	private:
		float m_dropTimer		 = 0.0f;	 /** 投下間隔用タイマー */
		float m_dropInterval	 = 0.3f;	 /** 投下間隔（秒）（JSONで上書きされる） */
		float m_dropHeight		 = 1500.0f;  /** 投下する上空の高さ（地面からのオフセット）（JSONで上書きされる） */
		int   m_feverDropCount	 = 100;		 /** 1回のフィーバーで投下する総数の上限（JSONで上書きされる） */
		int   m_pendingDropCount = 0;		 /** 投下待ちの子ペンギンの数（キュー） */
		int   m_totalQueuedCount = 0;		 /** 今回のフィーバーで投下キューに積んだ累計数（feverDropCountでクランプするために使う） */
		bool  m_isActive		 = false;	 /** フィーバータイム中かどうか */
		bool  m_feverEnabled	 = true;	 /** このステージでフィーバーを発生させるかどうか（JSONで上書きされる。チュートリアルはfalse） */
		bool  m_hasTriggered	 = false;	 /** このステージで既にフィーバーが発生済みかどうか（1ステージ1回のみ発生させるためのガード） */


		//============================================//
		// シングルトン関連
		//============================================//
	public:
		/**
		 * @brief インスタンスの生成
		 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new FeverTimeManager();
			}
		}

		/**
		 * @brief インスタンスの取得
		 */
		static FeverTimeManager* GetInstance()
		{
			return m_instance;
		}

		/**
		 * @brief インスタンスの破棄
		 */
		static void DestroyInstance()
		{
			delete m_instance;
			m_instance = nullptr;
		}


	private:
		/** シングルトンインスタンス */
		static FeverTimeManager* m_instance;
	};
}
