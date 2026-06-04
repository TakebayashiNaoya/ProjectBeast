/**
 * @file INatureObject.h
 * @brief 自然オブジェクトの基底インターフェース
 * @author 竹林
 */
#pragma once
#include "Geometry/Frustum.h"


namespace nsBeastEngine
{
	/**
	 * @brief 自然オブジェクトの基底インターフェース
	 * @details 海・渦潮など、RenderingEngine::Execute()内で描画される
	 *          自然オブジェクトが実装するインターフェース。
	 *          Game側でこのインターフェースを継承することで、
	 *          BeastEngine側を変更せずに描画対象を追加できる。
	 */
	class INatureObject : public Noncopyable
	{
	public:
		virtual ~INatureObject() = default;

		/**
		 * @brief 描画処理
		 * @details RenderingEngine::Execute()内から呼ばれる。
		 *          この時点でm_mainRenderTargetがセットされているため、
		 *          描画コマンドを直接発行できる。
		 * @param rc レンダリングコンテキスト
		 */
		virtual void Render(RenderContext& rc, const Frustum& frustum) = 0;
	};
}