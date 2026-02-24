/**
 * @file SoundManager.h
 * @brief サウンドの管理をするクラス
 * @author 立山
 */
#pragma once


namespace app
{
	/** サウンド用のハンドル名 */
	using SoundHandle = uint32_t;
	/** ハンドル無効値 */
	static constexpr SoundHandle INVALID_SOUND_HANDLE = 0xffffffff;


	/**
	 * サウンドを管理するクラス
	 */
	class SoundManager
	{
	public:
		/**
		 * 更新処理
		 * @brief 不要なインスタンスを削除したりする
		 * @detail main.cppで呼び出す
		 */
		void Update();


	public:
		/** BGMの再生 */
		void PlayBGM(const int kind);
		/** BGMの停止 */
		void StopBGM();


		/** SEの再生 */
		SoundHandle PlaySE(const int kind, const bool isLoop = false, const bool is3D = false);
		/** SEの停止 */
		void StopSE(const SoundHandle handle);


		SoundSource* FindSE(const SoundHandle handle)
		{
			auto it = m_seList.find(handle);
			if (it != m_seList.end()) {
				return it->second;
			}
			K2_ASSERT(false, "削除済みか追加されていないSEにアクセスしようとしています。\n");
			return nullptr;
		}


	public:
		/**
		 * インスタンスを作成
		 * @detail main.cppで呼び出す
		 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new SoundManager();
			}
		}


		/**
		 * インスタンスを取得
		 */
		static SoundManager& Get()
		{
			return *m_instance;
		}


		/**
		 * インスタンスを削除
		 */
		static void DestroyInstance()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}


	private:
		/** BGM用のサウンドソースインスタンスを保持する */
		SoundSource* m_bgm = nullptr;
		/** SE用のサウンドソースインスタンスを保持する */
		std::map<SoundHandle, SoundSource*> m_seList;
		/**
		 * マップで参照するようにハンドル数を保持
		 */
		SoundHandle m_soundHandleCount = 0;


	private:
		SoundManager();
		~SoundManager();


	private:
		/** シングルトンインスタンス */
		static SoundManager* m_instance;
	};
}
