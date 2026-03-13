/**
 * @file DebugWireframe.h
 * @brief デバッグワイヤーフレーム
 * @author 竹林尚哉
 */
#pragma once
#include "LinearMath/btIDebugDraw.h"
#include "graphics/Shader.h"


namespace nsBeastEngine
{
	namespace nsCollision
	{
		class DebugWireframe : public btIDebugDraw, public Noncopyable
		{
		public:
			DebugWireframe() = default;
			~DebugWireframe() = default;
			/**
			 * @brief 初期化
			 */
			void Init();
			/**
			 * @brief drawLine()を呼ぶ前に行う処理
			 */
			void Begin()
			{
				m_vertexList.clear();
			}
			/**
			 * @brief drawLine()を呼んだ後に行う処理
			 * @param rc レンダラーコンテキスト
			 */
			void End(RenderContext& rc);

			/**
			 * @brief 線ごとに一回ずつ呼ばれる
			 * @param from 1つ目の頂点の座標
			 * @param to 2つ目の頂点の座標
			 * @param color 色
			 */
			void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

			/**
			 * @brief デバッグモードの設定
			 * @param debugMode デバッグモード
			 */
			void setDebugMode(int debugMode) override {};

			/**
			 * @brief デバッグモードの取得
			 * @return デバッグモード
			 */
			int getDebugMode() const override
			{
				return true;
			};

			/** 何もしなくても問題なし{}　必要であれば定義 */
			void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override {};
			void reportErrorWarning(const char* warningString) override {};
			void draw3dText(const btVector3& location, const char* textString) override {};


		private:
			/**
			 * @brief ルートシグネチャの初期化
			 */
			void InitRootSignature();

			/**
			 * @brief シェーダーの初期化
			 */
			void InitSharder();

			/**
			 * @brief パイプラインステートの初期化
			 */
			void InitPipelineState();

			/**
			 * @brief 頂点バッファの初期化
			 */
			void InitVertexBuffer();

			/**
			 * @brief インデックスバッファの初期化
			 */
			void InitIndexBuffer();

			/**
			 * @brief 定数バッファの初期化
			 */
			void InitConstantBuffer();

			/**
			 * @brief ディスクリプタヒープの初期化
			 */
			void InitDescriptorHeap();

			/**
			 * @brief 頂点バッファの更新
			 */
			void VertexBufferUpdate(const btVector3& from, const btVector3& to, const btVector3& color);

			/**
			 * @brief 定数バッファの更新
			 */
			void ConstantBufferUpdate();

			/**
			 * @brief レンダーコンテキストの更新
			 */
			void RenderContextUpdate(RenderContext& rc);


		private:
			/**
			 * @brief 頂点構造体
			 */
			struct Vertex
			{
				Vector3 pos;
				Vector3 color;
			};

			/** 描画する頂点のリスト */
			std::vector<Vertex>	m_vertexList;
			/** 定数バッファ */
			ConstantBuffer		m_constantBuffer;
			/** 頂点バッファ */
			VertexBuffer		m_vertexBuffer;
			/** インデックスバッファ */
			IndexBuffer			m_indexBuffer;
			/** ルートシグネチャ */
			RootSignature		m_rootSignature;
			/** 頂点シェーダー */
			Shader				m_Vshader;
			/** ピクセルシェーダー */
			Shader				m_Pshader;
			/** パイプラインステート */
			PipelineState		m_pipelineState;
			/** ディスクリプタヒープ */
			DescriptorHeap		m_descriptorHeap;
			/** 頂点の最大数 */
			static const int	MAX_VERTEX = 10000000;
		};
	}
}

