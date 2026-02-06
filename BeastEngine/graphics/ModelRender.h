/**
 * @file ModelRender.h
 * @brief モデルレンダーのヘッダファイル
 * @author 竹林尚哉
 */
#pragma once
#include "graphics/ComputeAnimationVertexBuffer.h"
#include "graphics/IRenderer.h"  


namespace nsBeastEngine
{
	/**
	 * @brief モデルレンダー
	 */
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
		 * @param filePath	モデルファイルのパス
		 */
		void InitSkeleton(const char* filePath);

		/**
		 * @brief アニメーションの初期化
		 * @param animationClips	アニメーションクリップの配列
		 * @param numAnimationClips アニメーションクリップの数
		 * @param enModelUpAxis		モデルの上方向の軸
		 */
		void InitAnimation(AnimationClip* animationClips, int numAnimationClips, EnModelUpAxis enModelUpAxis);

		/**
		 * @brief アニメーション頂点バッファの計算処理の初期化
		 * @param tkmFilePath	tkmファイルのファイルパス
		 * @param enModelUpAxis モデルの上方向の軸
		 */
		void InitComputeAnimatoinVertexBuffer(const char* tkmFilePath, EnModelUpAxis enModelUpAxis);


	private:
		/** スケルトン */
		Skeleton m_skeleton;

		/** アニメーションクリップ */
		AnimationClip* m_animationClips;
		/** アニメーションクリップの数 */
		int m_numAnimationClips;
		/** アニメーション */
		Animation m_animation;

		/** インスタンシング描画が有効かどうか */
		bool m_isEnableInstancingDraw;
		/** ワールド行列の配列のストラクチャードバッファ */
		StructuredBuffer m_worldMatrixArraySB;
		/** アニメーション済み頂点バッファの計算処理 */
		ComputeAnimationVertexBuffer m_computeAnimationVertexBuffer;
		/** 最大インスタンス数 */
		int	m_maxInstance;
	};
}