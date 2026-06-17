/*!
 * @brief 地形 GBuffer 書き込みシェーダー
 *        スプラットマップ（RGBA）で 4 種のテクスチャをブレンドして GBuffer に出力する。
 *
 * レジスタ対応:
 *   b0 : ModelCb          （エンジン自動設定）
 *   b1 : TerrainCb        （TerrainObject::TerrainCb、地形の半幅・半奥行き）
 *   b2 : PBRParamCb       （InitRenderToGBufferModel が自動設定）
 *   b3 : DitherCb         （InitRenderToGBufferModel が自動設定）
 *   b4 : ModelDitherCb    （InitRenderToGBufferModel が自動設定）
 *   t10: スプラットマップ          （expandShaderResoruceView[0]）
 *   t11: snow BaseColor            （expandShaderResoruceView[1]）
 *   t12: glass BaseColor           （expandShaderResoruceView[2]）
 *   t13: rock BaseColor            （expandShaderResoruceView[3]）
 *   t14: snow BaseColor (fallback) （expandShaderResoruceView[4]）
 *   t15: snow Normal               （expandShaderResoruceView[5]）
 *   t16: snow Roughness            （expandShaderResoruceView[6]）
 */

// -----------------------------------------------------------------------
// 定数バッファ
// -----------------------------------------------------------------------
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   mulColor;
};

// TerrainCb は C++ 側 TerrainObject::TerrainCb と一致させること
cbuffer TerrainCb : register(b1)
{
    float terrainHalfWidth;   // 地形 X 方向の半幅（ワールド単位）
    float terrainHalfDepth;   // 地形 Z 方向の半奥行き（ワールド単位）
    float terrainAlbedoScale; // アルベド明度スケール（1.0=そのまま）
    float terrainPad;
};

cbuffer PBRParamCb : register(b2)
{
    float dirLightScale;
    float ambientScale;
    float metallicOffset;
    float smoothOffset;
};

cbuffer DitherCb : register(b3)
{
    float3 cameraWorldPos;
    float  cylinderRadius;
    float3 targetWorldPos;
    float  depthBias;
    float  ditherStrength;
    float3 ditherPad;
};

cbuffer ModelDitherCb : register(b4)
{
    float  modelDitherAlpha;
    float3 modelDitherPad;
};

// -----------------------------------------------------------------------
// テクスチャ・サンプラー
// -----------------------------------------------------------------------
Texture2D<float4> g_splatmap       : register(t10);
Texture2D<float4> g_terrainTex0    : register(t11);  // snow BaseColor
Texture2D<float4> g_terrainTex1    : register(t12);  // glass BaseColor
Texture2D<float4> g_terrainTex2    : register(t13);  // rock BaseColor
Texture2D<float4> g_terrainTex3    : register(t14);  // snow BaseColor fallback
Texture2D<float4> g_snowNormal     : register(t15);  // snow Normal map
Texture2D<float4> g_snowRoughness  : register(t16);  // snow Roughness map

sampler g_sampler : register(s0);

// -----------------------------------------------------------------------
// 構造体
// -----------------------------------------------------------------------
struct SSkinVSIn
{
    int4   Indices : BLENDINDICES0;
    float4 Weights : BLENDWEIGHT0;
};

struct SVSIn
{
    float4 pos      : POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv       : TEXCOORD0;
    SSkinVSIn skinVert;
    uint instanceID : SV_InstanceID;
};

struct SPSIn
{
    float4 pos      : SV_POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TEXCOORD2;
    float3 biNormal : TEXCOORD3;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

// RenderToGBuffer.fx と同じ出力レイアウト
struct SPSOut
{
    float4 albedo           : SV_Target0;
    float4 normal           : SV_Target1;
    float4 metaricSmoothMap : SV_Target2;
};

// -----------------------------------------------------------------------
// Bayer 4x4 ディザリングパターン（RenderToGBuffer.fx と同じ値）
// -----------------------------------------------------------------------
static const int g_bayerPattern[4][4] =
{
    {  0, 32,  8, 40 },
    { 48, 16, 56, 24 },
    { 12, 44,  4, 36 },
    { 60, 28, 52, 20 },
};

// -----------------------------------------------------------------------
// 頂点シェーダー
// -----------------------------------------------------------------------
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;

    float4 worldPos  = mul(mWorld, vsIn.pos);
    psIn.worldPos    = worldPos.xyz;

