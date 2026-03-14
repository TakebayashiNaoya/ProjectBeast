#pragma once

namespace nsBeastEngine
{
	class AnimationResource : public IResource
	{
		friend class AnimationLoader;

	public:
		AnimationResource() = default;
		~AnimationResource() = default;

	public:
		/** アニメーションクリップ本体を取得 */
		Animation* GetAnimation() { return m_animation; }

		void SetSkeleton(Skeleton* skeleton) { m_skeleton = skeleton; }

		void SetAnimationClips(AnimationClip* animationClips, int numAnimationClips)
		{
			m_animationClips = animationClips;
			m_numAnimationClips = numAnimationClips;
		}

	private:
		Animation* m_animation = nullptr;
		Skeleton* m_skeleton = nullptr;
		AnimationClip* m_animationClips = nullptr;
		int m_numAnimationClips = 0;
	};

	class AnimationLoader : public ResourceLoader<AnimationResource>
	{
	public:
		AnimationLoader() = default;
		~AnimationLoader() = default;

		bool LoadImpl(AnimationResource& resource, const std::string& key) override;
	};
}