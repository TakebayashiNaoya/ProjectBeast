#include "BeastEnginePreCompile.h"
#include "SDFFontEngine.h"

#include "DirectXTK/Inc/WICTextureLoader.h"

#include <fstream>
#include <cmath>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace nsK2EngineLow;

namespace nsBeastEngine
{
	// -----------------------------------------------------------------------
	// シングルトン
	// -----------------------------------------------------------------------

	SDFFontEngine& GetSDFFontEngine()
	{
		static SDFFontEngine s_instance;
		static bool s_initialized = false;
		if (!s_initialized) {
			s_instance.Init();
			s_initialized = true;
		}
		return s_instance;
	}

	// -----------------------------------------------------------------------
	// デストラクタ
	// -----------------------------------------------------------------------

	SDFFontEngine::~SDFFontEngine()
	{
		if (m_srvHeap) {
			m_srvHeap->Release();
			m_srvHeap = nullptr;
		}
	}

	// -----------------------------------------------------------------------
	// JSON パーサーヘルパー (msdf-atlas-gen 出力専用)
	// -----------------------------------------------------------------------

	// "key": <float> を fromPos 以降で最初に見つけて返す
	static float JsonFloat(const std::string& json, size_t fromPos, const char* key)
	{
		std::string searchKey = std::string("\"") + key + "\"";
		size_t pos = json.find(searchKey, fromPos);
		if (pos == std::string::npos) return 0.0f;
		size_t colonPos = json.find(':', pos + searchKey.size());
		if (colonPos == std::string::npos) return 0.0f;
		float v = 0.0f;
		sscanf_s(json.c_str() + colonPos + 1, " %f", &v);
		return v;
	}

	// "key": <int> を fromPos 以降で最初に見つけて返す
	static int JsonInt(const std::string& json, size_t fromPos, const char* key)
	{
		std::string searchKey = std::string("\"") + key + "\"";
		size_t pos = json.find(searchKey, fromPos);
		if (pos == std::string::npos) return 0;
		size_t colonPos = json.find(':', pos + searchKey.size());
		if (colonPos == std::string::npos) return 0;
		int v = 0;
		sscanf_s(json.c_str() + colonPos + 1, " %d", &v);
		return v;
	}

	// -----------------------------------------------------------------------
	// アトラス・グリフデータの読み込み
	// -----------------------------------------------------------------------

	bool SDFFontEngine::LoadAtlas()
	{
		std::ifstream file(ATLAS_JSON_PATH);
		if (!file.is_open()) {
			OutputDebugStringA("SDFFontEngine: sdf_atlas.json が見つかりません\n");
			return false;
		}
		std::string json(
			(std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>()
		);
		file.close();

		// --- atlas セクション ---
		size_t atlasPos = json.find("\"atlas\"");
		if (atlasPos != std::string::npos) {
			m_emSize = JsonFloat(json, atlasPos, "size");
			m_atlasWidth = (UINT)JsonInt(json, atlasPos, "width");
			m_atlasHeight = (UINT)JsonInt(json, atlasPos, "height");
		}

		// --- metrics セクション ---
		size_t metricsPos = json.find("\"metrics\"");
		if (metricsPos != std::string::npos) {
			m_lineHeight = JsonFloat(json, metricsPos, "lineHeight");
			m_ascender = JsonFloat(json, metricsPos, "ascender");
		}

		// --- glyphs 配列 ---
		size_t glyphsPos = json.find("\"glyphs\"");
		if (glyphsPos == std::string::npos) return false;

		size_t arrayStart = json.find('[', glyphsPos);
		size_t arrayEnd = json.rfind(']', json.find("\"kerning\"", glyphsPos));
		if (arrayStart == std::string::npos) return false;
		if (arrayEnd == std::string::npos) arrayEnd = json.size();

		size_t pos = arrayStart + 1;
		while (pos < arrayEnd)
		{
			size_t objStart = json.find('{', pos);
			if (objStart == std::string::npos || objStart >= arrayEnd) break;

			// 対応する } を探す (深さ追跡)
			size_t depth = 1;
			size_t objEnd = objStart + 1;
			while (objEnd < json.size() && depth > 0) {
				if (json[objEnd] == '{') ++depth;
				else if (json[objEnd] == '}') --depth;
				++objEnd;
			}

			// このグリフオブジェクトを部分文字列に切り出す
			std::string g_json = json.substr(objStart, objEnd - objStart);

			int unicode = JsonInt(g_json, 0, "unicode");
			if (unicode == 0) { pos = objEnd; continue; }

			SDFGlyph g{};
			g.advance = JsonFloat(g_json, 0, "advance");

			size_t planeBoundsPos = g_json.find("\"planeBounds\"");
			if (planeBoundsPos != std::string::npos) {
				g.planeLeft = JsonFloat(g_json, planeBoundsPos, "left");
				g.planeBottom = JsonFloat(g_json, planeBoundsPos, "bottom");
				g.planeRight = JsonFloat(g_json, planeBoundsPos, "right");
				g.planeTop = JsonFloat(g_json, planeBoundsPos, "top");
			}

			size_t atlasBoundsPos = g_json.find("\"atlasBounds\"");
			if (atlasBoundsPos != std::string::npos) {
				g.atlasLeft = JsonFloat(g_json, atlasBoundsPos, "left");
				g.atlasBottom = JsonFloat(g_json, atlasBoundsPos, "bottom");
				g.atlasRight = JsonFloat(g_json, atlasBoundsPos, "right");
				g.atlasTop = JsonFloat(g_json, atlasBoundsPos, "top");
			}

			m_glyphs[(uint32_t)unicode] = g;
			pos = objEnd;
		}

		return true;
	}

	// -----------------------------------------------------------------------
	// 初期化
	// -----------------------------------------------------------------------

	void SDFFontEngine::Init()
	{
		auto* d3dDevice = g_graphicsEngine->GetD3DDevice();

		// --- SDF ピクセルシェーダーをコンパイル ---
		m_psShader.LoadPS(SHADER_PATH, "PSMain");

		// --- テクスチャ用 SRV ディスクリプタヒープ ---
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_srvHeap));

