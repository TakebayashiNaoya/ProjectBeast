// ---------------------------------------------------------
// Decal.fx
// ---------------------------------------------------------

// ★修正: エンジンの標準仕様に合わせてb0バッファを統合
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;    // ワールド行列
    float4x4 mView;     // ビュー行列
    float4x4 mProj;     // プロジェクション行列
    float4   mulColor;  // 乗算カラー
};

// 拡張定数バッファ (Decal.cpp 側からフェードアウト用のアルファ値を送る)
cbuffer DecalCb : register(b2)
{
    float g_alpha;   // フェードアウト用アルファ
    float3 g_padding;
};

Texture2D g_texture : register(t10);
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
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

// 頂点シェーダー
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    
    // エンジンの仕様通りに座標変換
    float4 worldPos = mul(mWorld, input.pos);
    float4 viewPos  = mul(mView, worldPos);
    output.pos      = mul(mProj, viewPos);
    
    output.uv = input.uv;
    
    return output;
}

// ピクセルシェーダー
float4 PSMain(PS_INPUT input) : SV_Target0
{
    // テクスチャの色を取得
    float4 color = g_texture.Sample(g_sampler, input.uv);

    // 乗算カラー(mulColor)を適用する
    color.rgb *= mulColor.rgb;

    // アルファテスト
    if(color.a < 0.05f)
    {
        discard;
    }

    // フェードアウト用のアルファ値を乗算
    color.a *= g_alpha;

    return color;
}