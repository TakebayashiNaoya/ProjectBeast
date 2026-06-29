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
		const Vector3 FormationRangeVisualizer::JOIN_COLOR  = { 0.2f, 1.0f, 0.2f };
		const Vector3 FormationRangeVisualizer::LEAVE_COLOR = { 1.0f, 0.2f, 0.2f };


		void FormationRangeVisualizer::Init()
		{
			if (g_graphicsEngine == nullptr) return;

			InitRootSignature();
			InitShader();
			InitPipelineState();
			InitVertexBuffer();
			InitIndexBuffer();
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
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
			psoDesc.NumRenderTargets                   = 3;
			psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R8G8B8A8_UNORM;
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


		void FormationRangeVisualizer::BuildRingVertices(const Vector3& center, float radius, const Vector3& color)
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


		void FormationRangeVisualizer::Update(const Vector3& center, float joinRadius, float leaveRadius)
		{
			if (!m_isInitialized || !m_isVisible) return;

			m_vertexCount = 0;

			// 入隊半径と脱隊半径のリングを構築
			if (joinRadius  > 0.0f) BuildRingVertices(center, joinRadius,  JOIN_COLOR);
			if (leaveRadius > 0.0f) BuildRingVertices(center, leaveRadius, LEAVE_COLOR);
		}


		void FormationRangeVisualizer::Render(RenderContext& rc)
		{
			if (!m_isInitialized || !m_isVisible || m_vertexCount == 0) return;

			m_vertexBuffer.Copy(m_vertices.data());

			// 定数バッファに VP 行列をコピー
			Matrix VP;
			Matrix v = CameraSystem::Get().GetMainCamera().GetViewMatrix();
			Matrix p = CameraSystem::Get().GetMainCamera().GetProjectionMatrix();
			VP.Multiply(v, p);
			m_constantBuffer.CopyToVRAM(&VP);

			// 描画
			rc.SetRootSignature(m_rootSignature);
			rc.SetPipelineState(m_pipelineState);
			rc.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			rc.SetVertexBuffer(m_vertexBuffer);
			rc.SetIndexBuffer(m_indexBuffer);
			rc.SetDescriptorHeap(m_descriptorHeap);
			rc.DrawIndexed(static_cast<UINT>(m_vertexCount));
		}
	}
}
