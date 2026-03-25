#pragma once

namespace nsBeastEngine
{
	class TksResource : public IResource
	{
		friend class TksLoader;

	public:
		TksResource() = default;
		~TksResource() = default;

		void Finalize()
		{
			if (m_tksFile)
			{
				g_engine->RegistTksFileToBank(m_filePath.c_str(), m_tksFile.get());
				m_tksFile.release();
			}
		}

	private:
		std::string m_filePath;
		std::unique_ptr<nsK2EngineLow::TksFile> m_tksFile;
	};

	class TksLoader : public ResourceLoader<TksResource>
	{
	public:
		bool LoadImpl(TksResource& resource, const std::string& key) override
		{
			resource.m_filePath = key;

			nsK2EngineLow::TksFile* banked = g_engine->GetTksFileFromBank(key.c_str());
			if (banked != nullptr)
			{
				return true;
			}

			auto tksFile = std::make_unique<nsK2EngineLow::TksFile>();
			if (!tksFile->Load(key.c_str()))
			{
				return false;
			}
			resource.m_tksFile = std::move(tksFile);
			return true;
		}
	};

	class TksSkeletonLoader
	{
	public:
		void RequestLoad(ResourceManager& resourceManager, const char* filePath)
		{
			m_handle = resourceManager.Load<TksResource>(filePath);
		}

		bool IsReady() const
		{
			return m_handle && m_handle->IsCompleted();
		}

		void Finalize()
		{
			if (m_handle)
			{
				m_handle->Finalize();
			}
		}

		void Reset()
		{
			m_handle.reset();
		}

	private:
		std::shared_ptr<TksResource> m_handle;
	};
}
