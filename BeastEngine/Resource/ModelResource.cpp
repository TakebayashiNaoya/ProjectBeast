#include "BeastEnginePreCompile.h"
#include "ModelResource.h"


namespace nsBeastEngine
{
	bool ModelLoader::LoadImpl(ModelResource& resource, const std::string& key)
	{
		resource.m_model = new Model;
		resource.m_model->Init(resource.m_initData);

		if (resource.m_model->IsInited()) {
			return true;
		}
		else {
			return false;
		}
	}
}