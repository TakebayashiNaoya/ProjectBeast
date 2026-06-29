/**
 * @file FormationRangeVisualizer.cpp
 * @brief 陣形の入隊・脱隊半径を地形追従ラインリングで可視化する
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationRangeVisualizer.h"
#include "Source/Nature/Ocean.h"


namespace app
{
	namespace actor
	{
		const Vector4 FormationRangeVisualizer::JOIN_COLOR        = { 0.2f, 1.0f, 0.2f, 1.0f  };
		const Vector4 FormationRangeVisualizer::LEAVE_COLOR       = { 1.0f, 0.2f, 0.2f, 1.0f  };
		const Vector4 FormationRangeVisualizer::JOIN_FILL_COLOR   = { 0.2f, 1.0f, 0.2f, 0.25f };
		const Vector4 FormationRangeVisualizer::LEAVE_FILL_COLOR  = { 1.0f, 0.2f, 0.2f, 0.25f };


		void FormationRangeVisualizer::Init()
		{
			if (g_graphicsEngine == nullptr) return;

			InitRootSignature();
			InitShader();
			InitPipelineState();
			InitFillPipelineState();
			InitVertexBuffer();
			InitIndexBuffer();
			InitFillVertexBuffer();
			InitFillIndexBuffer();
			InitConstantBuffer();
			InitDescriptorHeap();

			m_isInitialized = true;
		}


		void FormationRangeVisualizer::InitRootSignature()
		{
			m_rootSignature.Init(
				D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP,
				D3D12_TEXTURE_ADDRESS_MODE_WRAP
			);
		}


		void FormationRangeVisualizer::InitShader()
		{
			m_vs.LoadVS("Assets/shader/FormationRange.fx", "VSMain");
			m_ps.LoadPS("Assets/shader/FormationRange.fx", "PSMain");
		}


		void FormationRangeVisualizer::InitPipelineState()
		{
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
			psoDesc.pRootSignature                     = m_rootSignature.Get();
			psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(m_vs.GetCompiledBlob());
			psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(m_ps.GetCompiledBlob());
			psoDesc.BlendState						   = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.SampleMask						   = UINT_MAX;
			psoDesc.RasterizerState					   = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthEnable	   = TRUE;
			psoDesc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
			psoDesc.DepthStencilState.DepthFunc		   = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			psoDesc.DepthStencilState.StencilEnable	   = FALSE;
			psoDesc.InputLayout						   = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			psoDesc.NumRenderTargets                   = 1;
			psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R32G32B32A32_FLOAT;
			psoDesc.DSVFormat                          = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count                   = 1;
			m_pipelineState.Init(psoDesc);
		}


		void FormationRangeVisualizer::InitVertexBuffer()
		{
			m_vertexBuffer.Init(sizeof(Vertex) * TOTAL_VERTS, sizeof(Vertex));
		}


		void FormationRangeVisualizer::InitIndexBuffer()
		{
			m_indexBuffer.Init(sizeof(uint16_t) * TOTAL_VERTS, sizeof(uint16_t));

			std::array<uint16_t, TOTAL_VERTS> indices;
			for (int i = 0; i < TOTAL_VERTS; ++i)
			{
				indices[i] = static_cast<uint16_t>(i);
			}
			m_indexBuffer.Copy(indices.data());
		}


		void FormationRangeVisualizer::InitConstantBuffer()
		{
			m_constantBuffer.Init(sizeof(Matrix));
		}


		void FormationRangeVisualizer::InitFillVertexBuffer()
		{
			m_fillVertexBuffer.Init(sizeof(Vertex) * TOTAL_FILL_VERTS, sizeof(Vertex));
		}


		void FormationRangeVisualizer::InitFillIndexBuffer()
		{
			m_fillIndexBuffer.Init(sizeof(uint16_t) * TOTAL_FILL_VERTS, sizeof(uint16_t));

			std::array<uint16_t, TOTAL_FILL_VERTS> indices;
			for (int i = 0; i < TOTAL_FILL_VERTS; ++i)
			{
				indices[i] = static_cast<uint16_t>(i);
			}
			m_fillIndexBuffer.Copy(indices.data());
		}


		void FormationRangeVisualizer::InitFillPipelineState()
		{
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_RENDER_TARGET_BLEND_DESC rtBlend       = {};
			rtBlend.BlendEnable                          = TRUE;
			rtBlend.SrcBlend                             = D3D12_BLEND_SRC_ALPHA;
			rtBlend.DestBlend                            = D3D12_BLEND_INV_SRC_ALPHA;
			rtBlend.BlendOp                              = D3D12_BLEND_OP_ADD;
			rtBlend.SrcBlendAlpha                        = D3D12_BLEND_ONE;
			rtBlend.DestBlendAlpha                       = D3D12_BLEND_ZERO;
			rtBlend.BlendOpAlpha                         = D3D12_BLEND_OP_ADD;
			rtBlend.RenderTargetWriteMask                = D3D12_COLOR_WRITE_ENABLE_ALL;

			CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
			blendDesc.RenderTarget[0]                    = rtBlend;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc    = { 0 };
			psoDesc.pRootSignature                        = m_rootSignature.Get();
			psoDesc.VS                                    = CD3DX12_SHADER_BYTECODE(m_vs.GetCompiledBlob());
			psoDesc.PS                                    = CD3DX12_SHADER_BYTECODE(m_ps.GetCompiledBlob());
			psoDesc.BlendState                            = blendDesc;
			psoDesc.SampleMask                            = UINT_MAX;
			CD3DX12_RASTERIZER_DESC rastDesc(D3D12_DEFAULT);
			rastDesc.CullMode                             = D3D12_CULL_MODE_NONE;
			psoDesc.RasterizerState                       = rastDesc;
			psoDesc.DepthStencilState.DepthEnable         = TRUE;
			psoDesc.DepthStencilState.DepthWriteMask      = D3D12_DEPTH_WRITE_MASK_ZERO;
			psoDesc.DepthStencilState.DepthFunc           = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			psoDesc.DepthStencilState.StencilEnable       = FALSE;
			psoDesc.InputLayout                           = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.PrimitiveTopologyType                 = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets                      = 1;
			psoDesc.RTVFormats[0]                         = DXGI_FORMAT_R32G32B32A32_FLOAT;
			psoDesc.DSVFormat                             = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count                      = 1;
			m_fillPipelineState.Init(psoDesc);
		}


		void FormationRangeVisualizer::InitDescriptorHeap()
		{
			m_descriptorHeap.RegistConstantBuffer(0, m_constantBuffer);
			m_descriptorHeap.Commit();
		}


		float FormationRangeVisualizer::SampleSurfaceY(float x, float z) const
		{
			// レイキャストの始点と終点を設定
			const Vector3 rayStart(x, RAYCAST_TOP,    z);
			const Vector3 rayEnd  (x, RAYCAST_BOTTOM, z);

			// 地形の高さをレイキャストでサンプリング
			nsBeastEngine::nsCollision::RaycastHit hit;
			if (nsBeastEngine::nsCollision::PhysicsWorld::Get().Raycast(rayStart, rayEnd, hit))
			{
				return hit.point.y;
			}

			// 地形がなければ海面（波を含む）でサンプリング
			if (const nature::Ocean* ocean = nature::Ocean::GetInstance())
			{
				return ocean->SampleWaveHeight(x, z);
			}

			return 0.0f;
		}


		void FormationRangeVisualizer::BuildRingVertices(const Vector3& center, float radius, const Vector4& color)
		{
			// 円周上の頂点を計算
			std::array<Vector3, N_SEGMENTS> points;
			const float angleStep = 2.0f * Math::PI / N_SEGMENTS;

			// 円周上の各点の座標を計算し、地表の高さをサンプリングして Y 座標を決定
			for (int i = 0; i < N_SEGMENTS; ++i)
			{
				const float angle = i * angleStep;
				const float x     = center.x + radius * cosf(angle);
				const float z     = center.z + radius * sinf(angle);
				const float y     = SampleSurfaceY(x, z) + HEIGHT_OFFSET;
				points[i]         = Vector3(x, y, z);
			}

			// LINE_LIST: セグメント i は points[i] → points[i+1]（最後は 0 番に折り返す）
			for (int i = 0; i < N_SEGMENTS; ++i)
			{
				const int next              = (i + 1) % N_SEGMENTS;
				m_vertices[m_vertexCount++] = { points[i],    color };
				m_vertices[m_vertexCount++] = { points[next], color };
			}
		}


		void FormationRangeVisualizer::BuildRingFillVertices(const Vector3& center, float radius, const Vector4& color)
		{
			// 円周上の頂点を計算
			std::array<Vector3, N_SEGMENTS> points;
			const float angleStep = 2.0f * Math::PI / N_SEGMENTS;

			for (int i = 0; i < N_SEGMENTS; ++i)
			{
				const float angle = i * angleStep;
				const float x     = center.x + radius * cosf(angle);
				const float z     = center.z + radius * sinf(angle);
				const float y     = SampleSurfaceY(x, z) + HEIGHT_OFFSET;
				points[i]         = Vector3(x, y, z);
			}

			// 中心点（地表の高さでサンプリング）
			const Vector3 centerPt(center.x, SampleSurfaceY(center.x, center.z) + HEIGHT_OFFSET, center.z);

			// TRIANGLE_LIST: セグメント i は 中心 → points[i] → points[i+1]
			for (int i = 0; i < N_SEGMENTS; ++i)
			{
				const int next                    = (i + 1) % N_SEGMENTS;
				m_fillVertices[m_fillVertexCount++] = { centerPt,     color };
				m_fillVertices[m_fillVertexCount++] = { points[i],    color };
				m_fillVertices[m_fillVertexCount++] = { points[next], color };
			}
		}


		void FormationRangeVisualizer::Update(const Vector3& center, float joinRadius, float leaveRadius)
		{
			if (!m_isInitialized || !m_isVisible) return;

			m_vertexCount     = 0;
			m_fillVertexCount = 0;

			// 入隊半径と脱隊半径のリングを構築
			if (joinRadius  > 0.0f) BuildRingVertices(center, joinRadius,  JOIN_COLOR);
			if (leaveRadius > 0.0f) BuildRingVertices(center, leaveRadius, LEAVE_COLOR);

			// 半透明の塗りつぶしリングを構築
			if (joinRadius  > 0.0f) BuildRingFillVertices(center, joinRadius,  JOIN_FILL_COLOR);
			if (leaveRadius > 0.0f) BuildRingFillVertices(center, leaveRadius, LEAVE_FILL_COLOR);
		}


		void FormationRangeVisualizer::Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view)
		{
			if (!m_isInitialized || !m_isVisible || m_vertexCount == 0) return;

			// 定数バッファに VP 行列をコピー
			Matrix VP;
			VP.Multiply(view.camera->GetViewMatrix(), view.camera->GetProjectionMatrix());
			m_constantBuffer.CopyToVRAM(&VP);

			rc.SetRootSignature(m_rootSignature);
			rc.SetDescriptorHeap(m_descriptorHeap);

			// 半透明の塗りつぶしを先に描画（縁取りが上に重なるよう順序を守る）
			if (m_fillVertexCount > 0)
			{
				m_fillVertexBuffer.Copy(m_fillVertices.data());
				rc.SetPipelineState(m_fillPipelineState);
				rc.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				rc.SetVertexBuffer(m_fillVertexBuffer);
				rc.SetIndexBuffer(m_fillIndexBuffer);
				rc.DrawIndexed(static_cast<UINT>(m_fillVertexCount));
			}

			// 縁取りを描画
			m_vertexBuffer.Copy(m_vertices.data());
			rc.SetPipelineState(m_pipelineState);
			rc.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			rc.SetVertexBuffer(m_vertexBuffer);
			rc.SetIndexBuffer(m_indexBuffer);
			rc.DrawIndexed(static_cast<UINT>(m_vertexCount));
		}
	}
}
