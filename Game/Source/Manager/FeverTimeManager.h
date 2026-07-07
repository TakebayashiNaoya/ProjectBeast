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
	 * @detail 残り時間が既定値を下回るとフィーバータイムに入り、
	 *         それまでの捕獲数によらず固定数の子ペンギンを上空から降らせる。
	 *         フィーバー中にプレイヤーが子ペンギンを捕獲するたびに、
	 *         捕獲した分だけ投下キューへ追加され、連続して降り続ける。
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
		 * @brief 子ペンギンが1匹捕獲された時に呼ぶ
		 * @detail フィーバータイム中であれば、捕獲された分だけ投下キューに積む
		 */
		void OnPenguinCaught();


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
		float m_feverStartTime	 = 30.0f;	 /** 終了何秒前にフィーバータイムへ入るか（JSONで上書きされる） */
		float m_dropInterval	 = 0.3f;	 /** 投下間隔（秒）（JSONで上書きされる） */
		float m_dropHeight		 = 1500.0f;  /** 投下する上空の高さ（地面からのオフセット）（JSONで上書きされる） */
		int   m_feverDropCount	 = 100;		 /** フィーバー開始時に投下キューへ積む固定数（JSONで上書きされる） */
		int   m_pendingDropCount = 0;		 /** 投下待ちの子ペンギンの数（キュー） */
		bool  m_isActive		 = false;	 /** フィーバータイム中かどうか */


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
