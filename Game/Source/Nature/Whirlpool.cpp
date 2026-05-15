/**
 * @file Whirlpool.cpp
 * @brief 渦潮クラス
 * @author 藤谷、竹林
 */
#include "stdafx.h"
#include "Whirlpool.h"
#include "Ocean.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinStateMachine.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Effect/EffectManager.h"
#include "graphics/effect/BeastEffectEmitter.h"


namespace app
{
	namespace nature
	{
		namespace
		{
			/** 渦潮シェーダーのパス */
			const char* WHIRLPOOL_SHADER_PATH = "Assets/shader/Whirlpool.fx";
			/** アルベドマップのパス */
			const wchar_t* WHIRLPOOL_ALBEDO_PATH = L"Assets/modelData/stage/Whirlpool/whirlpool.DDS";

			/** 渦潮の最小スケール */
			const Vector3 MIN_SCALE = Vector3(0.0f, 1.0f, 0.0f);

			constexpr float WHIRLPOOL_Y_OFFSET = 4.0f;	/** ワールドの海面から渦潮メッシュの頂点が浮いている高さ（ワールド単位） */

			/**
			 * @brief パラメーターを取得するヘルパー関数
			 * @return パラメーターポインタ（取得失敗時はnullptr）
			 */
			const MasterWhirlpoolParameter* GetParam()
			{
				return core::ParameterManager::Get()->GetParameter<MasterWhirlpoolParameter>();
			}

			/** エフェクトの基準スケール（渦潮が最大サイズのときのスケール） */
			const Vector3 EFFECT_SCALE = Vector3(1.0f, 1.0f, 1.0f);
		}


		void Whirlpool::Start()
		{
			m_whirlpoolPowerSystem = std::make_unique<WhirlpoolPowerSytem>(this);

			// 円形グリッドメッシュを生成する
			CreateCircleMesh();

			// シェーダーをロードする
			InitShaders();

			// テクスチャをロードする
			m_albedoMap.InitFromDDSFile(WHIRLPOOL_ALBEDO_PATH);

			// ルートシグネチャを構築する
			InitRootSignature();

			// パイプラインステートを構築する
			InitPipelineState();

			// 定数バッファを初期化する
			m_commonConstantBuffer.Init(sizeof(SCommonConstantBuffer), nullptr);
			m_whirlpoolConstantBuffer.Init(sizeof(SWhirlpoolConstantBuffer), nullptr);

			// ディスクリプタヒープを構築する
			InitDescriptorHeap();

			// パラメーターからスケールの最大値を計算する
			// メッシュ半径にスケールを掛けたワールド半径が渦潮の影響範囲と一致するように設定する
			const MasterWhirlpoolParameter* param = GetParam();
			m_maxScaleXZ = (param != nullptr)
				? param->whirlpoolRadius / MESH_RADIUS
				: 2.0f;
			const Vector3 maxScale = Vector3(m_maxScaleXZ, 1.0f, m_maxScaleXZ);

			// スケールカーブを初期化する
			m_scaleBigger.Initialize(
				MIN_SCALE, maxScale,
				(param != nullptr) ? param->scaleChangeTime : 2.5f,
				util::EasingType::EaseInOut,
				util::LoopMode::Once
			);
			m_scaleSmaller.Initialize(
				maxScale, MIN_SCALE,
				(param != nullptr) ? param->scaleChangeTime : 2.5f,
				util::EasingType::EaseInOut,
				util::LoopMode::Once
			);

			m_state = EnWhirlpoolState::Bigger;
			m_scaleBigger.Play();

			// 引き寄せシステムを開始する
			m_whirlpoolPowerSystem->StartWrapper();

			// Bigger開始と同時にエフェクトを再生する
			PlayWhirlpoolEffect();
		}


		void Whirlpool::Update()
		{
			StateMachine();
			// エフェクトのスケールを渦潮のスケールに合わせて更新する
			UpdateWhirlpoolEffectScale();
			m_whirlpoolPowerSystem->UpdateWrapper();
		}


