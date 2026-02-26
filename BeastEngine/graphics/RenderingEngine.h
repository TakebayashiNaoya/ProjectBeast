/**
 * @file RenderingEngine.h
 * @brief RenderingEngineクラスのヘッダー
 * @author 竹林尚哉
 */
#pragma once
#include "MyRenderer.h"
#include "Graphics/Light/SceneLight.h"


namespace nsBeastEngine
{
	class RenderingEngine
	{
	public:
		RenderingEngine();
		~RenderingEngine();

		/**
		 * @brief レンダリングエンジンの初期化\
		 */
		void Init();

		/**
		 * @brief 描画オブジェクトをリストに登録する（予約）
		 */
		void AddRenderObject(IRenderer* renderObject);

		/**
		 * 溜まったリストを一気に描画する
		 */
		void Execute(RenderContext& rc);


	private:
		/** 描画するオブジェクトの予約リスト */
		std::vector<IRenderer*> m_renderObjects;
	};
}