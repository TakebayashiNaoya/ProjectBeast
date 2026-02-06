#pragma once
/**
 * スキンモデルレンダー
 */
namespace nsBeastEngine
{
	class ModelRender : public IRenderer
	{
	public:
		ModelRender();
		~ModelRender();

		void Init(
			const char* filePath,
			AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnModelUpAxis enModelUpAxis = enModelUpAxisZ,
			bool isShadowReciever = true,
			int maxInstance = 1,
			bool isFrontCullingOnDrawShadowMap = false
		);


		void Update();

		void Draw(RenderContext& rc);


	private:
		/**
		 * @brief スケルトンの初期化
		 * @param filePath モデルファイルのパス
		 */
		void InitSkeleton(const char* filePath);

		/**
		 * @brief アニメーションの初期化
		 * @param animationClips アニメーションクリップの配列
		 * @param numAnimationClips アニメーションクリップの数
		 * @param enModelUpAxis モデルの上方向の軸
		 */
		void InitAnimation(AnimationClip* animationClips, int numAnimationClips, EnModelUpAxis enModelUpAxis);


	private:
		/** スケルトン */
		Skeleton m_skeleton;

		/** アニメーションクリップ */
		AnimationClip* m_animationClips;
		/** アニメーションクリップの数 */
		int m_numAnimationClips;
		/** アニメーション */
		Animation m_animation;
	};
}