		void Whirlpool::Render(RenderContext& rc)
		{
			if (m_state == EnWhirlpoolState::None) return;

			// 毎フレーム頂点YをOceanの波面に合わせて更新する
			UpdateVertexHeights();

			// ワールド行列を計算する（スケール × 回転 × 平行移動）
			Matrix mScale;
			mScale.MakeScaling(m_transform.m_scale);
			Matrix mRot;
			mRot.MakeRotationFromQuaternion(m_transform.m_rotation);
			Matrix mTrans;
			mTrans.MakeTranslation(m_transform.m_position);
			Matrix mWorld = mScale * mRot * mTrans;

			// 共通定数バッファを更新する（b0）
			SCommonConstantBuffer commonCb;
			commonCb.mWorld = mWorld;
			commonCb.mView = g_camera3D->GetViewMatrix();
			commonCb.mProj = g_camera3D->GetProjectionMatrix();
			commonCb.mulColor = Vector4::One;
			m_commonConstantBuffer.CopyToVRAM(commonCb);

			// 渦潮定数バッファを更新する（b1）
			SWhirlpoolConstantBuffer whirlpoolCb;
			whirlpoolCb.uvRotation = m_uvRotation;
			whirlpoolCb.padding[0] = 0.0f;
			whirlpoolCb.padding[1] = 0.0f;
			whirlpoolCb.padding[2] = 0.0f;
			m_whirlpoolConstantBuffer.CopyToVRAM(whirlpoolCb);

			// 描画コマンドを発行する
			rc.SetRootSignature(m_rootSignature);
			rc.SetPipelineState(m_pipelineState);
			rc.SetDescriptorHeap(m_descriptorHeap);
			rc.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			rc.SetVertexBuffer(m_vertexBuffer);
			rc.SetIndexBuffer(m_indexBuffer);
			rc.DrawIndexedInstance(m_indexCount, 1);

			m_whirlpoolPowerSystem->RenderWrapper(rc);
		}


		Whirlpool::Whirlpool()
			: m_state(EnWhirlpoolState::Bigger)
			, m_timer(0.0f)
			, m_uvRotation(0.0f)
			, m_index(0)
			, m_indexCount(0)
			, m_maxScaleXZ(2.0f)
			, m_effectHandle(INVALID_EFFECT_HANDLE)
			, m_vs(nullptr)
			, m_ps(nullptr)
		{}


		void Whirlpool::StateMachine()
		{
			const float deltaTime = g_gameTime->GetFrameDeltaTime();
			const MasterWhirlpoolParameter* param = GetParam();

			// UV回転を毎フレーム加算する（ピクセルシェーダーでテクスチャが回って見える）
			const float uvRotationSpeed = (param != nullptr) ? param->uvRotationSpeed : 1.5f;
			m_uvRotation += uvRotationSpeed * deltaTime;

			switch (m_state)
			{
			case EnWhirlpoolState::Bigger:
			{
				m_scaleBigger.Update(deltaTime);
				m_transform.m_scale = m_scaleBigger.GetCurrentValue();

				if (!m_scaleBigger.IsPlaying())
				{
					m_state = EnWhirlpoolState::Stay;
				}

				break;
			}
			case EnWhirlpoolState::Stay:
			{
				m_timer += deltaTime;

				const float stayTime = (param != nullptr) ? param->stayTime : 10.0f;
				if (m_timer >= stayTime)
				{
					m_timer = 0.0f;
					m_state = EnWhirlpoolState::Smaller;
					m_scaleSmaller.Play();
				}

				break;
			}
			case EnWhirlpoolState::Smaller:
			{
				m_scaleSmaller.Update(deltaTime);
				m_transform.m_scale = m_scaleSmaller.GetCurrentValue();

				if (!m_scaleSmaller.IsPlaying())
				{
					// Smaller完了時にエフェクトを停止する
					StopWhirlpoolEffect();
					m_state = EnWhirlpoolState::None;
				}

				break;
			}
			case EnWhirlpoolState::None:
			{
				break;
			}
			default:
				break;
			}
		}


		void Whirlpool::PlayWhirlpoolEffect()
		{
			m_effectHandle = EffectManager::Get().PlayEffect(
				EnEffectKind::Whirlpool,
				m_transform.m_position,
				Quaternion::Identity,
				EFFECT_SCALE
			);
		}