		// --- PNG アトラスをアップロード ---
		ResourceUploadBatch uploadBatch(d3dDevice);
		uploadBatch.Begin();

		HRESULT hr = CreateWICTextureFromFile(
			d3dDevice,
			uploadBatch,
			ATLAS_PNG_PATH,
			m_sdfTexture.GetAddressOf(),
			false   // ミップマップ不要
		);

		if (FAILED(hr) || !m_sdfTexture) {
			OutputDebugStringA("SDFFontEngine: sdf_atlas.png が見つかりません。\n"
				"  msdf-atlas-gen で Game/Assets/font/sdf_atlas.png を生成してください。\n");
			uploadBatch.End(g_graphicsEngine->GetCommandQueue()).wait();
			return;
		}

		// --- SpriteBatch をカスタム SDF シェーダーで初期化 ---
		RenderTargetState rtState;
		rtState.rtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		rtState.numRenderTargets = 1;
		rtState.dsvFormat = DXGI_FORMAT_D32_FLOAT;
		rtState.sampleMask = UINT_MAX;
		rtState.sampleDesc.Count = 1;

		D3D12_BLEND_DESC blendDesc = CommonStates::NonPremultiplied;
		SpriteBatchPipelineStateDescription psoDesc(rtState, &blendDesc);
		psoDesc.customPixelShader = {
			m_psShader.GetCompiledBlob()->GetBufferPointer(),
			m_psShader.GetCompiledBlob()->GetBufferSize()
		};

		D3D12_VIEWPORT viewport{};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(UI_SPACE_WIDTH);
		viewport.Height = static_cast<float>(UI_SPACE_HEIGHT);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_spriteBatch = std::make_unique<SpriteBatch>(
			d3dDevice, uploadBatch, psoDesc, &viewport
		);

		auto uploadFuture = uploadBatch.End(g_graphicsEngine->GetCommandQueue());
		uploadFuture.wait();

		// --- SRV を作成 ---
		D3D12_RESOURCE_DESC texDesc = m_sdfTexture->GetDesc();
		// テクスチャサイズを実リソースから取得 (JSON 値で上書きされる場合もある)
		if (m_atlasWidth == 0) m_atlasWidth = (UINT)texDesc.Width;
		if (m_atlasHeight == 0) m_atlasHeight = (UINT)texDesc.Height;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		d3dDevice->CreateShaderResourceView(
			m_sdfTexture.Get(),
			&srvDesc,
			m_srvHeap->GetCPUDescriptorHandleForHeapStart()
		);
		m_gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();

