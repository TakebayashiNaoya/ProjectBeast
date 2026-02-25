/**
 * @file SoundManager.h
 * @brief サウンドの管理をするクラス
 * @author 立山
 */
#pragma once
#include "Source/Sound/Types.h"


namespace app
{
	/** SE用のハンドル名 */
	using SEHandle = uint32_t;
	/** ハンドル無効値 */
	static constexpr SEHandle INVALID_SE_HANDLE = 0xffffffff;

	/** Voice用のハンドル名 */
	using VoiceHandle = uint32_t;
	/** ハンドル無効値 */
	static constexpr VoiceHandle INVALID_VOICE_HANDLE = 0xffffffff;


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
		SEHandle PlaySE(const int kind, const bool isLoop = false, const bool is3D = false);
		/** SEの停止 */
		void StopSE(const SEHandle handle);


		/** Voiceの再生 */
		VoiceHandle PlayVoice(const int kind, const bool isLoop = false, const bool is3D = false);
		/** Voiceの停止 */
		void StopVoice(const VoiceHandle handle);


		SoundSource* FindSE(const SEHandle handle)
		{
			auto it = m_seList.find(handle);
			if (it != m_seList.end()) {
				return it->second;
			}
			K2_ASSERT(false, "削除済みか追加されていないSEにアクセスしようとしています。\n");
			return nullptr;
		}


	public:
		/** 全体の音量設定 */
		void SetMasterVolume(float volume);
		/** BGMの音量設定 */
		void SetBGMVolume(float volume);
		/** SEの音量設定 */
		void SetSEVolume(float volume);
		/** Voiceの音量設定 */
		void SetVoiceVolume(float volume);


		/** 全体の音量を取得 */
		float GetMasterVolume() const
		{
			return m_masterVolume;
		}
		/** BGMの音量を取得 */
		float GetBGMVolume() const
		{
			return m_bgmVolume;
		}
		/** SEの音量を取得 */
		float GetSEVolume() const
		{
			return m_seVolume;
		}
		/** Voiceの音量を取得 */
		float GetVoiceVolume() const
		{
			return m_voiceVolume;
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
		 * @detail main.cppで呼び出す
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
		SoundManager();
		~SoundManager();


		void ApplyBGMVolume();
		void ApplySEVolume();
		void ApplyVoiceVolume();


	private:
		/** BGM用のサウンドソースインスタンスを保持する */
		SoundSource* m_bgm = nullptr;
		/** SE用のサウンドソースインスタンスを保持する */
		std::map<SEHandle, SoundSource*> m_seList;
		/** Voice用のサウンドソースインスタンスを保持する */
		std::map<SEHandle, SoundSource*> m_voiceList;
		/**
		 * マップで参照するようにハンドル数を保持
		 */
		SEHandle m_soundHandleCount = 0;


	private:
		/** 全体の音量 */
		float m_masterVolume = 1.0f;
		/** BGMの音量 */
		float m_bgmVolume = 1.0f;
		/** SEの音量 */
		float m_seVolume = 1.0f;
		/** ボイスの音量 */
		float m_voiceVolume = 1.0f;


	private:
		/** シングルトンインスタンス */
		static SoundManager* m_instance;
	};
}