		void Whirlpool::StopWhirlpoolEffect()
		{
			if (m_effectHandle == INVALID_EFFECT_HANDLE) return;

			EffectManager::Get().StopEffect(m_effectHandle);
			m_effectHandle = INVALID_EFFECT_HANDLE;
		}


		void Whirlpool::UpdateWhirlpoolEffectScale()
		{
			if (m_effectHandle == INVALID_EFFECT_HANDLE) return;

			auto* effect = EffectManager::Get().FindEffect(m_effectHandle);
			if (effect == nullptr)
			{
				// エフェクトが自己削除済みの場合はハンドルを無効化する
				m_effectHandle = INVALID_EFFECT_HANDLE;
				return;
			}

			// 渦潮のXZスケール比率（0.0〜1.0）をエフェクトスケールに反映する
			const float ratio = (m_maxScaleXZ > 0.0f) ? (m_transform.m_scale.x / m_maxScaleXZ) : 1.0f;
			effect->SetScale(EFFECT_SCALE * ratio);
		}


		void Whirlpool::CreateCircleMesh()
		{
			const int   numRings = NUM_RINGS;
			const int   numSegments = NUM_SEGMENTS;
			const float radius = MESH_RADIUS;

			// 頂点数：中心1点 + リングごとの頂点
			const int numVerts = 1 + numRings * numSegments;
			m_vertices.resize(numVerts);

			// 中心頂点（インデックス0）
			{
				WhirlpoolVertex& v = m_vertices[0];
				v.pos = Vector3(0.0f, 0.0f, 0.0f);
				v.normal = Vector3(0.0f, 1.0f, 0.0f);
				v.tangent = Vector3(1.0f, 0.0f, 0.0f);
				v.biNormal = Vector3(0.0f, 0.0f, 1.0f);
				v.uv = Vector2(0.5f, 0.5f);
				v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0;
				v.weights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			}

			// リングごとの頂点を生成する
			for (int ring = 0; ring < numRings; ++ring)
			{
				// 0.0（中心）～1.0（外周）の割合
				const float t = static_cast<float>(ring + 1) / static_cast<float>(numRings);
				const float ringRadius = radius * t;

				for (int seg = 0; seg < numSegments; ++seg)
				{
					const float angle = (2.0f * DirectX::XM_PI * static_cast<float>(seg))
						/ static_cast<float>(numSegments);

					const int vertIdx = 1 + ring * numSegments + seg;
					WhirlpoolVertex& v = m_vertices[vertIdx];

					v.pos = Vector3(
						ringRadius * cosf(angle),
						0.0f,
						ringRadius * sinf(angle)
					);
					v.normal = Vector3(0.0f, 1.0f, 0.0f);
					v.tangent = Vector3(1.0f, 0.0f, 0.0f);
					v.biNormal = Vector3(0.0f, 0.0f, 1.0f);

					// UV：円形テクスチャに合わせて0.0～1.0にマッピングする
					v.uv = Vector2(
						0.5f + 0.5f * t * cosf(angle),
						0.5f + 0.5f * t * sinf(angle)
					);

					v.indices[0] = v.indices[1] = v.indices[2] = v.indices[3] = 0;
					v.weights = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
				}
			}

			// 頂点バッファを初期化する
			m_vertexBuffer.Init(
				static_cast<int>(sizeof(WhirlpoolVertex) * m_vertices.size()),
				sizeof(WhirlpoolVertex)
			);
			m_vertexBuffer.Copy(m_vertices.data());

			// インデックスを生成する
			std::vector<uint32_t> indices;
			indices.reserve(numSegments * 3 + (numRings - 1) * numSegments * 6);

			// 中心点と最初のリングの三角形を生成する
			for (int seg = 0; seg < numSegments; ++seg)
			{
				const int curr = 1 + seg;
				const int next = 1 + (seg + 1) % numSegments;
				indices.push_back(0);
				indices.push_back(curr);
				indices.push_back(next);
			}

			// 隣接リング間の四角形を三角形2枚に分割して生成する
			for (int ring = 0; ring < numRings - 1; ++ring)
			{
				for (int seg = 0; seg < numSegments; ++seg)
				{
					const int curr = 1 + ring * numSegments + seg;
					const int next = 1 + ring * numSegments + (seg + 1) % numSegments;
					const int currOuter = 1 + (ring + 1) * numSegments + seg;
					const int nextOuter = 1 + (ring + 1) * numSegments + (seg + 1) % numSegments;

					// 三角形①
					indices.push_back(curr);
					indices.push_back(currOuter);
					indices.push_back(next);

					// 三角形②
					indices.push_back(next);
					indices.push_back(currOuter);
					indices.push_back(nextOuter);
				}
			}

			// インデックスバッファを初期化する
			m_indexCount = static_cast<int>(indices.size());
			m_indexBuffer.Init(
				static_cast<int>(sizeof(uint32_t) * indices.size()),
				sizeof(uint32_t)
			);
			m_indexBuffer.Copy(indices.data());
		}


