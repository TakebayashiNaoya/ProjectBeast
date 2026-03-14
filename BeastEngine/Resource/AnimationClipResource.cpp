#include "BeastEnginePreCompile.h"
#include "AnimationClipResource.h"

namespace nsBeastEngine
{
	bool AnimationLoader::LoadImpl(AnimationResource& resource, const std::string& key)
	{
		resource.m_animation = new Animation;
		resource.m_animation->Init(*resource.m_skeleton, resource.m_animationClips, resource.m_numAnimationClips);

		if (resource.m_animation->IsInited()) {
			return true;
		}
		else {
			return true;
		}
	}
}