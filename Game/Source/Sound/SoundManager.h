/**
 * @file SoundManager.h
 * @brief サウンドの管理をするクラス
 * @author 立山
 */
#pragma once
#include "Source/Sound/Types.h"
#include "Source/Sound/SoundHandle.h"
#include <optional>


namespace app
{
	/** 無効な音量値 */
	static constexpr float INVALID_VOLUME = -1.0f;
	static constexpr float DEFAULT_VOLUME_MAGNIFICATION = 1.0f;


	/** サウンド再生の優先度 */
	enum EnSoundPriority
	{
		enSoundPriority_Hight,
		enSoundPriority_Normal,
		enSoundPriority_Low,
		enSoundPriority_Max,
	};



	/**
	 * サウンドを管理するクラス
	 */
	class SoundManager
	{
	private:
		/**
		 * @brief サウンドの情報の構造体
		 */
		struct SoundInformation
		{
			/** サウンドの種類 */
			int m_kind;
			/** ループ再生するか */
			bool m_isLoop;
			/** 3Dサウンドか */
			bool m_is3D;
			/** 音量 */
			float m_volumeMagnification;


			SoundInformation(const int kind, const bool isLoop, const bool is3D, const float volumeMagn)
				: m_kind(kind)
				, m_isLoop(isLoop)
				, m_is3D(is3D)
				, m_volumeMagnification(volumeMagn)
			{}
			~SoundInformation() = default;
		};


		/**
		 * SE再生用の情報
		 */
		struct SEInformation : public SoundInformation
		{
			/** 再生中のSEのハンドル */
			SEHandle m_handle;


			SEInformation(const int kind, const bool isLoop, const bool is3D, const SEHandle handle, const float volumeMagn)
				: SoundInformation(kind, isLoop, is3D, volumeMagn)
				, m_handle(handle)
			{}
			~SEInformation() = default;
		};


		/**
		 * @brief BGM再生用の情報
		 * @detail BGMはゲーム上に1つしか存在せず、必ずループ再生される
		 *		   （isLoopは常にtrue固定、SoundManager側で1インスタンスのみ保持することで単一性を保証する）
		 */
		struct BGMInformation : public SoundInformation
		{
			BGMInformation(const int kind, const float volumeMagn)
				: SoundInformation(kind, /*isLoop=*/true, /*is3D=*/false, volumeMagn)
			{}
			~BGMInformation() = default;
		};


	public:
		/**
		 * 更新処理
		 * @brief 不要なインスタンスを削除したりする
		 * @detail main.cppで呼び出す
		 */
		void Update();


	public:
		/** BGMの再生（BGMは常に1つ・必ずループ） */
		void PlayBGM(const int kind, const float volumeMagnification = INVALID_VOLUME);
		/** BGMの停止 */
		void StopBGM();
		/** BGMを指定時間かけて徐々にフェードアウトし、完了後に停止する */
		void FadeOutBGM(const float duration);


		/**
		 * SEの再生のリクエスト
		 * NOTE: フレームの最後でまとめて再生される
		 *		 再生されない可能性があるのでHandleから取得する際はnullptrチェックをすること
		 */
		SEHandle PlaySE(const int kind, const float volumeMagnification = INVALID_VOLUME, const bool isLoop = false, const bool is3D = false, const EnSoundPriority priority = enSoundPriority_Normal);
		/** SEの停止 */
		void StopSE(const SEHandle handle);
		/** 全てのSEを停止 */
		void StopAllSE();


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
			//K2_ASSERT(false, "削除済みか追加されていないSEにアクセスしようとしています。\n");
			return nullptr;
		}


	public:
		/**
		 * @brief BGMが再生中かどうかを確認する
		 * @return 再生中ならtrue
		 */
		bool IsPlayingBGM() const
		{
			return (m_bgm != nullptr) && m_bgm->IsPlaying();
		}

		/**
		 * @brief 指定したハンドルのSEが再生中かどうかを確認する
		 * @param handle 確認するSEのハンドル
		 * @return 再生中ならtrue（見つからない・再生していない場合はfalse）
		 */
		bool IsPlayingSE(const SEHandle handle) const
		{
			auto it = m_seList.find(handle);
			if (it != m_seList.end() && it->second != nullptr) {
				return it->second->IsPlaying();
			}
			return false;
		}

		/**
		 * @brief 何かしらのSEが再生中かどうかを確認する
		 * @return 1つでも再生中のSEがあればtrue
		 */
		bool IsPlayingAnySE() const
		{
			for (const auto& it : m_seList) {
				if (it.second != nullptr && it.second->IsPlaying()) {
					return true;
				}
			}
			return false;
		}

		/**
		 * @brief 指定したハンドルのVoiceが再生中かどうかを確認する
		 * @param handle 確認するVoiceのハンドル
		 * @return 再生中ならtrue（見つからない・再生していない場合はfalse）
		 */
		bool IsPlayingVoice(const VoiceHandle handle) const
		{
			auto it = m_voiceList.find(handle);
			if (it != m_voiceList.end() && it->second != nullptr) {
				return it->second->IsPlaying();
			}
			return false;
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
		 * @detail Application.cppで呼び出す
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
		 * @detail Application.cppで呼び出す
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
		/**
		 * 現在再生中のBGMの情報
		 * @detail BGMはゲーム上に1つしか存在しないため、リストではなく単一のoptionalで保持する
		 */
		std::optional<BGMInformation> m_bgmInformation;
		/** SE用のサウンドソースインスタンスを保持する */
		std::map<SEHandle, SoundSource*> m_seList;
		/** Voice用のサウンドソースインスタンスを保持する */
		std::map<SEHandle, SoundSource*> m_voiceList;
		/**
		 * マップで参照するようにハンドル数を保持
		 */
		SEHandle m_soundHandleCount = 0;

		/** SE再生のリクエスト用情報 */
		std::vector<SEInformation> m_seInfomationList[enSoundPriority_Max];

		/** 再生中SEのKindを管理 */
		std::map<SEHandle, int>m_seHandleKindMap;

		/** SE種別ごとの同時再生数上限 */
		std::unordered_map<int, uint8_t> m_seConcurrentLimitMap;

		/** BGMフェードアウト中かどうか */
		bool m_isBgmFading = false;
		/** BGMフェードアウトの経過時間 */
		float m_bgmFadeTimer = 0.0f;
		/** BGMフェードアウトにかける時間（秒） */
		float m_bgmFadeDuration = 0.0f;


	private:
		/** 全体の音量 */
		float m_masterVolume = 0.1f;
		/** BGMの音量 */
		float m_bgmVolume = 0.1f;
		/** SEの音量 */
		float m_seVolume = 0.1f;
		/** ボイスの音量 */
		float m_voiceVolume = 0.1f;


	private:
		/** シングルトンインスタンス */
		static SoundManager* m_instance;
	};
}