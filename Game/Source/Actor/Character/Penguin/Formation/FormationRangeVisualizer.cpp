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
		const Vector4 FormationRangeVisualizer::BOUNDARY_COLOR = { 1.0f, 0.60f, 0.15f, 0.35f };  /** 範囲の外周（薄いオレンジの帯） */
		const Vector4 FormationRangeVisualizer::RIPPLE_COLOR   = { 1.0f, 0.55f, 0.10f, 0.85f };  /** 収束する波紋（オレンジ） */
		//const Vector4 FormationRangeVisualizer::SLOT_COLOR    = { 1.0f, 1.0f, 1.0f, 1.0f  };  /** スロット */


		void FormationRangeVisualizer::Init()
		{
			if (g_graphicsEngine == nullptr) return;

			InitRootSignature();
			InitShader();
			InitLinePipelineState();
			InitFillPipelineState();
			InitConstantBuffer();
			InitDescriptorHeap();

			m_joinCircle.InitBand(RANGE_SEGS, BOUNDARY_COLOR);

			for (int i = 0; i < RIPPLE_COUNT; ++i)
			{
				m_rippleCircles[i].InitBand(RANGE_SEGS, RIPPLE_COLOR);
			}

			/** ウルトリング（色は陣形色を毎回セットする） */
			m_ultRing.InitBand(RANGE_SEGS, Vector4(1.0f, 1.0f, 1.0f, 0.6f));

			// TODO: スロットマーカーの初期化。実装時は以下を有効化する。
			//for (int i = 0; i < MAX_SLOT_COUNT; ++i)
			//{
			//	m_slotCircles[i].Init(SLOT_SEGS, SLOT_COLOR, SLOT_COLOR, false);
			//}

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

			/** 波紋リングのフェードのため、縁取りもアルファブレンドを有効にする */
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

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
			psoDesc.pRootSignature                     = m_rootSignature.Get();
			psoDesc.VS                                 = CD3DX12_SHADER_BYTECODE(m_vs.GetCompiledBlob());
			psoDesc.PS                                 = CD3DX12_SHADER_BYTECODE(m_ps.GetCompiledBlob());
			psoDesc.BlendState                         = blendDesc;
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


		void FormationRangeVisualizer::Update(const Vector3& center, float joinRadius, const std::vector<Vector3>& slotPositions)
		{
			if (!m_isInitialized) return;

			if (m_isVisible && joinRadius > 0.0f)
			{
				/** 範囲の外周: 薄いオレンジの帯で「声の届く範囲」を示す */
				m_joinCircle.UpdateBand(center, joinRadius, BOUNDARY_HALF_WIDTH, 1.0f);

				/** 波紋: 外周から中心へ等間隔で収束するグラデーションの帯。
				 *  進行度 u が 0（外周）→ 1（中心付近）へ進み、
				 *  出現直後はフェードイン、そのあと中心へ向かうほど薄くなる */
				m_rippleTimer += g_gameTime->GetFrameDeltaTime();
				for (int i = 0; i < RIPPLE_COUNT; ++i)
				{
					float u = m_rippleTimer / RIPPLE_PERIOD + static_cast<float>(i) / RIPPLE_COUNT;
					u -= floorf(u);

					const float radius = joinRadius * (1.0f - u * (1.0f - RIPPLE_INNER_RATIO));
					const float fadeIn = (std::min)(u / RIPPLE_FADE_IN_END, 1.0f);
					const float alpha  = fadeIn * (1.0f - u);
					m_rippleCircles[i].UpdateBand(center, radius, RIPPLE_HALF_WIDTH, alpha);
				}
			}

			// TODO: スロットマーカーの更新。実装時は以下を有効化する。
			//m_activeSlotCount = static_cast<int>(std::min<size_t>(slotPositions.size(), MAX_SLOT_COUNT));
			//for (int i = 0; i < m_activeSlotCount; ++i)
			//{
			//	m_slotCircles[i].Update(slotPositions[i], SLOT_RADIUS);
			//}
		}


		void FormationRangeVisualizer::UpdateUltRing(
			const Vector3& center, float radius, float alpha, const Vector4& color)
		{
			if (!m_isInitialized) return;
			m_ultRing.SetBandColor(color);
			m_ultRing.UpdateBand(center, radius, ULT_RING_HALF_WIDTH, alpha);
		}


		void FormationRangeVisualizer::Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view)
		{
			if (!m_isInitialized) return;
			if (!m_isVisible && !m_isUltRingVisible) return;

			// 定数バッファに VP 行列をコピー
			Matrix VP;
			VP.Multiply(view.camera->GetViewMatrix(), view.camera->GetProjectionMatrix());
			m_constantBuffer.CopyToVRAM(&VP);

			rc.SetRootSignature(m_rootSignature);
			rc.SetDescriptorHeap(m_descriptorHeap);

			// 外周の帯と波紋の帯を、アルファブレンド有効の塗りつぶし用PSOで描画する
			rc.SetPipelineState(m_fillPipelineState);
			if (m_isVisible)
			{
				m_joinCircle.RenderBand(rc);
				for (int i = 0; i < RIPPLE_COUNT; ++i)
				{
					m_rippleCircles[i].RenderBand(rc);
				}
			}
			if (m_isUltRingVisible)
			{
				m_ultRing.RenderBand(rc);
			}
			// TODO: スロットマーカーの描画。実装時は m_linePipelineState をセットして
			//       以下を有効化する。
			//for (int i = 0; i < m_activeSlotCount; ++i)
			//{
			//	m_slotCircles[i].RenderEdge(rc);
			//}
		}
	}
}