		void Whirlpool::InitShaders()
		{
			// 頂点シェーダーをロードする
			m_vs = g_engine->GetShaderFromBank(WHIRLPOOL_SHADER_PATH, "VSMain");
			if (m_vs == nullptr)
			{
				m_vs = new Shader;
				m_vs->LoadVS(WHIRLPOOL_SHADER_PATH, "VSMain");
				g_engine->RegistShaderToBank(WHIRLPOOL_SHADER_PATH, "VSMain", m_vs);
			}

			// ピクセルシェーダーをロードする
			m_ps = g_engine->GetShaderFromBank(WHIRLPOOL_SHADER_PATH, "PSMain");
			if (m_ps == nullptr)
			{
				m_ps = new Shader;
				m_ps->LoadPS(WHIRLPOOL_SHADER_PATH, "PSMain");
				g_engine->RegistShaderToBank(WHIRLPOOL_SHADER_PATH, "PSMain", m_ps);
			}
		}


		void Whirlpool::InitRootSignature()
		{
			// サンプラーを設定する
			D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			samplerDesc.MipLODBias = 0;
			samplerDesc.MaxAnisotropy = 0;
			samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			samplerDesc.ShaderRegister = 0;
			samplerDesc.RegisterSpace = 0;
			samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			m_rootSignature.Init(
				&samplerDesc,
				1,	// サンプラー数
				2,	// CBV数（b0, b1）
				1,	// SRV数（t0）
				1	// UAV=0だと内部でシリアライズ失敗するので最低1にする
			);
		}


		void Whirlpool::InitPipelineState()
		{
			// OceanMeshと同じ頂点レイアウトを使用する
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT,  0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			// カラーバッファフォーマットをOceanに合わせる
			std::array<DXGI_FORMAT, MAX_RENDERING_TARGET> colorBufferFormat = {
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
				DXGI_FORMAT_UNKNOWN,
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
			psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			psoDesc.pRootSignature = m_rootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs->GetCompiledBlob());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps->GetCompiledBlob());

			// アルファブレンドを有効にする（テクスチャ円形マスク用）
			psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
			psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			psoDesc.SampleMask = UINT_MAX;
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = colorBufferFormat[0];
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			psoDesc.SampleDesc.Count = 1;

			m_pipelineState.Init(psoDesc);
		}


		void Whirlpool::InitDescriptorHeap()
		{
			m_descriptorHeap.RegistConstantBuffer(0, m_commonConstantBuffer);
			m_descriptorHeap.RegistConstantBuffer(1, m_whirlpoolConstantBuffer);
			m_descriptorHeap.RegistShaderResource(0, m_albedoMap);
			m_descriptorHeap.Commit();
		}


		void Whirlpool::UpdateVertexHeights()
		{
			const Ocean* ocean = Ocean::GetInstance();
			if (ocean == nullptr) return;

			for (auto& v : m_vertices)
			{
				// 頂点のワールド座標を算出する（スケールと平行移動のみ。回転は常にIdentityなので省略）
				const float worldX = v.pos.x * m_transform.m_scale.x + m_transform.m_position.x;
				const float worldZ = v.pos.z * m_transform.m_scale.z + m_transform.m_position.z;

				// Oceanの波面高さを取得してY座標に反映する
				v.pos.y = ocean->SampleWaveHeight(worldX, worldZ) + WHIRLPOOL_Y_OFFSET;
			}

			m_vertexBuffer.Copy(m_vertices.data());
		}
	}
}