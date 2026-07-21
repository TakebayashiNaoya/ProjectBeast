// ---------------------------------------------------------
// DecalTerrainHeight.fx
// 板ポリゴン1枚のまま、地形のハイトマップを直接参照して
// 凹凸に馴染ませる（深度バッファは使わない）
// ---------------------------------------------------------

cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   mulColor;
};

// 地形の凹凸判定用パラメータ（Decal.h の cbTerrainHeight と一致させること）
cbuffer TerrainHeightCb : register(b1)
{
    float g_halfWidth;
    float g_halfDepth;
    float g_heightScale;
    float g_yOffset;
};

// フェードアウト用アルファ（Decal.h の cbDecal と一致させること）
cbuffer DecalCb : register(b2)
{
    float g_alpha;
    float3 g_padding;
};

Texture2D g_texture   : register(t10); // 足跡テクスチャ
Texture2D g_heightmap : register(t11); // 地形のハイトマップ（R16_UNORM）
SamplerState g_sampler : register(s0);

struct VS_INPUT
{
    float4 pos      : POSITION;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv       : TEXCOORD0;
    uint4  indices  : BLENDINDICES0;
    float4 weights  : BLENDWEIGHT0;
};

struct PS_INPUT
{
    float4 pos      : SV_POSITION;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1; // 凹凸判定に必要なワールド座標
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(mWorld, input.pos);
    float4 viewPos  = mul(mView, worldPos);
    output.pos      = mul(mProj, viewPos);
    output.uv       = input.uv;
    output.worldPos = worldPos.xyz;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_Target0
{
    float4 color = g_texture.Sample(g_sampler, input.uv);
    color.rgb *= mulColor.rgb;
    if (color.a < 0.05f) discard;

    // ---- ここから凹凸判定（TerrainObject::GetHeightAt と同じ変換式） ----
    float2 hmUV;
    hmUV.x = (input.worldPos.x + g_halfWidth) / (g_halfWidth * 2.0f);
    hmUV.y = (g_halfDepth - input.worldPos.z) / (g_halfDepth * 2.0f);

    if (hmUV.x < 0.0f || hmUV.x > 1.0f || hmUV.y < 0.0f || hmUV.y > 1.0f)
    {
        discard; // 地形範囲外
    }

    float rawHeight = g_heightmap.Sample(g_sampler, hmUV).r;
    float terrainHeight = rawHeight * g_heightScale + g_yOffset;

    // 板の高さ(決め打ち)と実際の地形の高さの差
    float heightDiff = abs(input.worldPos.y - terrainHeight);

    // 差が大きい場所（大きな段差の下など）はなだらかに消して、
    // 板が地面から浮いたり埋まったりして見えるのを防ぐ
    const float FADE_START = 15.0f;
    const float FADE_END   = 30.0f;
    float heightFade = 1.0f - smoothstep(FADE_START, FADE_END, heightDiff);
    if (heightFade <= 0.0f) discard;
    color.a *= heightFade;
    // ---- 凹凸判定ここまで ----

    color.a *= g_alpha;
    return color;
}