/*!
 * @brief 川瀬式ブルーム合成
 * @details 縮小バッファ4枚を加算平均合成して bloomIntensity 倍した結果を出力する
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

/*!
 * @brief ブルーム合成パラメーター
 */
cbuffer BloomFinalCb : register(b1)
{
    float bloomIntensity; // ブルームの強度
    float3 bloomPad;      // パディング
};

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

// 縮小ボケテクスチャ（最大4枚）
Texture2D<float4> g_bokeTexture_0 : register(t0);
Texture2D<float4> g_bokeTexture_1 : register(t1);
Texture2D<float4> g_bokeTexture_2 : register(t2);
Texture2D<float4> g_bokeTexture_3 : register(t3);
sampler Sampler : register(s0);

/*!
 * @brief 頂点シェーダー
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

/*!
 * @brief 川瀬式ブルーム合成ピクセルシェーダー
 * @details 4枚の縮小ボケテクスチャを平均合成し bloomIntensity 倍して出力する
 */
float4 PSBloomFinalKawase(PSInput In) : SV_Target0
{
    // 4枚のボケテクスチャを平均合成する
    float4 color = g_bokeTexture_0.Sample(Sampler, In.uv);
    color += g_bokeTexture_1.Sample(Sampler, In.uv);
    color += g_bokeTexture_2.Sample(Sampler, In.uv);
    color += g_bokeTexture_3.Sample(Sampler, In.uv);
    color /= 4.0f;

    color.rgb *= bloomIntensity;
    color.a = 1.0f;
    return color;
}