		// --- グリフデータを読み込む ---
		LoadAtlas();
	}

	// -----------------------------------------------------------------------
	// 描画開始・終了
	// -----------------------------------------------------------------------

	void SDFFontEngine::BeginDraw(RenderContext& rc)
	{
		if (!m_spriteBatch) return;
		auto* commandList = g_graphicsEngine->GetCommandList();
		commandList->SetDescriptorHeaps(1, &m_srvHeap);
		m_spriteBatch->Begin(commandList, SpriteSortMode_Deferred, g_matIdentity);
	}

	void SDFFontEngine::EndDraw(RenderContext& rc)
	{
		if (!m_spriteBatch) return;
		m_spriteBatch->End();
	}

	// -----------------------------------------------------------------------
	// 文字列描画 (内部実装)
	// -----------------------------------------------------------------------

	void SDFFontEngine::DrawString(
		const wchar_t* text,
		const Vector2& screenPos,
		const Vector4& color,
		float           scale,
		Vector2         pivot)
	{
		if (!text || m_glyphs.empty()) return;

		float fontSize = m_emSize * scale;

		// テキスト全体の幅を計算 (ピボット補正用)
		float totalWidth = 0.0f;
		for (const wchar_t* ch = text; *ch != L'\0'; ++ch) {
			auto it = m_glyphs.find((uint32_t)*ch);
			if (it != m_glyphs.end())
				totalWidth += it->second.advance * fontSize;
		}

		// ベースライン位置: pivot.y=0 で ascender が screenPos.y に来るよう補正
		float totalHeight = m_lineHeight * fontSize;
		float cursorX = screenPos.x - totalWidth * pivot.x;
		float baselineY = screenPos.y
			+ m_ascender * fontSize        // ascender 分だけ下に移動
			- totalHeight * pivot.y;

		XMUINT2 texSize = { m_atlasWidth, m_atlasHeight };

		for (const wchar_t* ch = text; *ch != L'\0'; ++ch)
		{
			auto it = m_glyphs.find((uint32_t)*ch);
			if (it == m_glyphs.end()) {
				cursorX += fontSize * 0.5f;  // 未登録文字は半角分進める
				continue;
			}
			const SDFGlyph& g = it->second;

			// アトラスに絵がある文字だけ描画
			if (g.atlasRight > g.atlasLeft && g.atlasTop > g.atlasBottom)
			{
				// atlasBounds は yOrigin:bottom のためY反転して DX テクスチャ座標に変換
				RECT srcRect;
				srcRect.left = (LONG)std::round(g.atlasLeft);
				srcRect.right = (LONG)std::round(g.atlasRight);
				srcRect.top = (LONG)std::round(m_atlasHeight - g.atlasTop);
				srcRect.bottom = (LONG)std::round(m_atlasHeight - g.atlasBottom);

				// planeBounds は Y上が正のため符号反転してスクリーン座標に変換
				RECT dstRect;
				dstRect.left = (LONG)(cursorX + g.planeLeft * fontSize);
				dstRect.right = (LONG)(cursorX + g.planeRight * fontSize);
				dstRect.top = (LONG)(baselineY - g.planeTop * fontSize);
				dstRect.bottom = (LONG)(baselineY - g.planeBottom * fontSize);

				m_spriteBatch->Draw(
					m_gpuHandle,
					texSize,
					dstRect,
					&srcRect,
					XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&color))
				);
			}

			cursorX += g.advance * fontSize;
		}
	}

	// -----------------------------------------------------------------------
	// 公開 Draw (座標変換・影処理を含む)
	// -----------------------------------------------------------------------

	void SDFFontEngine::Draw(
		const wchar_t* text,
		const Vector2& position,
		const Vector4& color,
		float           /*rotation*/,   // TODO: 将来実装
		float           scale,
		Vector2         pivot)
	{
		// Font.cpp と同じ座標変換: 中心原点 → スクリーン座標
		Vector2 screenPos;
		screenPos.x = position.x + UI_SPACE_WIDTH * 0.5f;
		screenPos.y = -position.y + UI_SPACE_HEIGHT * 0.5f;

		pivot.y = 1.0f - pivot.y;  // Font.cpp に合わせて Y ピボットを反転

		if (m_isDrawShadow)
		{
			const Vector2 offsets[] = {
				{  m_shadowOffset,  0.0f            },
				{ -m_shadowOffset,  0.0f            },
				{  0.0f,            m_shadowOffset  },
				{  0.0f,           -m_shadowOffset  },
				{  m_shadowOffset,  m_shadowOffset  },
				{  m_shadowOffset, -m_shadowOffset  },
				{ -m_shadowOffset,  m_shadowOffset  },
				{ -m_shadowOffset, -m_shadowOffset  },
			};
			for (const auto& offset : offsets) {
				Vector2 sPos = { screenPos.x + offset.x, screenPos.y + offset.y };
				DrawString(text, sPos, m_shadowColor, scale, pivot);
			}
		}

		DrawString(text, screenPos, color, scale, pivot);
	}

} // namespace nsBeastEngine
