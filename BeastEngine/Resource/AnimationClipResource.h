#pragma once

namespace nsBeastEngine
{
	// =========================================================
	//  TkaResource / TkaLoader / TkaClipSetLoader
	//  tkaファイルのIOを非同期化する。
	//
	//  スレッドセーフ設計：
	//    ワーカースレッド → TkaFile::Load()のみ（バンク不使用）
	//    メインスレッド   → RegistTkaFileToBank() → AnimationClip::Load()
	// =========================================================
	class TkaResource : public IResource
	{
		friend class TkaLoader;

	public:
		TkaResource() = default;
		~TkaResource() = default;

		// [メインスレッドから呼ぶ]
		// バンク登録 → AnimationClip::Load() を安全に実行する。
		// IsCompleted() == true を確認してから呼ぶこと。
		void Finalize(nsK2EngineLow::AnimationClip& outClip)
		{
			// ワーカーが読んだ TkaFile をバンクに登録する。
			// m_tkaFile が nullptr の場合はバンク登録済み（キャッシュヒット）なので何もしない。
			if (m_tkaFile)
			{
				g_engine->RegistTkaFileToBank(m_filePath.c_str(), m_tkaFile.get());
				m_tkaFile.release(); // バンクが以後の寿命を管理する
			}

			// AnimationClip::Load() 内で GetTkaFileFromBank() を呼ぶ。
			// 上でバンク登録済みなので必ずキャッシュヒットし、二重IOは発生しない。
			outClip.Load(m_filePath.c_str());
		}

	private:
		std::string                              m_filePath;
		std::unique_ptr<nsK2EngineLow::TkaFile>  m_tkaFile;
	};


	class TkaLoader : public ResourceLoader<TkaResource>
	{
	public:
		// ワーカースレッドで TkaFile::Load()（ファイルIOのみ）を実行する。
		// TResourceBank には一切触れない。
		bool LoadImpl(TkaResource& resource, const std::string& key) override
		{
			resource.m_filePath = key;

			// バンクにキャッシュ済みならIOをスキップ
			nsK2EngineLow::TkaFile* banked = g_engine->GetTkaFileFromBank(key.c_str());
			if (banked != nullptr)
			{
				return true;
			}

			// ファイルIO（グローバル状態に触れないのでスレッドセーフ）
			auto tkaFile = std::make_unique<nsK2EngineLow::TkaFile>();
			tkaFile->Load(key.c_str());
			resource.m_tkaFile = std::move(tkaFile);

			return true;
		}
	};


	// 複数のtkaクリップをまとめて非同期ロードし、
	// 完了後にAnimationClip[]を構築するヘルパー。
	// CharacterBaseがメンバとして持つ。
	class TkaClipSetLoader
	{
	public:
		// ロードをリクエストする（ワーカースレッドが動き出す）
		void RequestLoad(
			ResourceManager& resourceManager,
			const char* const* filePaths,
			int numClips)
		{
			m_handles.clear();
			m_handles.reserve(numClips);
			for (int i = 0; i < numClips; i++)
			{
				m_handles.push_back(resourceManager.Load<TkaResource>(filePaths[i]));
			}
		}

		// 全クリップが揃ったか確認する（毎フレーム呼ぶ）
		bool IsReady() const
		{
			if (m_handles.empty()) return false;
			for (const auto& h : m_handles)
			{
				if (!h || !(h->IsCompleted() || h->IsError())) return false;
			}
			return true;
		}

		// [メインスレッドから呼ぶ] IsReady() == true を確認してから呼ぶこと。
		// outClips : RequestLoadに渡したnumClipsと同じ要素数のAnimationClip配列
		void Finalize(nsK2EngineLow::AnimationClip* outClips)
		{
			for (int i = 0; i < static_cast<int>(m_handles.size()); i++)
			{
				// エラー状態のリソースはスキップ
				if (m_handles[i] && m_handles[i]->IsCompleted())
				{
					m_handles[i]->Finalize(outClips[i]);
				}
			}
		}

		void Reset() { m_handles.clear(); }

	private:
		std::vector<std::shared_ptr<TkaResource>> m_handles;
	};

} // namespace nsBeastEngine