    float4 viewPos   = mul(mView, worldPos);
    psIn.pos         = mul(mProj, viewPos);

    float3x3 m3      = (float3x3)mWorld;
    psIn.normal      = normalize(mul(m3, vsIn.normal));
    psIn.tangent     = normalize(mul(m3, vsIn.tangent));
    psIn.biNormal    = normalize(mul(m3, vsIn.biNormal));

    psIn.uv          = vsIn.uv;

    return psIn;
}

// 地形はスキニング不要のため VSMain に委譲
SPSIn VSMainSkin(SVSIn vsIn)
{
    return VSMain(vsIn);
}

// -----------------------------------------------------------------------
// ピクセルシェーダー
// -----------------------------------------------------------------------
SPSOut PSMain(SPSIn psIn)
{
    // ------ モデル単位ディザリング ------
    int2  pixelPos  = (int2)psIn.pos.xy;
    int   bayerIdx  = g_bayerPattern[pixelPos.y % 4][pixelPos.x % 4];
    float threshold = (bayerIdx + 0.5f) / 64.0f;

    if (modelDitherAlpha > threshold)
    {
        discard;
    }

    // ------ スプラットマップ UV（地形全体で 0〜1）------
    // 頂点は 3ds Max Z-up で生成し、エンジンが MakeRotationX(-PI/2) で Y-up に変換する。
    // 変換後: worldPos.z = -(3ds Max Y) なので V は符号反転して計算する。
    float2 splatUV;
    splatUV.x = (psIn.worldPos.x + terrainHalfWidth) / (terrainHalfWidth * 2.0f);
    splatUV.y = (terrainHalfDepth - psIn.worldPos.z) / (terrainHalfDepth * 2.0f);
    splatUV   = saturate(splatUV);

    // ------ スプラットマップサンプル ------
    float4 splat = g_splatmap.Sample(g_sampler, splatUV);

    // 雪ウェイト（R + A チャンネルの合算）
    float snowWeight = saturate(splat.r + splat.a);

    // ------ BaseColor ブレンド ------
    float4 col0 = g_terrainTex0.Sample(g_sampler, psIn.uv);
    float4 col1 = g_terrainTex1.Sample(g_sampler, psIn.uv);
    float4 col2 = g_terrainTex2.Sample(g_sampler, psIn.uv);
    float4 col3 = g_terrainTex3.Sample(g_sampler, psIn.uv);

    float4 albedo = col0 * splat.r
                  + col1 * splat.g
                  + col2 * splat.b
                  + col3 * splat.a;
    albedo.a = 1.0f;

    // ------ 法線マッピング（雪エリアのみ適用）------
    float4 normalSample  = g_snowNormal.Sample(g_sampler, psIn.uv);
    float3 tangentNormal = normalSample.xyz * 2.0f - 1.0f;  // [0,1] → [-1,1]

    // TBN 行列でタンジェント空間 → ワールド空間に変換
    float3x3 tbn        = float3x3(psIn.tangent, psIn.biNormal, psIn.normal);
    float3 mappedNormal = normalize(mul(tangentNormal, tbn));

    // 雪エリアはノーマルマップを使用、それ以外はジオメトリ法線
    float3 finalNormal  = normalize(lerp(psIn.normal, mappedNormal, snowWeight));

    // ------ ラフネス（雪: テクスチャ値、その他: デフォルト）------
    float roughness  = g_snowRoughness.Sample(g_sampler, psIn.uv).r;
    float snowSmooth = 1.0f - roughness;               // Roughness → Smooth 変換
    float baseSmooth = 0.25f;                           // 非雪エリアのデフォルト
    float finalSmooth = lerp(baseSmooth, snowSmooth, snowWeight);

    // ------ GBuffer 出力 ------
    SPSOut psOut;

    psOut.albedo = albedo * mulColor * terrainAlbedoScale;

    // 法線（0〜1 レンジに圧縮）
    psOut.normal = float4(finalNormal * 0.5f + 0.5f, 1.0f);

    // PBR パラメータ
    psOut.metaricSmoothMap = float4(
        clamp(0.0f        + metallicOffset, 0.0f, 1.0f),  // metallic（非金属）
        dirLightScale,
        ambientScale,
        clamp(finalSmooth + smoothOffset,   0.0f, 1.0f)   // smooth（ラフネスから算出）
    );

    return psOut;
}
