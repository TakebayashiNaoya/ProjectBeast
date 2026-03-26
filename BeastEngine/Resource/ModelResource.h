#pragma once

#include <memory>
#include <string>

#include "ResourceManager.h"
#include "SkeletonResource.h"
#include "AnimationClipResource.h"

// 前方宣言（ポインタ/参照のみ利用）
namespace nsK2EngineLow {
	class TkmFile;
	struct ModelInitData;
	class Skeleton;
	class AnimationClip;
}

namespace nsBeastEngine
{
 using nsK2EngineLow::ModelInitData;
	using nsK2EngineLow::Skeleton;
	using nsK2EngineLow::AnimationClip;

	/**
	 * @brief tkmファイルを非同期ロードし、バンクに登録するためのリソース
	 *
	 * 共有：tkmファイル（メッシュ/マテリアル等の不変データ）
	 * インスタンス：ModelRender側で個別に Model を生成してワールド行列などの状態を保持する
	 */
	class TkmResource : public IResource
	{
		friend class TkmLoader;

	public:
		TkmResource() = default;
		~TkmResource() = default;

		/**
		 * @brief バンク登録を行い、ModelInitData に tkm パスを設定する（メインスレッドで呼ぶ）
		 * @details IsCompleted()==true を確認した上で呼び出すこと
		 */
		void Finalize(ModelInitData& outInitData)
		{
			// ワーカーが読み込んだ tkm をバンクへ登録（キャッシュヒット時は m_tkmFile=nullptr）
			if (m_tkmFile)
			{
				g_engine->RegistTkmFileToBank(m_filePath.c_str(), m_tkmFile.get());
				m_tkmFile.release();
			}

			// ModelInitData にパスを渡す（以降、Model.Initでバンクを参照できる）
			outInitData.m_tkmFilePath = m_filePath.c_str();
		}

	private:
		std::string                         m_filePath;
		std::unique_ptr<nsK2EngineLow::TkmFile> m_tkmFile;
	};


	class TkmLoader : public ResourceLoader<TkmResource>
	{
	public:
		bool LoadImpl(TkmResource& resource, const std::string& key) override
		{
			resource.m_filePath = key;

			// 既にバンクにある場合はI/O不要
			nsK2EngineLow::TkmFile* banked = g_engine->GetTkmFileFromBank(key.c_str());
			if (banked != nullptr)
			{
				return true;
			}

           // tkmファイルをロード（ワーカースレッド側でI/Oのみ実行）
			auto tkmFile = std::make_unique<nsK2EngineLow::TkmFile>();
			if (!tkmFile->Load(key.c_str(), false))
			{
				return false;
			}
			resource.m_tkmFile = std::move(tkmFile);
			return true;
		}
	};


	/**
	 * @brief tkmの非同期ロード＋Finalizeを簡便化するヘルパ
	 */
	class TkmModelLoader
	{
	public:
		void RequestLoad(ResourceManager& resourceManager, const char* filePath)
		{
			m_handle = resourceManager.Load<TkmResource>(filePath);
		}

		bool IsReady() const
		{
			return m_handle && (m_handle->IsCompleted() || m_handle->IsError());
		}

		void Finalize(ModelInitData& initData)
		{
			if (m_handle && m_handle->IsCompleted())
			{
				m_handle->Finalize(initData);
			}
		}

		void Reset()
		{
			m_handle.reset();
		}

	private:
		std::shared_ptr<TkmResource> m_handle;
	};


	/**
	 * @brief tkm / tks / tka をまとめて扱うヘルパ
	 * キャラクター系（tkm+tks+tka）とステージ系（tkmのみ）の両方で共通利用できる
	 */
	class ModelAssetsLoader
	{
	public:
		void Request(ResourceManager& resourceManager,
			const char* tkmPath,
			const char* tksPath = nullptr,
			const char* const* tkaPaths = nullptr,
			int numClips = 0)
		{
			m_hasTkm = tkmPath != nullptr;
			m_hasTks = (tksPath != nullptr && tksPath[0] != '\0');
			m_hasTka = (tkaPaths != nullptr && numClips > 0);
			m_tksPath = tksPath ? tksPath : "";
			m_numClips = m_hasTka ? numClips : 0;

			if (m_hasTkm) { m_tkm.RequestLoad(resourceManager, tkmPath); }
			if (m_hasTks) { m_tks.RequestLoad(resourceManager, tksPath); }
			if (m_hasTka) { m_tka.RequestLoad(resourceManager, tkaPaths, numClips); }
		}

		bool IsReady() const
		{
			if (m_hasTkm && !m_tkm.IsReady()) return false;
			if (m_hasTks && !m_tks.IsReady()) return false;
			if (m_hasTka && !m_tka.IsReady()) return false;
			return true;
		}

		// skeleton/clips は必要な場合のみ引数を渡す。不要なら nullptr でOK。
		void Finalize(ModelInitData& initData, Skeleton* skeleton = nullptr, AnimationClip* clips = nullptr)
		{
			if (m_hasTkm) { m_tkm.Finalize(initData); }
			if (m_hasTks)
			{
				// 非同期ロード済みの TksFile をバンクに登録
				m_tks.Finalize();
				// バンクヒットにより二重ロードを防止
				if (skeleton) { skeleton->Init(m_tksPath.c_str()); }
			}
			if (m_hasTka && clips) { m_tka.Finalize(clips); }
		}

		void Reset()
		{
			m_tkm.Reset();
			m_tks.Reset();
			m_tka.Reset();
			m_hasTkm = m_hasTks = m_hasTka = false;
			m_numClips = 0;
			m_tksPath.clear();
		}

		int GetNumClips() const { return m_numClips; }

	private:
		TkmModelLoader	  m_tkm;
		TksSkeletonLoader m_tks;
		TkaClipSetLoader  m_tka;
		bool m_hasTkm = false;
		bool m_hasTks = false;
		bool m_hasTka = false;
		int  m_numClips = 0;
		std::string m_tksPath;
	};
}
