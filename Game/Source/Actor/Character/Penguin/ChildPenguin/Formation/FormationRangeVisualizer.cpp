/**
 * @file FormationRangeVisualizer.cpp
 * @brief 陣形の入隊・脱隊半径を地形追従ラインリングで可視化する
 * @author 竹林
 */
#include "stdafx.h"
#include "FormationRangeVisualizer.h"


namespace app
{
	namespace actor
	{
		const Vector4 FormationRangeVisualizer::JOIN_EDGE_COLOR  = { 0.2f, 1.0f, 0.2f, 1.0f  };  /** 入隊範囲（縁） */
		const Vector4 FormationRangeVisualizer::JOIN_FILL_COLOR  = { 0.2f, 1.0f, 0.2f, 0.25f };  /** 入隊範囲（塗りつぶし） */
		const Vector4 FormationRangeVisualizer::LEAVE_EDGE_COLOR = { 1.0f, 0.2f, 0.2f, 1.0f  };  /** 脱隊範囲（縁） */
		const Vector4 FormationRangeVisualizer::LEAVE_FILL_COLOR = { 1.0f, 0.2f, 0.2f, 0.25f };  /** 脱隊範囲（塗りつぶし） */
		const Vector4 FormationRangeVisualizer::SLOT_COLOR       = { 1.0f, 1.0f, 1.0f, 1.0f  };  /** スロット */


		void FormationRangeVisualizer::Init()
		{
			if (g_graphicsEngine == nullptr) return;

			InitRootSignature();
			InitShader();
			InitLinePipelineState();
			InitFillPipelineState();
			InitConstantBuffer();
			InitDescriptorHeap();

			m_joinCircle.Init(RANGE_SEGS,  JOIN_EDGE_COLOR,  JOIN_FILL_COLOR,  true);
			m_leaveCircle.Init(RANGE_SEGS, LEAVE_EDGE_COLOR, LEAVE_FILL_COLOR, true);

			// スロットマーカーを全て事前確保（Update()内でのGPUバッファ作成を避ける）
			for (int i = 0; i < MAX_SLOT_COUNT; ++i)
			{
				m_slotCircles[i].Init(SLOT_SEGS, SLOT_COLOR, SLOT_COLOR, false);
			}

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


		void FormationRangeVisualizer::InitLinePipelineState()
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
			psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.SampleMask                         = UINT_MAX;
			psoDesc.RasterizerState                    = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthEnable      = TRUE;
			psoDesc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
			psoDesc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			psoDesc.DepthStencilState.StencilEnable    = FALSE;
			psoDesc.InputLayout                        = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			psoDesc.NumRenderTargets                   = 1;
			psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R32G32B32A32_FLOAT;
			psoDesc.DSVFormat                          = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count                   = 1;
			m_linePipelineState.Init(psoDesc);
		}


		void FormationRangeVisualizer::InitFillPipelineState()
		{
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_RENDER_TARGET_BLEND_DESC rtBlend = {};
			rtBlend.BlendEnable                    = TRUE;
			rtBlend.SrcBlend                       = D3D12_BLEND_SRC_ALPHA;
			rtBlend.DestBlend                      = D3D12_BLEND_INV_SRC_ALPHA;
			rtBlend.BlendOp                        = D3D12_BLEND_OP_ADD;
			rtBlend.SrcBlendAlpha                  = D3D12_BLEND_ONE;
			rtBlend.DestBlendAlpha                 = D3D12_BLEND_ZERO;
			rtBlend.BlendOpAlpha                   = D3D12_BLEND_OP_ADD;
			rtBlend.RenderTargetWriteMask          = D3D12_COLOR_WRITE_ENABLE_ALL;

			CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
			blendDesc.RenderTarget[0] = rtBlend;

			CD3DX12_RASTERIZER_DESC rastDesc(D3D12_DEFAULT);
			rastDesc.CullMode = D3D12_CULL_MODE_NONE;

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
			psoDesc.pRootSignature                     = m_rootSignature.Get();
			psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(m_vs.GetCompiledBlob());
			psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(m_ps.GetCompiledBlob());
			psoDesc.BlendState                         = blendDesc;
			psoDesc.SampleMask                         = UINT_MAX;
			psoDesc.RasterizerState                    = rastDesc;
			psoDesc.DepthStencilState.DepthEnable      = TRUE;
			psoDesc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ZERO;
			psoDesc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			psoDesc.DepthStencilState.StencilEnable    = FALSE;
			psoDesc.InputLayout                        = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets                   = 1;
			psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R32G32B32A32_FLOAT;
			psoDesc.DSVFormat                          = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count                   = 1;
			m_fillPipelineState.Init(psoDesc);
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


		void FormationRangeVisualizer::Update(const Vector3& center, float joinRadius, float leaveRadius, const std::vector<Vector3>& slotPositions)
		{
			if (!m_isInitialized) return;

			if (m_isVisible)
			{
				// 入隊・脱隊半径リングを更新（各頂点で地表 Y をサンプリング）
				if (joinRadius  > 0.0f) m_joinCircle.Update(center, joinRadius);
				if (leaveRadius > 0.0f) m_leaveCircle.Update(center, leaveRadius);
			}

			// スロットマーカーの更新（フォロワーの有無に関係なく常に表示）
			m_activeSlotCount = static_cast<int>(std::min<size_t>(slotPositions.size(), MAX_SLOT_COUNT));
			for (int i = 0; i < m_activeSlotCount; ++i)
			{
				m_slotCircles[i].Update(slotPositions[i], SLOT_RADIUS);
			}
		}


		void FormationRangeVisualizer::Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view)
		{
			if (!m_isInitialized) return;
			if (!m_isVisible && m_activeSlotCount == 0) return;

			// 定数バッファに VP 行列をコピー
			Matrix VP;
			VP.Multiply(view.camera->GetViewMatrix(), view.camera->GetProjectionMatrix());
			m_constantBuffer.CopyToVRAM(&VP);

			rc.SetRootSignature(m_rootSignature);
			rc.SetDescriptorHeap(m_descriptorHeap);

			// 半透明の塗りつぶしを先に描画（縁取りが上に重なるよう順序を守る）
			if (m_isVisible)
			{
				rc.SetPipelineState(m_fillPipelineState);
				m_joinCircle.RenderFill(rc);
				m_leaveCircle.RenderFill(rc);
			}

			// 縁取りと白いスロットマーカーを描画
			rc.SetPipelineState(m_linePipelineState);
			if (m_isVisible)
			{
				m_joinCircle.RenderEdge(rc);
				m_leaveCircle.RenderEdge(rc);
			}
			for (int i = 0; i < m_activeSlotCount; ++i)
			{
				m_slotCircles[i].RenderEdge(rc);
			}
		}
	}
}
