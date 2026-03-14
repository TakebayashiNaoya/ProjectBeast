#pragma once


namespace nsBeastEngine
{
	class ModelResource : public IResource
	{
		friend class ModelLoader;

	public:
		ModelResource() = default;
		~ModelResource() = default;


	public:
		Model* GetModel() { return m_model; }
		void  SetInitData(ModelInitData& data) { m_initData = data; }


	private:
		Model* m_model = nullptr;	/** モデル */
		ModelInitData m_initData;	/** 初期化データ */
	};


	class ModelLoader : public ResourceLoader<ModelResource>
	{
	public:
		ModelLoader() = default;
		~ModelLoader() = default;

		bool LoadImpl(ModelResource& resource, const std::string& key) override;
	};
}